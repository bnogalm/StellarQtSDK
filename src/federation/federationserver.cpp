#include "federationserver.h"
#include <QNetworkReply>
#include <QUrlQuery>
#include <QRegularExpression>

#include <QCoreApplication>
#include <QHash>
#include <QThreadStorage>
#include <stdexcept>

#include "../stellartomlresolver.h"
#include "../strkey.h"

QSTELLAR_BEGIN_NS


namespace {

// The federation server answers with the account a name is paid to, so it is
// reached over https only, as the constructor taking a QUrl and js require: an
// http FEDERATION_SERVER was used as given, and anyone on the path could
// answer with their own account.
bool isHttpsUrl(const QUrl& uri)
{
    return uri.isValid() && !uri.host().isEmpty()
           && uri.scheme().compare(QLatin1String("https"), Qt::CaseInsensitive) == 0;
}

// The domain as StellarTomlResolver::request() reads it: surrounding
// whitespace and a trailing '/' (or '\') are not part of it.
QString domainOf(const QString& domain)
{
    QString name = domain.trimmed();
    while (name.endsWith(QLatin1Char('/')) || name.endsWith(QLatin1Char('\\')))
        name.chop(1);
    return name;
}

// A thread's cache of servers, kept in a QThreadStorage: deleted with the
// servers it holds when the thread ends (the main thread's when the
// application object is), as the thread's network manager is. It was a
// thread_local raw pointer that nothing freed: a worker that turned the
// cache on and ended without activeCache(false) leaked the hash and every
// server in it, QObjects of a thread that was gone.
struct ServerCache
{
    QHash<QString, FederationServer*> servers;
    ~ServerCache()
    {
        // Emptied first: a server deleted looks itself up in the cache (by
        // then, QThreadStorage no longer hands this one out).
        const QList<FederationServer*> held = servers.values();
        servers.clear();
        qDeleteAll(held);
    }
};

QThreadStorage<ServerCache*>& serverCaches()
{
    static QThreadStorage<ServerCache*> caches;
    return caches;
}

} // namespace

QNetworkAccessManager* FederationServer::s_httpClient = nullptr;
int FederationServer::s_tomlTimeout = STELLAR_QT_REPLY_TIMEOUT;
FederationServer::FederationServer()
{

}

QNetworkReply *FederationServer::buildReply(const QString& type, const QString& address)
{
    // The query is percent-encoded whole, as js (URLSearchParams) and py
    // (requests) send it. QUrlQuery left '+', '#' and '%' as they were: a
    // server read "john+pay@x.com*d" as "john pay@x.com*d", "john#x*d" was
    // cut at the '#' (the rest became a fragment) and "a%2B*d" was decoded
    // to "a+*d" — another name than the one asked for. A query the
    // FEDERATION_SERVER URL already carries is kept, as py (requests) keeps
    // it; js drops it (url.search = ""), and with it a key the server may
    // need.
    QUrl url(m_serverUri);
    QString query = url.query(QUrl::FullyEncoded);
    if (!query.isEmpty())
        query += QLatin1Char('&');
    query += QStringLiteral("type=") + type + QStringLiteral("&q=")
             + QString::fromLatin1(QUrl::toPercentEncoding(address));
    url.setQuery(query, QUrl::StrictMode);
    QNetworkRequest request(url);
    // A redirect is followed, as js (fetch) and py (requests) follow it, but
    // not from https to http, which they follow too. The policy was left
    // unset: Qt 6 followed a redirect, and Qt 5.15 stopped at it and the 3xx
    // was refused as an answer. (The stellar.toml is read with no redirect
    // followed, as js reads it: StellarTomlResolver.)
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    return FederationServer::httpClient().get(request);
}

QNetworkAccessManager& FederationServer::httpClient(){
    // The tests' manager when one is set, otherwise the calling thread's own.
    // One made by the first caller, parented to qApp, served every thread
    // from the thread that happened to make it.
    if(FederationServer::s_httpClient)
        return *FederationServer::s_httpClient;
    return StellarTomlResolver::defaultNetworkAccessManager();
}

QHash<QString,FederationServer*>* FederationServer::cachedServers()
{
    QThreadStorage<ServerCache*>& caches = serverCaches();
    return caches.hasLocalData() ? &caches.localData()->servers : nullptr;
}

bool FederationServer::isCacheActive(){
    return cachedServers() != nullptr;
}

