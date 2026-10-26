#ifndef FEDERATIONSERVER_H
#define FEDERATIONSERVER_H
#include <QNetworkAccessManager>
#include <exception>
#include <QObject>
#include "federationresponse.h"
#include "../qstellar_namespace.h"
QSTELLAR_BEGIN_NS


/**
 * FederationServer handles a network connection to a
 * <a href="https://www.stellar.org/developers/learn/concepts/federation.html" target="_blank">federation server</a>
 * instance and exposes an interface for requests to that instance.
 *
 * A lookup follows the federation server's redirects, as js and py do, but
 * not one from https to http (Qt's NoLessSafeRedirectPolicy; js and py
 * follow that one too), on Qt 5 and Qt 6 alike. The domain's stellar.toml
 * is read with no redirect followed.
 *
 * For resolving a stellar address without knowing which federation server
 * to query use {@link Federation#resolve(String)}.
 *
 * Requests go through the calling thread's own network manager
 * (StellarTomlResolver::defaultNetworkAccessManager()), so a lookup may be
 * made from any thread with an event loop; a server is used from the thread
 * that made it.
 *
 * A query the server's URL already carries ("https://fed.example/api?v=2")
 * is kept, and type and q are added to it, as py sends them; js drops that
 * query and sends type and q alone.
 *
 * @see <a href="https://www.stellar.org/developers/learn/concepts/federation.html" target="_blank">Federation docs</a>
 */
class FederationServer : public QObject
{
    Q_OBJECT

    QUrl m_serverUri;
    QString m_domain;
    // Created by createForDomain() with the cache on: the cache deletes it.
    bool m_ownedByCache = false;
    static QNetworkAccessManager* s_httpClient;
    // The calling thread's cache, null while it is off: a cached server is a
    // QObject of the thread that made it, and the cache had no lock.
    static QHash<QString,FederationServer*>* cachedServers();
    static int s_tomlTimeout;
    QNetworkReply * buildReply(const QString &type, const QString &query);
    /** A lookup of `type` for `query`, now or once the stellar.toml is in. */
    FederationResponse* lookup(const QString& type, const QString& query);
    static QNetworkAccessManager &httpClient();


public:
    static bool isCacheActive();

    /** Turns the per-domain cache of federation servers on or off. Off, the
     *  cached servers are released and nothing is cached any more. The cache
     *  is the calling thread's, as its servers are: another thread has its
     *  own, off until it turns it on. A thread's cache, and the servers in
     *  it, are deleted when the thread ends (the main thread's when the
     *  application object is), as the thread's network manager is. */
    static void activeCache(bool active);

    /**
     * Releases the cached servers: deleted, except one still reading its
     * stellar.toml, which is forgotten at once, answers the lookups waiting
     * on it, and then deletes itself. activeCache(false) does the same.
     */
    static void clearCached();

#ifdef STELLAR_ENABLE_TEST_METHODS
    /**
     * Tests only: the network manager the stellar.toml and federation
     * requests go through from now on (a local fake server's), nullptr for
     * the default one. Returns the one it replaces.
     */
    static QNetworkAccessManager* setNetworkAccessManager(QNetworkAccessManager* manager)
    {
        QNetworkAccessManager* previous = s_httpClient;
        s_httpClient = manager;
        return previous;
    }
#endif

    /**
     * How long, in milliseconds, the stellar.toml request createForDomain()
     * sends may go without an answer before it is abandoned and error() is
     * emitted. STELLAR_QT_REPLY_TIMEOUT unless set; 0 or less restores that.
     * Without it, a host that accepted the connection and never answered left
     * the lookup pending for ever.
     */
    static void setTomlTimeout(int milliseconds);
    static int tomlTimeout();

    /**
     * Creates a new <code>FederationServer</code> instance.
     * @param serverUri Federation Server URI (https, with a host)
     * @param domain Domain name this federation server is responsible for
     * @throws FederationServerInvalidException Federation server is invalid (malformed URL, not HTTPS, etc.)
     */
    FederationServer(QUrl serverUri, QString domain);

    /**
     * Creates a new <code>FederationServer</code> instance.
     * @param serverUri Federation Server URI, https only as the QUrl
     *        constructor takes it: an http one was accepted, and anyone on the
     *        path could answer with their own account.
     * @param domain Domain name this federation server is responsible for
     * @throws FederationServerInvalidException Federation server is invalid (malformed URL, not HTTPS, etc.)
     */
    FederationServer(QString serverUri, QString domain);

    FederationServer();


    /**
     * Creates a <code>FederationServer</code> instance for a given domain.
     * It tries to find a federation server URL in stellar.toml file.
     * @see <a href="https://www.stellar.org/developers/learn/concepts/stellar-toml.html" target="_blank">Stellar.toml docs</a>
     * @param domain Domain to find a federation server for. Surrounding
     *        whitespace and a trailing '/' are dropped, as StellarTomlResolver
     *        drops them, and the domain so read is the server's getDomain(),
     *        what a name alone is looked up at, and its key in the cache (js
     *        refuses such a domain here, and trims it in its Resolver)
     *
     * @return FederationServer, you will have to wait until ready signal is emmited or error, as load happends asynchoronous.
     *
     * With the cache off the server is the caller's. With it on, it belongs
     * to the cache, from the moment it is created: every lookup for the
     * domain gets the same one, the lookups made before its stellar.toml
     * arrives included, and the toml is read once. The cache deletes it on
     * clearCached() or activeCache(false), and once every lookup has heard of
     * a toml that failed (so the next one asks again): hold it in a QPointer.
     * A cached server deleted by somebody else leaves the cache as well.
     */
    static FederationServer* createForDomain(QString domain);

    bool valid() const;
    /**
     * Resolves a stellar address using a given federation server.
     * @param address Stellar address, like <code>bob*stellar.org</code>, or
     *        a name alone (<code>bob</code>), which is looked up at this
     *        server's domain (bob*domain), as js does; py takes a full
     *        address only
     * @return FederationResponse
     * @throws std::runtime_error on an empty address, one with more than one
     *         '*', or a name alone when this server has no domain, whether or
     *         not the stellar.toml has arrived yet
     */
    FederationResponse* resolveAddress(const QString& address);

    /**
     * SEP-2 reverse lookup (type=id), as js's resolveAccountId and py's
     * resolve_account_id: the stellar address the server gives `accountId`.
     * The answer is read as py reads it: one without a stellar_address is
     * refused (js hands it back), and one without an account_id, or with a
     * null or empty one, is the record of `accountId`, which getAccountId()
     * and json() then carry (it was refused). One naming another account is
     * refused; py puts `accountId` in its place, js hands it back as sent.
     * @throws std::runtime_error when `accountId` is not an account id (G...)
     */
    FederationResponse* resolveAccountId(const QString& accountId);

    /**
     * SEP-2 transaction lookup (type=txid), as js's resolveTransactionId (py
     * has no such lookup): the federation record of the transaction's
     * sender.
     * @throws std::runtime_error when `transactionId` is not 64 hex digits
     */
    FederationResponse* resolveTransactionId(const QString& transactionId);

    /**
     * Returns a federation server URI.
     * @return URI
     */
    QUrl getServerUri() {
      return m_serverUri;
    }

    /**
     * Returns a domain this server is responsible for.
     * @return InternetDomainName
     */
    QString getDomain() {
      return m_domain;
    }

private slots:
    void initialize();

signals:
    void ready();
    void error(QString reason);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(FederationServer)

#endif // FEDERATIONSERVER_H
