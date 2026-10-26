#include "horizonqml.h"

#include <QtEndian>
#include <QChildEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QScopedPointer>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>

#include "abstracttransaction.h"
#include "asset.h"
#include "assetregistry.h"
#include "keypair.h"
#include "network.h"
#include "requests/accountsrequestbuilder.h"
#include "responses/claimablebalanceresponse.h"
#include "responses/accountresponse.h"
#include "responses/problem.h"
#include "responses/submittransactionresponse.h"
#include "server.h"

#include "requests/assetsrequestbuilder.h"
#include "requests/feestatsrequestbuilder.h"
#include "requests/ledgersrequestbuilder.h"
#include "requests/offersrequestbuilder.h"
#include "requests/operationsrequestbuilder.h"
#include "requests/orderbookrequestbuilder.h"
#include "requests/paymentsrequestbuilder.h"
#include "requests/rootrequestbuilder.h"
#include "requests/transactionsrequestbuilder.h"

#include "responses/assetresponse.h"
#include "responses/feestatsresponse.h"
#include "responses/ledgerresponse.h"
#include "responses/offerresponse.h"
#include "responses/operationpage.h"
#include "responses/operations/operationresponse.h"
#include "responses/orderbookresponse.h"
#include "responses/page.h"
#include "responses/rootresponse.h"
#include "responses/transactionresponse.h"
#include "price.h"
#include "strkey.h"

#include "exception/accountnotfoundexception.h"
#include "exception/accountrequiresmemoexception.h"
#include "exception/badrequestexception.h"
#include "exception/badresponseexception.h"
#include "exception/connectionerrorexception.h"
#include "exception/requesttimeoutexception.h"
#include "exception/toomanyrequestsexception.h"
#include "exception/unknownresponseexception.h"

QSTELLAR_BEGIN_NS

namespace {

const char* const TESTNET_HORIZON = "https://horizon-testnet.stellar.org";
const char* const PUBLIC_HORIZON = "https://horizon.stellar.org";
const char* const TESTNET_FRIENDBOT = "https://friendbot.stellar.org";
const char* const FUTURENET_HORIZON = "horizon-futurenet.stellar.org";
const char* const FUTURENET_FRIENDBOT = "https://friendbot-futurenet.stellar.org";

/** True if `value` decodes as a G... account id. Never throws. */
bool isAccountId(const QString& value)
{
    try {
        StrKey::decodeStellarAccountId(value);
        return true;
    } catch (...) {
        return false;
    }
}

/**
 * The Network object for a passphrase.
 *
 * The SDK caches the two well-known networks; a custom passphrase (a standalone
 * core, a quickstart image) would otherwise leak a Network per call, so those
 * are cached here the same way.
 */
Network* networkFor(const QString& passphrase)
{
    if (passphrase == Network::PUBLIC_NETWORK_PASSPHRASE())
        return Network::publicNetwork();
    if (passphrase == Network::TESTNET_NETWORK_PASSPHRASE())
        return Network::testnetNetwork();

    static QHash<QString, Network*> custom;
    auto it = custom.constFind(passphrase);
    if (it != custom.constEnd())
        return it.value();
    Network* net = new Network(passphrase);
    custom.insert(passphrase, net);
    return net;
}

/** Canonical "native" / "CODE:GISSUER" for a balance line. */
QString canonicalAsset(AccountResponseAttach::Balance& balance)
{
    const QString type = balance.getAssetType();
    if (type == QLatin1String("native"))
        return QStringLiteral("native");
    if (type == QLatin1String("liquidity_pool_shares"))
        return QStringLiteral("pool:") + balance.getLiquidityPoolId();
    return balance.getAssetCode() + QLatin1Char(':')
           + balance.getAssetIssuer().getAccountId();
}

/**
 * The problem document attached to a failed request, if there is one.
 *
 * Horizon answers a rejected submission with 400 and a body that says exactly
 * what went wrong; the SDK parses it and hands it to the exception, but the
 * exception's own message is the flat string "Bad Request.". Reaching past that
 * message to the document is the difference between telling a user "Bad
 * Request" and telling them "tx_bad_seq".
 */
const Problem* problemOf(qstellar::exception::SdkException* ex)
{
    using namespace qstellar::exception;
    if (auto* e = dynamic_cast<BadRequestException*>(ex))     return e->getProblem();
    if (auto* e = dynamic_cast<BadResponseException*>(ex))    return e->getProblem();
    if (auto* e = dynamic_cast<RequestTimeoutException*>(ex)) return e->getProblem();
    return nullptr;
}

/**
 * The POST /transactions a submit's own Server has handed to the network
 * stack, or nullptr while it has not (its SEP-29 lookups are GETs). That
 * Server makes no other POST, and Qt parents every reply to the manager
 * that made it. The reply lives until its response is deleted, which is
 * after the submit has been reported; so while the submit is pending, no
 * reply means not posted.
 */
QNetworkReply* postOf(Server* server)
{
    const QList<QNetworkReply*> replies = server->manager().findChildren<QNetworkReply*>();
    for (QNetworkReply* reply : replies) {
        if (reply->operation() == QNetworkAccessManager::PostOperation)
            return reply;
    }
    return nullptr;
}

/**
 * The network errors of a connection that never opened. Every other error
 * may come after the request was written (a connection lost midway, an HTTP
 * error status). The same list SorobanServer uses.
 *
 * Not enough, on its own, to tell that a request never left: it describes
 * the LAST connection tried. Qt 5 sends a request again, by itself, when the
 * connection it was written on closes before an answer, and that second
 * connection can be refused although the first carried the request whole;
 * the reply then ends in ConnectionRefusedError. So, as SorobanServer does,
 * a POST is known never to have left only when this is true AND not a byte
 * of it was ever written (Submit::written).
 */
bool neverConnected(QNetworkReply::NetworkError error)
{
    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::ProxyConnectionRefusedError:
    case QNetworkReply::ProxyNotFoundError:
    case QNetworkReply::SslHandshakeFailedError:
    case QNetworkReply::ProtocolUnknownError:
        return true;
    default:
        return false;
    }
}

/** A failure map of `kind` saying `message`. */
QVariantMap failureOf(const QString& kind, const QString& message)
{
    QVariantMap error;
    error.insert(QStringLiteral("kind"), kind);
    error.insert(QStringLiteral("message"), message);
    return error;
}

/**
 * The message of a failure of a submit whose POST Qt sent again (`resent`,
 * see failed()), whatever its kind: "not final", the hash to look up, then
 * what ended it. A cancel's said neither, and the rest named no hash.
 */
QString notFinal(const QString& hash, const QString& message)
{
    return QStringLiteral("not final: Qt sent the transaction again, by itself, after the "
                          "connection that carried it whole closed with no answer, so what "
                          "follows came of a later send, and Horizon may have applied the "
                          "first; look it up by its hash, %1, before submitting it again or "
                          "reusing its sequence number. %2").arg(hash, message);
}

/**
 * An event filter for a submit's QNetworkAccessManager that calls `written`
 * each time the connection writes any of the body of a request the manager
 * makes, with `again` true when that body had already been written whole
 * before: Qt 5 sends a request again, by itself, on a new connection, when
 * the one it was written on closes before an answer, and writes its body
 * from the start. Only the submit's POST has a body: its network and SEP-29
 * lookups are GETs. The manager parents every reply it makes, so it is told
 * of each one (QEvent::ChildAdded) from inside the reply's constructor,
 * whoever makes it: the POST the Server makes once the SEP-29 lookups have
 * passed is made inside the Server, out of HorizonQml's sight otherwise.
 */
class UploadWatch : public QObject
{
public:
    UploadWatch(QObject* parent, std::function<void(bool again)> written)
        : QObject(parent), m_written(std::move(written))
    {
    }

    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (event->type() == QEvent::ChildAdded) {
            // Told of from inside the reply's constructor: only a QObject so
            // far, so it is looked at once it is made, by an event of its
            // own. That event is queued before any the reply queues itself,
            // the one that starts its request included, and what the
            // connection writes is reported by events queued after those:
            // so the watch is on before a byte of it can be written, and
            // misses none. A reply deleted first takes the event with it.
            QObject* child = static_cast<QChildEvent*>(event)->child();
            const std::function<void(bool)> written = m_written;
            // The reply outlives this watch: a deleted Server's manager and
            // replies are deleted later, from the event loop (I8), while the
            // watch goes with the Server, and the Server with HorizonQml.
            // What the reply emits in between must not reach `written`,
            // whose HorizonQml may be gone: so the connection is the watch's,
            // and none is made once the watch is gone. It was the reply's,
            // and a HorizonQml deleted with a submit on the wire was written
            // to after it was freed.
            const QPointer<UploadWatch> watch(this);
            QMetaObject::invokeMethod(child, [child, written, watch]() {
                // Not every child is a reply: the manager makes its cookie
                // jar once an answer sets a cookie.
                auto* reply = qobject_cast<QNetworkReply*>(child);
                if (!reply || !watch)
                    return;
                // Whether this reply's body has been written whole. Qt says
                // so once per send, as (total, total), however it throttles
                // the steps before; a send Qt makes again starts from the
                // start, and its (total, total) is said again. The SDK's POST
                // has a length (its form body), so the total is known.
                auto whole = std::make_shared<bool>(false);
                // A reply that ends with nothing written says so too, as (0, 0).
                QObject::connect(reply, &QNetworkReply::uploadProgress, watch.data(),
                                 [written, whole](qint64 sent, qint64 total) {
                    if (sent <= 0)
                        return;
                    const bool again = *whole;
                    if (sent == total)
                        *whole = true;
                    written(again);
                });
            }, Qt::QueuedConnection);
        }
        return QObject::eventFilter(watched, event);
    }