void FederationServer::activeCache(bool active)
{
    QThreadStorage<ServerCache*>& caches = serverCaches();
    if(active && !caches.hasLocalData()){
        caches.setLocalData(new ServerCache());
    }
    else if(!active && caches.hasLocalData()){
        clearCached();
        // Deleted and forgotten (setLocalData() deletes the one it
        // replaces): left pointing at the freed hash, the cache still read as
        // active and the next createForDomain() looked a domain up in freed
        // memory.
        caches.setLocalData(nullptr);
    }
}

void FederationServer::setTomlTimeout(int milliseconds)
{
    s_tomlTimeout = milliseconds > 0 ? milliseconds : STELLAR_QT_REPLY_TIMEOUT;
}

int FederationServer::tomlTimeout()
{
    return s_tomlTimeout;
}

void FederationServer::clearCached()
{
    QHash<QString,FederationServer*>* cache = cachedServers();
    if(cache){
        const QList<FederationServer*> servers = cache->values();
        cache->clear();
        for(FederationServer * s : servers){
            // One still reading its stellar.toml has lookups waiting on it,
            // which deleted now would never be answered. It answers them,
            // and then deletes itself (see initialize()).
            if(s->valid())
                s->deleteLater();
        }
    }
}

FederationServer::FederationServer(QUrl serverUri, QString domain) {
    m_serverUri = serverUri;
    if (!isHttpsUrl(m_serverUri)) {
        throw std::runtime_error("FederationServerInvalidException");
    }
    m_domain = domain;
}

FederationServer::FederationServer(QString serverUri, QString domain) {

    m_serverUri = QUrl(serverUri);
    if(!m_serverUri.isValid())
        throw std::runtime_error("invalid federation uri");
    // https only, as the QUrl constructor and a toml's FEDERATION_SERVER: an
    // http server was accepted here, and anyone on the path could answer with
    // their own account for the name being paid.
    if (!isHttpsUrl(m_serverUri))
        throw std::runtime_error("FederationServerInvalidException");

    m_domain = domain;
}



FederationServer *FederationServer::createForDomain(QString domain) {

    // Read once, as the stellar.toml request reads it: " acme.com/" fetched
    // acme.com's toml but was kept as given, so resolveAddress("bob") asked
    // for "bob* acme.com/", and the cache kept it apart from "acme.com". js
    // refuses such a domain here (validateDomain), and its StellarToml.Resolver
    // trims it as this does.
    domain = domainOf(domain);
    QHash<QString,FederationServer*>* cache = cachedServers();
    if(cache){
        // Ready, or still reading its stellar.toml.
        FederationServer * cached = cache->value(domain,nullptr);
        if(cached)
            return cached;
    }
    // SEP-1's request, as StellarTomlResolver makes it: https, no redirect,
    // given up after the toml timeout (a host that accepted the connection
    // and never answered left the lookup pending for ever), and stopped past
    // 100 KB. A domain that is not a host name (an optional port aside)
    // throws.
    QNetworkReply * reply = StellarTomlResolver::request(domain, FederationServer::httpClient(), tomlTimeout());

    FederationServer *federationServer = new FederationServer();
    federationServer->m_domain=domain;
    QObject::connect(reply, &QNetworkReply::finished, federationServer , &FederationServer::initialize);
    // A server deleted before its toml arrives (its lookup's response was)
    // takes the request with it: nothing else would ever delete it.
    QObject::connect(federationServer, &QObject::destroyed, reply, &QObject::deleteLater);

    // Cached from the start, still reading its toml: a second lookup for the
    // domain made before the toml arrives waits on this one. It was cached
    // once the toml had been read, and with USE_CPPTOML only, so the default
    // build cached nothing and left every server of a lookup made with the
    // cache on to leak; and two early lookups made two servers, the second
    // replacing the first in the cache, which leaked.
    if(cache){
        cache->insert(domain, federationServer);
        federationServer->m_ownedByCache = true;
        // The domain by value (destroyed() comes after this object's own
        // members are gone), and the cache only if it still exists:
        // activeCache(false) deletes it. Deleted by whoever, a server leaves
        // no pointer behind in the cache.
        QObject::connect(federationServer, &QObject::destroyed, [domain](QObject *obj){
            QHash<QString,FederationServer*>* servers = cachedServers();
            if(servers && servers->value(domain) == obj)
                servers->remove(domain);
        });
    }

    return federationServer;
}

bool FederationServer::valid() const
{
    return this->m_serverUri.isValid();
}