private:
    std::function<void(bool again)> m_written;
};

} // namespace

HorizonQml::HorizonQml(AssetRegistry* assets, QObject* parent)
    : QObject(parent)
    , m_assets(assets)
    , m_endpoint(QString::fromLatin1(TESTNET_HORIZON))
{
}

void HorizonQml::setFriendbotUrl(const QString& url)
{
    if (url == m_friendbotUrl)
        return;
    m_friendbotUrl = url;
    emit friendbotUrlChanged();
}

QString HorizonQml::friendbotUrlFor(const QString& endpoint) const
{
    // By the endpoint's host, exactly: "contains testnet" refused futurenet
    // and a local quickstart (which have one) and sent any URL with
    // "testnet" in it to SDF's testnet friendbot, whatever it served. SDF's
    // test Horizons only redirect /friendbot to these.
    QUrl url(endpoint);
    const QString host = url.host().toLower();
    if (host == QUrl(QString::fromLatin1(TESTNET_HORIZON)).host())
        return QString::fromLatin1(TESTNET_FRIENDBOT);
    if (host == QLatin1String(FUTURENET_HORIZON))
        return QString::fromLatin1(FUTURENET_FRIENDBOT);
    // The public network has none: its Horizon answers /friendbot with 404.
    if (host.isEmpty() || host == QUrl(QString::fromLatin1(PUBLIC_HORIZON)).host())
        return QString();
    // Any other Horizon is asked at its own /friendbot, as js-stellar-sdk's
    // Horizon.Server.friendbot() asks it: a local quickstart serves one
    // there, and was refused unless friendbotUrl was set. One that has none
    // answers 404, which fundAccount() reports.
    QString path = url.path();
    while (path.endsWith(QLatin1Char('/')))
        path.chop(1);
    url.setPath(path + QStringLiteral("/friendbot"));
    url.setFragment(QString());
    return url.toString();
}

// No thread, so nothing to wait for or to stop: the Servers are children,
// and go with this object. Their managers and replies are deleted later,
// from the event loop (see ~Server), which aborts whatever is in flight;
// nothing of them reaches this object in between, as every connection to
// them has this object or one of its children as context. Nothing is
// reported (see the class comment).
HorizonQml::~HorizonQml() = default;

QString HorizonQml::horizonUrlFor(const QString& network) const
{
    if (network == QLatin1String("testnet"))
        return QString::fromLatin1(TESTNET_HORIZON);
    if (network == QLatin1String("public"))
        return QString::fromLatin1(PUBLIC_HORIZON);
    return QString();
}

void HorizonQml::setEndpoint(const QString& endpoint)
{
    if (endpoint == m_endpoint)
        return;
    m_endpoint = endpoint;
    // The Server holds the QNetworkAccessManager and the base URL; pointing at
    // a different Horizon means a different Server. The shared one carries
    // queries only, which are forgotten with it. A submit is on a Server of
    // its own, which cancelAll() leaves alive until the submit is reported:
    // this used to delete the one Server every request shared, a posted
    // submit's manager and reply included, and forget the submit.
    if (m_server) {
        m_server->deleteLater();
        m_server = nullptr;
    }
    // Which network it serves is the new endpoint's to say. Forgotten before
    // the cancel, whose pendingChanged a handler may start a submit from.
    // A lookup of the old one's still running is dropped by the cancel, with
    // its submit (not posted), so no answer of the old endpoint lands here.
    m_networkPassphrase.clear();
    cancelAll();
    emit endpointChanged();
}

Server* HorizonQml::server()
{
    if (!m_server) {
        try {
            m_server = new Server(m_endpoint);
            m_server->setParent(this);
        } catch (const std::exception&) {
            m_server = nullptr;
        }
    }
    return m_server;
}

QNetworkAccessManager* HorizonQml::directManager()
{
    if (!m_directManager)
        m_directManager = new QNetworkAccessManager(this);
    return m_directManager;
}

// A handler of this pendingChanged can cancel the request it announces
// (cancelAll(), a new endpoint) before the call that started it has asked
// anything of Horizon: every caller checks that it is still pending before
// it does, and returns the id untouched when it is not. submit() used to post
// all the same, report nothing, and keep the Server it built until this
// object died.
int HorizonQml::nextRequestId()
{
    const int id = ++m_lastRequestId;
    m_pending.insert(id, true);
    emit pendingChanged();
    return id;
}

void HorizonQml::finish(int requestId)
{
    const auto submit = m_submits.find(requestId);
    if (submit != m_submits.end()) {
        // Its Server has done its work. Later, not now: this runs from its
        // reply's own signals.
        submit->server->deleteLater();
        m_submits.erase(submit);
    }
    if (m_pending.remove(requestId))
        emit pendingChanged();
}

void HorizonQml::cancelAll()
{
    // Forgotten at once, all but the submits already posted: one of those
    // may reach Horizon and be applied, so it is kept, and reported on the
    // next pass (settleCancelledSubmits). It used to be forgotten with the
    // rest, its outcome dropped when it came, and nothing aborted its POST:
    // a wallet that cancelled while a payment was on the wire never heard
    // that it may land.
    bool forgot = false;
    bool toSettle = false;
    for (auto it = m_pending.begin(); it != m_pending.end();) {
        const auto submit = m_submits.find(it.key());
        if (submit != m_submits.end()) {
            if (postOf(submit->server)) {
                submit->cancelled = true;
                toSettle = true;
                ++it;
                continue;
            }
            // Still asking the endpoint's network, or in its SEP-29
            // lookups: it never goes out. Its response is deleted now
            // rather than later, because the Server posts the envelope
            // from a lookup's answer unless the response is gone, and an
            // answer already queued would come before a deferred delete.
            // Its lookups go with its Server; the answer of its network
            // lookup, already queued or not, finds it gone and posts
            // nothing (checkNetwork()).
            delete submit->response.data();
            submit->server->deleteLater();
            m_submits.erase(submit);
        }
        it = m_pending.erase(it);
        forgot = true;
    }
    if (forgot)
        emit pendingChanged();
    // Queued, so no failed() is emitted from inside this call (a handler
    // could start or cancel requests while the loop above runs), and so an
    // end already on its way here is reported as what it was.
    if (toSettle)
        QMetaObject::invokeMethod(this, [this]() { settleCancelledSubmits(); },
                                  Qt::QueuedConnection);
}

void HorizonQml::settleCancelledSubmits()
{
    QList<int> ids;
    for (auto it = m_submits.cbegin(); it != m_submits.cend(); ++it) {
        if (it->cancelled)
            ids.append(it.key());
    }
    std::sort(ids.begin(), ids.end());      // reported in the order started
    for (const int id : ids) {
        // A handler of an earlier report may have settled it, and one that
        // ended on its own since was reported as that end.
        const auto submit = m_submits.constFind(id);
        if (submit == m_submits.constEnd() || !submit->cancelled || !m_pending.contains(id))
            continue;
        const QString hash = submit->hash;
        const bool resent = submit->resent;
        const QPointer<QNetworkReply> post(postOf(submit->server));
        finish(id);
        // Off `pending` first: the error the abort produces, which Qt
        // delivers before abort() returns, is then dropped as a cancelled
        // request's.
        if (post)
            post->abort();
        QVariantMap error;
        error.insert(QStringLiteral("kind"), QStringLiteral("cancelled"));
        const QString message =
            QStringLiteral("cancelled while it was on the wire: Horizon may have received "
                           "the transaction, so look it up by its hash before submitting "
                           "it again");
        error.insert(QStringLiteral("mayHaveReachedServer"), true);
        error.insert(QStringLiteral("hash"), hash);
        // As every failure of a submit Qt sent again says (failSubmit()): the
        // key, and the message. (The message was the cancel's alone.)
        if (resent)
            error.insert(QStringLiteral("resent"), true);
        error.insert(QStringLiteral("message"), resent ? notFinal(hash, message) : message);
        emit failed(id, error);
    }
}

void HorizonQml::failWith(int requestId, const QString& kind, const QString& message)
{
    if (!m_pending.contains(requestId))
        return; // cancelled while in flight
    finish(requestId);
    QVariantMap error;
    error.insert(QStringLiteral("kind"), kind);
    error.insert(QStringLiteral("message"), message);

    // QUEUED, DELIBERATELY. Several rejections happen before a byte goes out —
    // an unknown query name, a malformed address, an unusable endpoint — and
    // this is called from inside loadAccount()/submit()/runQuery(), which have
    // not returned the request id yet. Emitting inline meant the caller's
    // `page.request = Horizon.submit(...)` had not been assigned when the
    // signal arrived, so every consumer that filters on the id — which is all
    // of them, because the facade tells them to — silently dropped the
    // failure and sat there marked busy for ever.
    QMetaObject::invokeMethod(this, [this, requestId, error]() {
        emit failed(requestId, error);
    }, Qt::QueuedConnection);
}

QVariantMap HorizonQml::responseError(Response* response) const
{
    QString kind = QStringLiteral("unknown");
    QString message;
    QVariantMap error;

    // The typed exception is the reliable classifier; the RFC 7807 fields
    // Horizon fills in are what makes the message worth reading.
    if (qstellar::exception::SdkException* ex = response->lastException()) {
        message = QString::fromUtf8(ex->what());
        if (auto* memo = dynamic_cast<qstellar::exception::AccountRequiresMemoException*>(ex)) {
            // SEP-29 stopped the submit before it went out. This arrived as a
            // bare error, which fell through to status 0 and was reported as
            // "connection": a user told to check their network retried and
            // was refused again, never told that a memo was missing.
            kind = QStringLiteral("memoRequired");
            error.insert(QStringLiteral("destination"), memo->getAccountId());
            error.insert(QStringLiteral("operationIndex"), memo->getOperationIndex());
        }
        else if (dynamic_cast<qstellar::exception::AccountNotFoundException*>(ex))   kind = QStringLiteral("notFound");
        else if (dynamic_cast<qstellar::exception::RequestTimeoutException*>(ex))    kind = QStringLiteral("timeout");
        else if (auto* limited = dynamic_cast<qstellar::exception::TooManyRequestsException*>(ex)) {
            kind = QStringLiteral("rateLimited");
            // When to ask again, which is the one thing a rate limit is for
            // (also when a SEP-29 lookup ahead of a submit met it).
            if (limited->getRetryAfter() >= 0)
                error.insert(QStringLiteral("retryAfter"), limited->getRetryAfter());
        }
        else if (dynamic_cast<qstellar::exception::BadRequestException*>(ex))        kind = QStringLiteral("badRequest");
        else if (dynamic_cast<qstellar::exception::BadResponseException*>(ex))       kind = QStringLiteral("badResponse");
        // The SDK gave up waiting (it is a ConnectionErrorException too, so it
        // is asked first). It was "connection", which says the request never
        // went out, while a submit that timed out may still land.
        else if (dynamic_cast<qstellar::exception::ClientTimeoutException*>(ex))     kind = QStringLiteral("timeout");
        else if (dynamic_cast<qstellar::exception::ConnectionErrorException*>(ex))   kind = QStringLiteral("connection");
        // A reply that could not be read (bad JSON, a status line with no
        // code, a 1xx or 3xx) is the "badResponse" the class comment names.
        // It fell through to the status: "unknown" after a 200, and
        // "connection" when a SEP-29 lookup met it ahead of a submit, as if
        // the server had never answered.
        else if (dynamic_cast<qstellar::exception::UnknownResponseException*>(ex))   kind = QStringLiteral("badResponse");
    }

    const int status = response->getStatus();
    // Fall back to the HTTP status when no typed exception was attached.
    if (kind == QStringLiteral("unknown")) {
        if (status == 404)      kind = QStringLiteral("notFound");
        else if (status == 429) kind = QStringLiteral("rateLimited");
        else if (status >= 500) kind = QStringLiteral("badResponse");
        else if (status >= 400) kind = QStringLiteral("badRequest");
        else if (status == 0)   kind = QStringLiteral("connection");
    }

    error.insert(QStringLiteral("status"), status);
    QString title = response->property("title").toString();
    QString detail = response->property("detail").toString();

    // Horizon's problem document, when there is one, outranks everything above:
    // it carries the result codes, and those are the only part of a failure a
    // user can actually act on.
    if (const Problem* problem = problemOf(response->lastException())) {
        if (!problem->getTitle().isEmpty())
            title = problem->getTitle();
        if (!problem->getDetail().isEmpty())
            detail = problem->getDetail();

        const auto codes = problem->getExtras().getResultCodes();
        const QString txCode = codes.getTransaction();
        if (!txCode.isEmpty()) {
            error.insert(QStringLiteral("txResultCode"), txCode);
            // A rejected-by-the-network transaction is not a malformed request.
            // The remedies are unrelated — reread the sequence, add a signature,
            // fund the account — so it gets its own kind.
            kind = QStringLiteral("txFailed");
            message = txCode;
        }
        if (!codes.getOperations().isEmpty())
            error.insert(QStringLiteral("opResultCodes"), QVariant(codes.getOperations()));
        if (!codes.getInnerTransaction().isEmpty())
            error.insert(QStringLiteral("innerTxResultCode"), codes.getInnerTransaction());
        if (!problem->getExtras().getResultXdr().isEmpty())
            error.insert(QStringLiteral("resultXdr"), problem->getExtras().getResultXdr());
    }

    error.insert(QStringLiteral("kind"), kind);
    error.insert(QStringLiteral("title"), title);
    error.insert(QStringLiteral("detail"), detail);

    // The typed exception's what() is a category, not a reason:
    // BadRequestException says "Bad Request." for a malformed envelope, a
    // rejected fee-bump and an operation the network stopped supporting alike.
    // When Horizon filled in a problem document and there was no result code
    // to lead with, its title is the actual answer — this function went to the
    // trouble of reading it and then let the flat message win. Submitting an
    // `inflation` operation reported "Bad Request." rather than saying the
    // transaction was malformed.
    if (!title.isEmpty()
            && !error.contains(QStringLiteral("txResultCode"))
            && !error.contains(QStringLiteral("opResultCodes"))) {
        message = detail.isEmpty() ? title
                                   : title + QStringLiteral(" \u2014 ") + detail;
    }
    if (message.isEmpty())
        message = title;
    if (message.isEmpty())
        message = QStringLiteral("request failed");
    error.insert(QStringLiteral("message"), message);
    return error;
}

void HorizonQml::failNow(int requestId, const QString& kind, const QString& message)
{
    finish(requestId);
    QVariantMap error;
    error.insert(QStringLiteral("kind"), kind);
    error.insert(QStringLiteral("message"), message);
    emit failed(requestId, error);
}

void HorizonQml::wire(int requestId, Response* response, std::function<QVariantMap()> read,
                      void (HorizonQml::*outcome)(int, const QVariantMap&))
{
    QPointer<Response> guard(response);

    connect(response, &Response::ready, this, [this, requestId, guard, read, outcome]() {
        if (!m_pending.contains(requestId)) {
            if (guard) guard->deleteLater();
            return;
        }
        finish(requestId);
        // Nothing may be thrown out of here: this runs inside Qt's signal
        // dispatch, off the network reply, with no catch above it, and the
        // SDK's getters throw on data they cannot read (a balance whose
        // issuer is not an account id). One reply like that terminated the
        // app. It is answered as the failure it is instead, and read apart
        // from the emit so a result is never followed by a failure.
        QVariantMap result;
        bool readOk = false;
        QString problem;
        try {
            result = read();
            readOk = true;
        } catch (const std::exception& e) {
            problem = QString::fromUtf8(e.what());
        } catch (...) {
            problem = QStringLiteral("unknown error");
        }
        if (guard) guard->deleteLater();
        if (readOk)
            emit (this->*outcome)(requestId, result);
        else
            failNow(requestId, QStringLiteral("badResponse"),
                    QStringLiteral("Horizon's reply could not be read: ") + problem);
    });

    connect(response, &Response::error, this, [this, requestId, guard]() {
        if (!m_pending.contains(requestId)) {
            if (guard) guard->deleteLater();
            return;         // cancelled while in flight
        }
        finish(requestId);
        // Same rule as above: classifying the failure must not throw it
        // through Qt either.
        QVariantMap error;
        try {
            if (guard)
                error = responseError(guard);
        } catch (...) {
            error.clear();
        }
        if (error.isEmpty()) {
            error.insert(QStringLiteral("kind"), QStringLiteral("unknown"));
            error.insert(QStringLiteral("message"), QStringLiteral("request failed"));
        }
        if (guard) guard->deleteLater();
        emit failed(requestId, error);
    });
}