FederationResponse *FederationServer::resolveAddress(const QString &address) {
    // Refused now, before any wait: thrown when the toml arrived, from inside
    // ready(), it cut that signal short for every other lookup waiting on
    // this server, which with the cache on is shared.
    if (address.isEmpty())
        throw std::runtime_error("Malformed address: the address is empty");
    // A name alone is the name at this server's domain, as js reads it
    // (createForDomain("acme.com") then resolveAddress("bob") asks for
    // bob*acme.com); it threw, whatever the server knew. py takes a full
    // address only.
    QString full = address;
    if (!address.contains(QLatin1Char('*'))) {
        if (m_domain.isEmpty())
            throw std::runtime_error("Malformed address: a name alone needs a server that knows its domain "
                                     "(or give the address whole, as bob*example.com)");
        full = address + QLatin1Char('*') + m_domain;
    }
    if (full.split(QLatin1Char('*')).size() != 2)
        throw std::runtime_error("Malformed address: more than one '*'");
    return lookup(QStringLiteral("name"), full);
}

FederationResponse *FederationServer::resolveAccountId(const QString &accountId) {
    // Checked, as the address is, before any request: js and py send
    // whatever they are given, and the server's answer to junk is its own.
    bool account = false;
    try {
        account = StrKey::decodeVersionByte(accountId) == StrKey::VersionByte::ACCOUNT_ID
                  && StrKey::encodeStellarAccountId(StrKey::decodeStellarAccountId(accountId)) == accountId;
    } catch (const std::exception&) {
    }
    if (!account)
        throw std::runtime_error("a reverse federation lookup takes an account id (G...)");
    return lookup(QStringLiteral("id"), accountId);
}

FederationResponse *FederationServer::resolveTransactionId(const QString &transactionId) {
    static const QRegularExpression hash(QStringLiteral("^[0-9a-fA-F]{64}$"));
    if (!hash.match(transactionId).hasMatch())
        throw std::runtime_error("a transaction lookup takes a transaction id (64 hex digits)");
    return lookup(QStringLiteral("txid"), transactionId);
}

FederationResponse *FederationServer::lookup(const QString &type, const QString &query) {
    if(!this->valid()){
        //delay request
        auto response = new FederationResponse(nullptr);
        if (type == QLatin1String("id"))
            response->m_reverseOf = query;

        // In the response's context, so a response deleted before the toml
        // arrives is not written to.
        connect(this,&FederationServer::ready,response,[this,type,query,response](){
            auto reply = buildReply(type, query);
            response->loadFromReply(reply);
            response->limitSize(reply);
        });
        // With the reason: a bare error() left the caller unable to tell a
        // missing toml from one with no FEDERATION_SERVER or a non-https one.
        connect(this, &FederationServer::error, response,
                [response](const QString& reason) { response->fail(reason); });
        return response;
    }
    auto reply = buildReply(type, query);
    auto response = new FederationResponse(reply);
    if (type == QLatin1String("id"))
        response->m_reverseOf = query;
    return response;
}

//initalize the federation server from reply, reply is the sender()
void FederationServer::initialize()
{
    QNetworkReply * reply = static_cast<QNetworkReply*>(sender());
    reply->deleteLater();
    // Why the toml gives no federation server; empty when it gives one.
    QString failure;
    // The toml as StellarTomlResolver::read() has it, or why not: a failed
    // request (only codes from 300 up used to count, so a 404 page or an
    // unreachable host was read as the toml), an answer over 100 KB, not TOML.
    QString server;
    try {
        server = StellarTomlResolver::read(reply).value("FEDERATION_SERVER").toString();
    } catch (const std::exception& e) {
        failure = QString::fromUtf8(e.what());
    }
    if (failure.isEmpty()) {
        const QUrl uri(server, QUrl::StrictMode);
        if (server.isEmpty())
            failure = QStringLiteral("No federation server");
        else if (!isHttpsUrl(uri))
            failure = QStringLiteral("Invalid federation server");
        else
            this->m_serverUri = uri;
    }

    // Done before anybody hears of it: a slot
    // may delete this server. A server of the cache whose toml failed leaves
    // it, so that the next lookup asks again, and is deleted once the lookups
    // waiting on it have heard: it used to stay behind, owned by nobody. So is
    // one clearCached() or activeCache(false) let go of while its toml was on
    // its way.
    if (m_ownedByCache) {
        QHash<QString,FederationServer*>* cache = cachedServers();
        const bool cached = cache && cache->value(m_domain) == this;
        if (!failure.isEmpty() && cached)
            cache->remove(m_domain);
        if (!failure.isEmpty() || !cached)
            deleteLater();
    }
    if (failure.isEmpty())
        emit ready();
    else
        emit error(failure);
}
QSTELLAR_END_NS