namespace {

struct QueryField
{
    const char* name;
    const char* kind;
    const char* label;
    bool required;
    const char* note;
    /** A plausible value, so a demo has something to run without typing. */
    const char* demo;
};

struct QuerySpec
{
    const char* name;
    const char* summary;
    QVector<QueryField> fields;
};

/**
 * What a form may ask Horizon.
 *
 * Ordered the way somebody explores: what network is this, then one thing, then
 * lists of things. Built with calls rather than a nested brace initialiser
 * because MSVC will not deduce the inner QVector<QueryField> through an outer
 * initializer_list.
 */
const QVector<QuerySpec>& queryTable()
{
    static const QVector<QuerySpec> table = []() {
        QVector<QuerySpec> t;
        auto add = [&t](const char* name, const char* summary,
                        const QVector<QueryField>& fields) {
            QuerySpec spec;
            spec.name = name;
            spec.summary = summary;
            spec.fields = fields;
            t.append(spec);
        };

        add("network", "Which network this Horizon serves, and how current it is.", {});

        add("account", "Everything the ledger holds about one account.", {
            { "account", "address", "Account", true,
              "A G address. An account that was never funded does not exist at "
              "all, and comes back as not found rather than as empty.", "" },
        });

        add("transaction", "One transaction, by its hash.", {
            { "hash", "text", "Transaction hash", true,
              "64 hex characters. This is the hash of the transaction, so it is "
              "the same before and after it was signed.", "" },
        });

        add("transactionsForAccount", "The transactions an account took part in.", {
            { "account", "address", "Account", true, "", "" },
            { "limit", "uint32", "How many", false, "1 to 200; Horizon defaults to 10.", "" },
            { "includeFailed", "bool", "Include failed", false,
              "Off by default, so a rejected transaction does not appear at all "
              "— which is confusing when it is the one you are looking for.", "" },
        });

        add("operationsForAccount", "Every operation that touched an account.", {
            { "account", "address", "Account", true, "", "" },
            { "limit", "uint32", "How many", false, "1 to 200.", "" },
            { "includeFailed", "bool", "Include failed", false, "", "" },
        });

        add("paymentsForAccount", "Only the operations that moved value.", {
            { "account", "address", "Account", true,
              "Payments, account creations, path payments and merges. Trustline "
              "and offer changes are operations but not payments.", "" },
            { "limit", "uint32", "How many", false, "1 to 200.", "" },
        });

        add("offersForAccount", "The offers an account has on the book right now.", {
            { "account", "address", "Account", true, "", "" },
            { "limit", "uint32", "How many", false, "1 to 200.", "" },
        });

        add("ledgers", "The most recent ledgers, newest first.", {
            { "limit", "uint32", "How many", false, "1 to 200.", "" },
        });

        add("assets", "Issued assets, and how widely they are held.", {
            { "code", "text", "Asset code", false, "Leave both empty to list everything.", "" },
            { "issuer", "address", "Issuer", false, "", "" },
            { "limit", "uint32", "How many", false, "1 to 200.", "" },
        });

        add("claimableBalances",
            "Balances waiting for somebody to claim them.", {
            { "claimant", "address", "Claimable by", true,
              "The account allowed to claim it. A claimable balance is the only "
              "way to send an asset to somebody who has not trusted it yet — "
              "and the id it comes back with is the only handle on it, because "
              "nothing else identifies one.", "" },
            { "limit", "uint32", "How many", false, "1 to 200.", "" },
        });

        add("orderBook", "The book between two assets: bids and asks.", {
            { "selling", "asset", "Selling", true,
              "\"native\" or CODE:GISSUER. Both sides are required — Horizon "
              "refuses a half-specified book.", "native" },
            { "buying", "asset", "Buying", true, "", "" },
            { "limit", "uint32", "Depth", false,
              "How deep to look. This is depth, not paging: a book has no next "
              "page.", "" },
        });

        add("feeStats", "What transactions have actually been paying lately.", {});

        return t;
    }();
    return table;
}

const QuerySpec* querySpec(const QString& name)
{
    for (const QuerySpec& spec : queryTable())
        if (name == QLatin1String(spec.name))
            return &spec;
    return nullptr;
}

QString queryText(const QVariantMap& spec, const char* key)
{
    return spec.value(QString::fromLatin1(key)).toString().trimmed();
}

int queryLimit(const QVariantMap& spec, int fallback)
{
    const QString raw = queryText(spec, "limit");
    if (raw.isEmpty())
        return fallback;
    bool ok = false;
    const int value = raw.toInt(&ok);
    if (!ok || value < 1)
        return fallback;
    return qMin(value, 200);            // Horizon's own ceiling
}

bool queryFlag(const QVariantMap& spec, const char* key)
{
    return queryText(spec, key).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
}

/**
 * A row, with the cells rendered as they are inserted.
 *
 * Takes QVariant rather than QString so a cell that is already a number or
 * a float needs no conversion at every call site; the table is text by the
 * time it reaches QML either way.
 */
QVariantMap makeRow(std::initializer_list<QPair<const char*, QVariant>> cells)
{
    QVariantMap out;
    for (const auto& cell : cells)
        out.insert(QString::fromLatin1(cell.first), cell.second.toString());
    return out;
}

QVariant columnsOf(std::initializer_list<const char*> names)
{
    QStringList out;
    for (const char* name : names)
        out.append(QString::fromLatin1(name));
    return QVariant(out);
}

/**
 * The SDK's Boolean is an Optional<bool>, and absent is not false.
 *
 * A transaction whose success field never arrived is not a failed transaction;
 * reading it as one would put a red mark against something that simply was not
 * reported.
 */
QString tristate(Boolean value)
{
    if (value.isNull())
        return QStringLiteral("\u2014");
    return value ? QStringLiteral("yes") : QStringLiteral("no");
}

QString assetLabel(Asset* asset)
{
    // Borrowed: an OfferResponse owns the assets it hands out, and they die
    // with it.
    return asset ? AssetRegistry::canonicalFormOf(asset) : QStringLiteral("\u2014");
}

QVariantMap tableOf(const QVariant& columns, const QVariantList& rows)
{
    QVariantMap out;
    out.insert(QStringLiteral("columns"), columns);
    out.insert(QStringLiteral("rows"), rows);
    out.insert(QStringLiteral("count"), rows.size());
    return out;
}

} // namespace

QStringList HorizonQml::supportedQueries() const
{
    QStringList names;
    for (const QuerySpec& spec : queryTable())
        names.append(QString::fromLatin1(spec.name));
    return names;
}

QString HorizonQml::describeQuery(const QString& query) const
{
    const QuerySpec* spec = querySpec(query);
    return spec ? QString::fromUtf8(spec->summary) : QString();
}

QVariantList HorizonQml::fieldsForQuery(const QString& query) const
{
    QVariantList out;
    const QuerySpec* spec = querySpec(query);
    if (!spec)
        return out;
    for (const QueryField& field : spec->fields) {
        QVariantMap entry;
        entry.insert(QStringLiteral("name"), QString::fromLatin1(field.name));
        entry.insert(QStringLiteral("kind"), QString::fromLatin1(field.kind));
        entry.insert(QStringLiteral("label"), QString::fromLatin1(field.label));
        entry.insert(QStringLiteral("required"), field.required);
        entry.insert(QStringLiteral("note"), QString::fromUtf8(field.note));
        entry.insert(QStringLiteral("demo"), QString::fromUtf8(field.demo));
        out.append(entry);
    }
    return out;
}

int HorizonQml::runQuery(const QVariantMap& spec)
{
    const int id = nextRequestId();
    if (!m_pending.contains(id))
        return id;              // cancelled as it started (nextRequestId())
    const QString query = queryText(spec, "query");

    if (!querySpec(query)) {
        failWith(id, QStringLiteral("validation"),
                 QStringLiteral("unknown query: ") + query);
        return id;
    }
    Server* srv = server();
    if (!srv) {
        failWith(id, QStringLiteral("validation"),
                 QStringLiteral("invalid Horizon endpoint: ") + m_endpoint);
        return id;
    }

    const int limit = queryLimit(spec, 20);

    // Several builders throw on bad input rather than returning an error, and
    // an exception must never cross into QML.
    try {
        if (query == QLatin1String("network")) {
            RootResponse* response = srv->root().execute();
            QPointer<RootResponse> guard(response);
            wire(id, response, [guard]() -> QVariantMap {
                if (!guard) throw std::runtime_error("the reply was discarded");
                QVariantList rows;
                rows.append(makeRow({ { "field", QStringLiteral("network") },
                                      { "value", guard->getNetworkPassphrase() } }));
                rows.append(makeRow({ { "field", QStringLiteral("horizon") },
                                      { "value", guard->getHorizonVersion() } }));
                rows.append(makeRow({ { "field", QStringLiteral("core") },
                                      { "value", guard->getStellarCoreVersion() } }));
                rows.append(makeRow({ { "field", QStringLiteral("protocol") },
                                      { "value", QString::number(guard->getCurrentProtocolVersion()) } }));
                rows.append(makeRow({ { "field", QStringLiteral("latest ledger") },
                                      { "value", QString::number(guard->getHistoryLatestLedger()) } }));
                return tableOf(columnsOf({ "field", "value" }), rows);
            }, &HorizonQml::queried);

        } else if (query == QLatin1String("account")) {
            QScopedPointer<KeyPair> key(KeyPair::fromAccountId(queryText(spec, "account")));
            AccountResponse* response = srv->accounts().account(key.data());
            QPointer<AccountResponse> guard(response);
            wire(id, response, [guard]() -> QVariantMap {
                if (!guard) throw std::runtime_error("the reply was discarded");
                QVariantList rows;
                auto balances = guard->getBalances();
                for (auto& line : balances) {
                    rows.append(makeRow({
                        // canonicalAsset(), not a second copy of the same
                        // ternary. The copy that used to be here handled
                        // native and credit assets and not pool shares, and a
                        // pool-share line carries no asset code and no issuer
                        // — so getAssetIssuer() decoded an empty strkey and
                        // threw, from inside a lambda Qt calls off the network
                        // reply with no catch above it anywhere. Any account
                        // that had ever deposited into a liquidity pool
                        // terminated the app when you looked at it.
                        { "asset", canonicalAsset(line) },
                        { "balance", line.getBalance() },
                        { "limit", line.getLimit().isEmpty() ? QStringLiteral("\u2014")
                                                             : line.getLimit() } }));
                }
                QVariantMap out = tableOf(columnsOf({ "asset", "balance", "limit" }), rows);
                out.insert(QStringLiteral("sequence"),
                           QString::number(guard->getSequenceNumber()));
                return out;
            }, &HorizonQml::queried);

        } else if (query == QLatin1String("transaction")) {
            TransactionResponse* response =
                srv->transactions().transaction(queryText(spec, "hash"));
            QPointer<TransactionResponse> guard(response);
            wire(id, response, [guard]() -> QVariantMap {
                if (!guard) throw std::runtime_error("the reply was discarded");
                QVariantList rows;
                rows.append(makeRow({ { "field", QStringLiteral("hash") },
                                      { "value", guard->getHash() } }));
                rows.append(makeRow({ { "field", QStringLiteral("successful") },
                                      { "value", tristate(guard->isSuccessful()) } }));
                rows.append(makeRow({ { "field", QStringLiteral("ledger") },
                                      { "value", QString::number(guard->getLedger()) } }));
                rows.append(makeRow({ { "field", QStringLiteral("closed at") },
                                      { "value", guard->getCreatedAt() } }));
                rows.append(makeRow({ { "field", QStringLiteral("fee charged") },
                                      { "value", QString::number(guard->getFeeCharged()) } }));
                rows.append(makeRow({ { "field", QStringLiteral("operations") },
                                      { "value", QString::number(guard->getOperationCount()) } }));
                return tableOf(columnsOf({ "field", "value" }), rows);
            }, &HorizonQml::queried);

        } else if (query == QLatin1String("transactionsForAccount")) {
            QScopedPointer<KeyPair> key(KeyPair::fromAccountId(queryText(spec, "account")));
            auto* page = srv->transactions()
                             .forAccount(key.data())
                             .includeFailed(queryFlag(spec, "includeFailed"))
                             .limit(limit)
                             .order(RequestBuilder::Order::DESC)
                             .execute();
            QPointer<Page<TransactionResponse>> guard(page);
            wire(id, page, [guard]() -> QVariantMap {
                if (!guard) throw std::runtime_error("the reply was discarded");
                QVariantList rows;
                for (int i = 0; i < guard->size(); ++i) {
                    TransactionResponse* t = guard->at(i);
                    if (!t) continue;
                    rows.append(makeRow({
                        { "hash", t->getHash().left(12) },
                        { "ok", tristate(t->isSuccessful()) },
                        { "ledger", QString::number(t->getLedger()) },
                        { "ops", QString::number(t->getOperationCount()) },
                        { "when", t->getCreatedAt() } }));
                }
                return tableOf(
                    columnsOf({ "hash", "ok", "ledger", "ops", "when" }), rows);
            }, &HorizonQml::queried);

        } else if (query == QLatin1String("operationsForAccount")
                   || query == QLatin1String("paymentsForAccount")) {
            QScopedPointer<KeyPair> key(KeyPair::fromAccountId(queryText(spec, "account")));
            OperationPage* page = nullptr;
            if (query == QLatin1String("operationsForAccount")) {
                page = srv->operations()
                           .forAccount(key.data())
                           .includeFailed(queryFlag(spec, "includeFailed"))
                           .limit(limit)
                           .order(RequestBuilder::Order::DESC)
                           .execute();
            } else {
                page = srv->payments()
                           .forAccount(key.data())
                           .limit(limit)
                           .order(RequestBuilder::Order::DESC)
                           .execute();
            }
            QPointer<OperationPage> guard(page);
            wire(id, page, [guard]() -> QVariantMap {
                if (!guard) throw std::runtime_error("the reply was discarded");
                QVariantList rows;
                for (int i = 0; i < guard->size(); ++i) {
                    OperationResponse* op = guard->at(i);
                    if (!op) continue;
                    rows.append(makeRow({
                        { "type", op->getType() },
                        { "ok", tristate(op->isTransactionSuccessful()) },
                        { "transaction", op->getTransactionHash().left(12) },
                        { "when", op->getCreatedAt() } }));
                }
                return tableOf(
                    columnsOf({ "type", "ok", "transaction", "when" }), rows);
            }, &HorizonQml::queried);

        } else if (query == QLatin1String("offersForAccount")) {
            QScopedPointer<KeyPair> key(KeyPair::fromAccountId(queryText(spec, "account")));
            auto* page = srv->offers().forAccount(key.data()).limit(limit).execute();
            QPointer<Page<OfferResponse>> guard(page);
            wire(id, page, [guard]() -> QVariantMap {
                if (!guard) throw std::runtime_error("the reply was discarded");
                QVariantList rows;
                for (int i = 0; i < guard->size(); ++i) {
                    OfferResponse* offer = guard->at(i);
                    if (!offer) continue;
                    // The exact fraction, not the decimal string: a Stellar
                    // price IS a fraction and the decimal has already lost it.
                    Price exact = offer->getPriceR();
                    rows.append(makeRow({
                        { "id", QString::number(offer->getId()) },
                        { "selling", assetLabel(offer->getSelling()) },
                        { "buying", assetLabel(offer->getBuying()) },
                        { "amount", offer->getAmount() },
                        { "price", QStringLiteral("%1/%2")
                                       .arg(exact.getNumerator())
                                       .arg(exact.getDenominator()) } }));
                }
                return tableOf(
                    columnsOf({ "id", "selling", "buying", "amount", "price" }), rows);
            }, &HorizonQml::queried);

        } else if (query == QLatin1String("claimableBalances")) {
            auto* page = srv->claimableBalances()
                             .forClaimant(queryText(spec, "claimant"))
                             .limit(limit).execute();
            QPointer<Page<ClaimableBalanceResponse>> guard(page);
            wire(id, page, [guard]() -> QVariantMap {
                if (!guard) throw std::runtime_error("the reply was discarded");
                QVariantList rows;
                for (int i = 0; i < guard->size(); ++i) {
                    ClaimableBalanceResponse* balance = guard->at(i);
                    if (!balance) continue;
                    rows.append(makeRow({
                        // The id is the whole point of this query: it is
                        // assigned by the network, appears in the transaction
                        // result metadata this SDK cannot decode, and is the
                        // only handle the three claimable-balance operations
                        // will accept.
                        { "id", balance->getID() },
                        { "asset", balance->getAssetString() },
                        { "amount", balance->getAmount() },
                        { "sponsor", balance->getSponsor() },
                        { "claimants",
                          QString::number(balance->getClaimants().size()) } }));
                }
                return tableOf(
                    columnsOf({ "id", "asset", "amount", "sponsor", "claimants" }),
                    rows);
            }, &HorizonQml::queried);

        } else if (query == QLatin1String("ledgers")) {
            auto* page = srv->ledgers().order(RequestBuilder::Order::DESC)
                             .limit(limit).execute();
            QPointer<Page<LedgerResponse>> guard(page);
            wire(id, page, [guard]() -> QVariantMap {
                if (!guard) throw std::runtime_error("the reply was discarded");
                QVariantList rows;
                for (int i = 0; i < guard->size(); ++i) {
                    LedgerResponse* ledger = guard->at(i);
                    if (!ledger) continue;
                    rows.append(makeRow({
                        { "sequence", QString::number(ledger->getSequence()) },
                        { "closed", ledger->getClosedAt() },
                        { "transactions", QStringLiteral("%1 ok, %2 failed")
                                              .arg(ledger->getSuccessfulTransactionCount())
                                              .arg(ledger->getFailedTransactionCount()) },
                        { "ops", QString::number(ledger->getOperationCount()) },
                        { "protocol", QString::number(ledger->getProtocolVersion()) } }));
                }
                return tableOf(
                    columnsOf({ "sequence", "closed", "transactions", "ops", "protocol" }), rows);
            }, &HorizonQml::queried);

        } else if (query == QLatin1String("assets")) {
            AssetsRequestBuilder builder = srv->assets();
            if (!queryText(spec, "code").isEmpty())
                builder.assetCode(queryText(spec, "code"));
            if (!queryText(spec, "issuer").isEmpty())
                builder.assetIssuer(queryText(spec, "issuer"));
            auto* page = builder.limit(limit).execute();
            QPointer<Page<AssetResponse>> guard(page);
            wire(id, page, [guard]() -> QVariantMap {
                if (!guard) throw std::runtime_error("the reply was discarded");
                QVariantList rows;
                for (int i = 0; i < guard->size(); ++i) {
                    AssetResponse* asset = guard->at(i);
                    if (!asset) continue;
                    // Horizon 29 no longer sends "amount" and "num_accounts"
                    // (getAmount/getNumAccounts): the table read "" and 0 for
                    // every asset. The authorized balances and accounts are
                    // what those two were.
                    rows.append(makeRow({
                        { "asset", asset->getAssetCode() + QLatin1Char(':')
                                   + asset->getAssetIssuer() },
                        { "supply", asset->getBalances().authorized() },
                        { "holders", QString::number(asset->getAccounts().authorized()) } }));
                }
                return tableOf(
                    columnsOf({ "asset", "supply", "holders" }), rows);
            }, &HorizonQml::queried);

        } else if (query == QLatin1String("orderBook")) {
            Asset* selling = m_assets ? m_assets->assetFor(queryText(spec, "selling")) : nullptr;
            Asset* buying = m_assets ? m_assets->assetFor(queryText(spec, "buying")) : nullptr;
            if (!selling || !buying)
                throw std::runtime_error("both sides of the book must be valid assets");
            OrderBookResponse* response = srv->orderBook()
                                              .sellingAsset(selling)
                                              .buyingAsset(buying)
                                              .limit(limit)
                                              .execute();
            QPointer<OrderBookResponse> guard(response);
            wire(id, response, [guard]() -> QVariantMap {
                if (!guard) throw std::runtime_error("the reply was discarded");
                QVariantList rows;
                auto bids = guard->getBids();
                for (auto& bid : bids)
                    rows.append(makeRow({ { "side", QStringLiteral("bid") },
                                          { "amount", bid.getAmount() },
                                          { "price", bid.getPrice() } }));
                auto asks = guard->getAsks();
                for (auto& ask : asks)
                    rows.append(makeRow({ { "side", QStringLiteral("ask") },
                                          { "amount", ask.getAmount() },
                                          { "price", ask.getPrice() } }));
                return tableOf(columnsOf({ "side", "amount", "price" }), rows);
            }, &HorizonQml::queried);

        } else if (query == QLatin1String("feeStats")) {
            // Named operationFeeStats() but it asks /fee_stats; the SDK's own
            // doc comment naming the old endpoint is stale.
            FeeStatsResponse* response = srv->operationFeeStats().execute();
            QPointer<FeeStatsResponse> guard(response);
            wire(id, response, [guard]() -> QVariantMap {
                if (!guard) throw std::runtime_error("the reply was discarded");
                QVariantList rows;
                rows.append(makeRow({ { "field", QStringLiteral("base fee") },
                                      { "value", guard->getLastLedgerBaseFee() } }));
                rows.append(makeRow({ { "field", QStringLiteral("ledger capacity used") },
                                      { "value", QString::number(guard->getLedgerCapacityUsage()) } }));
                rows.append(makeRow({ { "field", QStringLiteral("charged, median") },
                                      { "value", guard->getFeeCharged().getP50() } }));
                rows.append(makeRow({ { "field", QStringLiteral("charged, 99th") },
                                      { "value", guard->getFeeCharged().getP99() } }));
                rows.append(makeRow({ { "field", QStringLiteral("last ledger") },
                                      { "value", guard->getLastLedger() } }));
                return tableOf(columnsOf({ "field", "value" }), rows);
            }, &HorizonQml::queried);

        } else {
            failWith(id, QStringLiteral("validation"),
                     QStringLiteral("query not implemented: ") + query);
        }
    } catch (const std::exception& e) {
        failWith(id, QStringLiteral("validation"), QString::fromUtf8(e.what()));
    } catch (...) {
        failWith(id, QStringLiteral("unknown"),
                 QStringLiteral("the query could not be built"));
    }

    return id;
}

int HorizonQml::loadAccount(const QString& accountId)
{
    const int id = nextRequestId();
    if (!m_pending.contains(id))
        return id;              // cancelled as it started (nextRequestId())

    if (!isAccountId(accountId)) {
        failWith(id, QStringLiteral("validation"),
                 QStringLiteral("not a valid account id (expected G...)"));
        return id;
    }
    Server* srv = server();
    if (!srv) {
        failWith(id, QStringLiteral("validation"),
                 QStringLiteral("invalid Horizon endpoint: ") + m_endpoint);
        return id;
    }

    AccountResponse* response = nullptr;
    try {
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(accountId));
        response = srv->accounts().account(key.data());
    } catch (const std::exception& e) {
        failWith(id, QStringLiteral("validation"), QString::fromUtf8(e.what()));
        return id;
    }
    if (!response) {
        failWith(id, QStringLiteral("unknown"), QStringLiteral("no response object"));
        return id;
    }

    QPointer<AccountResponse> account(response);
    wire(id, response, [account]() -> QVariantMap {
        if (!account)
            throw std::runtime_error("the reply was discarded");

        QVariantMap out;
        out.insert(QStringLiteral("id"), account->accountID());
        // int64 as string: a sequence number above 2^53 does not survive a
        // JavaScript Number, and sequence numbers reach that range.
        out.insert(QStringLiteral("sequence"),
                   QString::number(account->getSequenceNumber()));
        // None follows INT64_MAX: left out rather than shown as INT64_MIN.
        if (account->getSequenceNumber() < std::numeric_limits<qint64>::max())
            out.insert(QStringLiteral("nextSequence"),
                       QString::number(account->getIncrementedSequenceNumber()));
        out.insert(QStringLiteral("subentryCount"), account->getSubentryCount());
        out.insert(QStringLiteral("homeDomain"), account->getHomeDomain());
        out.insert(QStringLiteral("sponsor"), account->getSponsor());
        out.insert(QStringLiteral("numSponsoring"),
                   QString::number(account->getNumSponsoring()));
        out.insert(QStringLiteral("numSponsored"),
                   QString::number(account->getNumSponsored()));

        QVariantList balances;
        auto lines = account->getBalances();
        for (auto& line : lines) {
            QVariantMap b;
            b.insert(QStringLiteral("asset"), canonicalAsset(line));
            b.insert(QStringLiteral("assetType"), line.getAssetType());
            b.insert(QStringLiteral("balance"), line.getBalance());
            b.insert(QStringLiteral("limit"), line.getLimit());
            b.insert(QStringLiteral("buyingLiabilities"), line.getBuyingLiabilities());
            b.insert(QStringLiteral("sellingLiabilities"), line.getSellingLiabilities());
            // Only when Horizon sent is_authorized: it sends none for XLM,
            // where authorization does not apply. It read as authorized:
            // false, a definite "frozen" for the one balance that cannot be;
            // js leaves it undefined.
            if (line.hasAuthorized())
                b.insert(QStringLiteral("authorized"), line.getAuthorized());
            balances.append(b);
        }
        out.insert(QStringLiteral("balances"), balances);

        QVariantList signers;
        const auto keys = account->getSigners();
        for (const auto& signer : keys) {
            QVariantMap s;
            s.insert(QStringLiteral("key"), signer.getKey());
            s.insert(QStringLiteral("type"), signer.getType());
            s.insert(QStringLiteral("weight"), signer.getWeight());
            signers.append(s);
        }
        out.insert(QStringLiteral("signers"), signers);

        auto thresholds = account->getThresholds();
        QVariantMap t;
        t.insert(QStringLiteral("low"), thresholds.getLowThreshold());
        t.insert(QStringLiteral("med"), thresholds.getMedThreshold());
        t.insert(QStringLiteral("high"), thresholds.getHighThreshold());
        out.insert(QStringLiteral("thresholds"), t);

        auto flags = account->getFlags();
        QVariantMap f;
        f.insert(QStringLiteral("authRequired"), flags.getAuthRequired());
        f.insert(QStringLiteral("authRevocable"), flags.getAuthRevocable());
        f.insert(QStringLiteral("authImmutable"), flags.getAuthImmutable());
        f.insert(QStringLiteral("authClawbackEnabled"), flags.getAuthClawbackEnabled());
        out.insert(QStringLiteral("flags"), f);

        return out;
    }, &HorizonQml::accountLoaded);

    return id;
}

int HorizonQml::fundAccount(const QString& accountId)
{
    const int id = nextRequestId();
    if (!m_pending.contains(id))
        return id;              // cancelled as it started: friendbot is not asked

    if (!isAccountId(accountId)) {
        failWith(id, QStringLiteral("validation"),
                 QStringLiteral("not a valid account id (expected G...)"));
        return id;
    }
    // Friendbot is a test-network service: the one set in friendbotUrl, or the
    // endpoint's (see friendbotUrlFor). An endpoint with none, the public
    // network's, is refused with what to do rather than asked for a 404.
    const QString friendbot = m_friendbotUrl.isEmpty() ? friendbotUrlFor(m_endpoint) : m_friendbotUrl;
    if (friendbot.isEmpty()) {
        failWith(id, QStringLiteral("validation"),
                 QStringLiteral("friendbot only exists on test networks, and none is known for ") +
                 m_endpoint + QStringLiteral("; for another test network set friendbotUrl"));
        return id;
    }

    QUrl url(friendbot);
    // Added to the query friendbotUrl may carry (a token, say), as js sets
    // addr on its URL: setQuery() replaced that query with addr alone.
    QUrlQuery query(url);
    query.removeAllQueryItems(QStringLiteral("addr"));
    query.addQueryItem(QStringLiteral("addr"), accountId);
    url.setQuery(query);

    QNetworkReply* reply = directManager()->get(QNetworkRequest(url));
    // Given up on after Response::replyTimeout(), as every Horizon request
    // is: a friendbot that took the connection and never answered left the
    // request pending for ever.
    auto timedOut = std::make_shared<bool>(false);
    QTimer* timer = new QTimer(reply);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, reply, [reply, timedOut]() {
        if (reply->isRunning()) {
            *timedOut = true;
            reply->abort();
        }
    });
    timer->start(Response::replyTimeout());

    connect(reply, &QNetworkReply::finished, this, [this, id, reply, accountId, timedOut]() {
        reply->deleteLater();
        if (!m_pending.contains(id))
            return;

        const QByteArray body = reply->readAll();
        const QVariant statusAttribute = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        const int status = statusAttribute.isValid() ? statusAttribute.toInt() : 0;

        if (*timedOut || reply->error() == QNetworkReply::TimeoutError) {
            // Friendbot may still fund the account: it was "connection",
            // which says the request never went out.
            failNow(id, QStringLiteral("timeout"),
                    QStringLiteral("friendbot did not answer within %1 ms; the account may still be funded")
                        .arg(Response::replyTimeout()));
            return;
        }

        // Only a 200 is friendbot's answer. Any other status, or a 200 whose
        // body did not arrive whole, used to fall through to "funded".
        if (status != 200 || reply->error() != QNetworkReply::NoError) {
            QString message = reply->errorString();
            // Friendbot answers a request for an already-funded account with a
            // JSON problem document; its detail says more than the generic
            // "server replied: Bad Request".
            const QJsonValue detail = QJsonDocument::fromJson(body).object().value(QStringLiteral("detail"));
            if (detail.isString() && !detail.toString().isEmpty())
                message = detail.toString();

            finish(id);
            QString kind = QStringLiteral("badResponse");
            if (status == 0)
                kind = QStringLiteral("connection");
            else if (status == 429)
                kind = QStringLiteral("rateLimited");
            else if (status >= 400 && status < 500)
                kind = QStringLiteral("badRequest");
            QVariantMap error;
            error.insert(QStringLiteral("kind"), kind);
            error.insert(QStringLiteral("status"), status);
            error.insert(QStringLiteral("message"), message);
            emit failed(id, error);
            return;
        }

        // What friendbot answers once it has funded the account:
        // {"successful": true, "hash": "<64 hex digits>", "envelope_xdr": ...},
        // and a ledger when it waited for one. Anything else was reported as
        // funded all the same, with an empty hash and ledger "0".
        QJsonParseError parse;
        const QJsonDocument document = QJsonDocument::fromJson(body, &parse);
        const QJsonObject answer = document.object();
        const QJsonValue hash = answer.value(QStringLiteral("hash"));
        bool hex = hash.isString() && hash.toString().size() == 64;
        for (const QChar c : hash.toString())
            hex = hex && ((c >= QLatin1Char('0') && c <= QLatin1Char('9'))
                          || (c >= QLatin1Char('a') && c <= QLatin1Char('f'))
                          || (c >= QLatin1Char('A') && c <= QLatin1Char('F')));
        if (parse.error != QJsonParseError::NoError || !document.isObject() || !hex) {
            failNow(id, QStringLiteral("badResponse"),
                    QStringLiteral("friendbot's answer names no transaction hash"));
            return;
        }
        const QJsonValue successful = answer.value(QStringLiteral("successful"));
        if (!successful.isBool() || !successful.toBool()) {
            failNow(id, QStringLiteral("badResponse"),
                    QStringLiteral("friendbot did not report transaction %1 successful").arg(hash.toString()));
            return;
        }
        QVariantMap out;
        out.insert(QStringLiteral("accountId"), accountId);
        out.insert(QStringLiteral("hash"), hash.toString());
        // Only when friendbot says: today's does not wait for the ledger.
        const QJsonValue ledger = answer.value(QStringLiteral("ledger"));
        if (!ledger.isUndefined()) {
            try {
                out.insert(QStringLiteral("ledger"),
                           QString::number(ResponseConverters::exactInteger(
                               ledger, QStringLiteral("ledger"), 1, std::numeric_limits<quint32>::max())));
            } catch (const std::exception& e) {
                failNow(id, QStringLiteral("badResponse"), QString::fromUtf8(e.what()));
                return;
            }
        }
        finish(id);
        emit funded(id, out);
    });

    return id;
}

int HorizonQml::submit(const QString& envelopeXdrBase64, const QString& networkPassphrase)
{
    const int id = nextRequestId();
    // Cancelled as it started, from a handler of that pendingChanged: never
    // posted, which is what a cancel does to a submit not yet posted. It used
    // to go on and post, and its outcome, with nobody left to report it to,
    // was dropped; its Server stayed until this object died.
    if (!m_pending.contains(id))
        return id;

    AbstractTransaction* transaction = nullptr;
    // What a failure that may have reached Horizon reports it under: the
    // envelope's hash on the network it was signed for. It is the one
    // Horizon files it under once checkNetwork() has found that network to
    // be the endpoint's, which it does before the transaction is posted.
    QString hash;
    try {
        transaction = AbstractTransaction::fromEnvelopeXdr(envelopeXdrBase64,
                                                           networkFor(networkPassphrase));
        if (transaction)
            hash = transaction->hashHex().toLower();
    } catch (const std::exception& e) {
        delete transaction;
        failSubmit(id, failureOf(QStringLiteral("validation"), QString::fromUtf8(e.what())),
                   false, true);
        return id;
    }
    if (!transaction) {
        failSubmit(id, failureOf(QStringLiteral("validation"),
                                 QStringLiteral("could not read the envelope XDR")),
                   false, true);
        return id;
    }

    // A Server of its own rather than the shared one (see submit() in the
    // header): the only POST its manager makes is this submit's, so a cancel
    // can tell whether it was posted, and an endpoint change, which drops the
    // shared Server, cannot delete this one under it.
    Server* srv = nullptr;
    try {
        srv = new Server(m_endpoint);
        srv->setParent(this);
    } catch (const std::exception&) {
        srv = nullptr;
    }
    if (!srv) {
        delete transaction;
        failSubmit(id, failureOf(QStringLiteral("validation"),
                                 QStringLiteral("invalid Horizon endpoint: ") + m_endpoint),
                   false, true);
        return id;
    }
    // Its POST is watched from the moment it is made, wherever the Server
    // makes it: at once (post()), or from a SEP-29 lookup's answer; so is
    // every send of it Qt makes by itself after the first.
    srv->manager().installEventFilter(new UploadWatch(srv, [this, id](bool again) {
        const auto submit = m_submits.find(id);
        if (submit == m_submits.end())
            return;
        submit->written = true;
        if (again)
            submit->resent = true;
    }));
    // It goes with its Server until it is handed to it (post()).
    transaction->setParent(srv);

    Submit record;
    record.server = srv;
    record.transaction = transaction;
    record.passphrase = networkPassphrase;
    record.hash = hash;
    m_submits.insert(id, record);

    checkNetwork(id);
    return id;
}

void HorizonQml::checkNetwork(int requestId)
{
    const auto submit = m_submits.find(requestId);
    // Remembered from this endpoint's root, and the one given: nothing to ask.
    if (!m_networkPassphrase.isEmpty() && submit->passphrase == m_networkPassphrase) {
        post(requestId);
        return;
    }

    // Not known yet, or remembered as another network, which can be stale
    // (the Horizon behind the same URL may serve another network since): so
    // asked, and only a fresh answer refuses the passphrase. Asked before
    // anything else, the SEP-29 lookups included: those read the accounts of
    // the network the endpoint serves, which only mean something once it is
    // known to be the one the transaction is for.
    RootResponse* root = nullptr;
    try {
        root = submit->server->root().execute();
    } catch (const std::exception& e) {
        failSubmit(requestId, failureOf(QStringLiteral("validation"), QString::fromUtf8(e.what())),
                   false, true);
        return;
    }
    QPointer<RootResponse> guard(root);
    // The submit is still to be posted. A cancel drops it at once, and its
    // Server (this lookup included) later: an answer delivered in between,
    // one already queued when the cancel came or one read in an event loop
    // nested in the slot that cancelled, must post nothing.
    auto current = [this, requestId]() {
        return m_pending.contains(requestId) && m_submits.contains(requestId);
    };

    connect(root, &Response::ready, this, [this, requestId, guard, current]() {
        if (!guard || !current()) {
            if (guard) guard->deleteLater();
            return;
        }
        const auto submit = m_submits.find(requestId);
        const QString given = submit->passphrase;
        const QString served = guard->getNetworkPassphrase();
        guard->deleteLater();
        if (served.isEmpty()) {
            failSubmit(requestId, failureOf(QStringLiteral("badResponse"),
                                            QStringLiteral("not submitted: Horizon's root (GET /) names no "
                                                           "network passphrase, and the transaction's hash "
                                                           "depends on it")),
                       false, false);
            return;
        }
        m_networkPassphrase = served;
        if (served != given) {
            failSubmit(requestId, failureOf(QStringLiteral("validation"),
                           QStringLiteral("not submitted: Horizon serves the network \"%1\", not the one "
                                          "given (\"%2\"); a network files a transaction under the hash its "
                                          "own passphrase gives it, so the transaction would be looked up "
                                          "under a hash that network never gives it")
                               .arg(served, given)),
                       false, false);
            return;
        }
        post(requestId);
    });

    connect(root, &Response::error, this, [this, requestId, guard, current]() {
        if (!guard || !current()) {
            if (guard) guard->deleteLater();
            return;
        }
        QVariantMap error;
        try {
            error = responseError(guard);
        } catch (...) {
            error.clear();
        }
        guard->deleteLater();
        QString why = error.value(QStringLiteral("message")).toString();
        if (why.isEmpty())
            why = QStringLiteral("request failed");
        if (error.value(QStringLiteral("kind")).toString().isEmpty())
            error.insert(QStringLiteral("kind"), QStringLiteral("unknown"));
        // Its own kind, as a SEP-29 lookup's is: a rate limit keeps its
        // Retry-After, a timeout is a timeout.
        error.insert(QStringLiteral("message"),
                     QStringLiteral("not submitted: the endpoint's network could not be read "
                                    "(GET /), and the transaction's hash depends on it: %1").arg(why));
        failSubmit(requestId, error, false, false);
    });
}

void HorizonQml::post(int requestId)
{
    const auto submit = m_submits.find(requestId);
    Server* srv = submit->server;
    AbstractTransaction* transaction = submit->transaction;

    SubmitTransactionResponse* response = nullptr;
    try {
        // The memo-required check (SEP-29) is left on. It costs a round trip per
        // destination, and it is exactly the sort of thing this app exists to
        // demonstrate: paying a custodial address without a memo loses the money.
        response = srv->submitTransaction(transaction, false);
    } catch (const std::exception& e) {
        // Thrown before the POST is made; told by what was made all the same.
        failSubmit(requestId, failureOf(QStringLiteral("validation"), QString::fromUtf8(e.what())),
                   postOf(srv) != nullptr, true);
        return;
    }
    if (!response) {
        failSubmit(requestId, failureOf(QStringLiteral("unknown"), QStringLiteral("no response object")),
                   postOf(srv) != nullptr, true);
        return;
    }

    // The response keeps a pointer to the transaction, so the transaction has to
    // outlive it. Parenting it to the response ties the two lifetimes together.
    transaction->setParent(response);
    // And the response to its Server, which is released once the submit is
    // reported (finish()), or with this object: an unfinished one leaked.
    response->setParent(srv);
    submit->response = response;
    wireSubmit(requestId, response);
}

void HorizonQml::wireSubmit(int requestId, SubmitTransactionResponse* response)
{
    QPointer<SubmitTransactionResponse> guard(response);

    connect(response, &Response::ready, this, [this, requestId, guard]() {
        if (!m_pending.contains(requestId) || !m_submits.contains(requestId)) {
            if (guard) guard->deleteLater();
            return;         // cancelled while in flight, and reported as that
        }
        // Read apart from the emit, inside a catch, as wire() does: the SDK's
        // getters throw on data they cannot read, and this runs from Qt's
        // signal dispatch.
        QVariantMap out;
        bool readOk = false;
        QString problem;
        try {
            if (!guard)
                throw std::runtime_error("the reply was discarded");
            out.insert(QStringLiteral("successful"), guard->isSuccess());
            out.insert(QStringLiteral("hash"), guard->getHash());
            out.insert(QStringLiteral("ledger"), QString::number(guard->getLedger()));
            out.insert(QStringLiteral("envelopeXdr"), guard->getEnvelopeXdr());
            out.insert(QStringLiteral("resultXdr"), guard->getResultXdr());
            auto extras = guard->getExtras();
            // Horizon sends no result codes with a 200 (only with a refusal),
            // and Extras::getTransactionResultCode() is the envelope, whatever
            // its name says: this key was always "". It is read from the
            // result: its code follows the int64 fee charged.
            QString txResultCode;
            const QByteArray result = QByteArray::fromBase64Encoding(
                guard->getResultXdr().toLatin1(), QByteArray::AbortOnBase64DecodingErrors).decoded;
            if (result.size() >= 12) {
                const qint32 code = qFromBigEndian<qint32>(reinterpret_cast<const uchar*>(result.constData() + 8));
                if (code == 0)
                    txResultCode = QStringLiteral("tx_success");
                else if (code == 1)
                    txResultCode = QStringLiteral("tx_fee_bump_inner_success");
            }
            out.insert(QStringLiteral("txResultCode"), txResultCode);
            out.insert(QStringLiteral("opResultCodes"),
                       QVariant(extras.getResultCodes().getOperationsResultCodes()));
            readOk = true;
        } catch (const std::exception& e) {
            problem = QString::fromUtf8(e.what());
        } catch (...) {
            problem = QStringLiteral("unknown error");
        }
        if (guard) guard->deleteLater();
        if (readOk) {
            finish(requestId);
            emit submitted(requestId, out);
            return;
        }
        // Horizon answered for it: it was received, and may have been applied.
        failSubmit(requestId, failureOf(QStringLiteral("badResponse"),
                                        QStringLiteral("Horizon's reply could not be read: ") + problem),
                   true, false);
    });

    connect(response, &Response::error, this, [this, requestId, guard]() {
        const auto submit = m_submits.constFind(requestId);
        if (!m_pending.contains(requestId) || submit == m_submits.constEnd()) {
            if (guard) guard->deleteLater();
            return;         // cancelled while in flight, and reported as that
        }
        // Before its POST (SEP-29 stopped it, or a lookup failed) nothing was
        // sent. After, it is known not to have left only when its connection
        // never opened and not a byte of it was written: a timeout, a
        // connection lost midway, a 5xx or an answer that cannot be read may
        // all come after Horizon read it, and so may a refused connection,
        // when it was Qt sending the POST again after the one that carried
        // it closed (see neverConnected()). (These said neither, and had no
        // hash: only a cancel did. Then a refused resend said "not sent",
        // though Horizon had read the POST whole.)
        const bool neverLeft = !submit->written
                               && guard && neverConnected(guard->lastErrorCode());
        const bool reached = postOf(submit->server) != nullptr && !neverLeft;
        QVariantMap error;
        try {
            if (guard)
                error = responseError(guard);
        } catch (...) {
            error.clear();
        }
        if (error.isEmpty())
            error = failureOf(QStringLiteral("unknown"), QStringLiteral("request failed"));
        if (guard) guard->deleteLater();
        failSubmit(requestId, error, reached, false);
    });
}

void HorizonQml::failSubmit(int requestId, QVariantMap error, bool mayHaveReachedServer,
                            bool queued)
{
    if (!m_pending.contains(requestId))
        return;             // cancelled meanwhile
    const Submit submit = m_submits.value(requestId);
    finish(requestId);
    error.insert(QStringLiteral("mayHaveReachedServer"), mayHaveReachedServer);
    if (mayHaveReachedServer && !submit.hash.isEmpty())
        error.insert(QStringLiteral("hash"), submit.hash);
    // Qt sent the POST again, by itself, after a send that wrote it whole
    // (Qt 5 does, when the connection closes with no answer): whatever ended
    // the submit came of a later send, while Horizon may have read the first
    // and applied it. So not even Horizon's own answer is final: a 429, or
    // the tx_bad_seq Horizon answers a transaction sent again once the first
    // has been applied (before Horizon has taken in its ledger), answers the
    // later send only. The kind and Horizon's codes are kept; `resent` and
    // the message say it, the message naming the hash. (They were reported
    // as final, and a wallet that rebuilt the payment with a fresh sequence
    // number paid twice.)
    if (submit.resent) {
        error.insert(QStringLiteral("resent"), true);
        error.insert(QStringLiteral("message"),
                     notFinal(submit.hash, error.value(QStringLiteral("message")).toString()));
    }
    if (!queued) {
        emit failed(requestId, error);
        return;
    }
    // Queued from inside submit(), which has not returned the id yet (see
    // failWith()).
    QMetaObject::invokeMethod(this, [this, requestId, error]() {
        emit failed(requestId, error);
    }, Qt::QueuedConnection);
}

QSTELLAR_END_NS
