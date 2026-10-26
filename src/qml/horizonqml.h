#ifndef QSTELLAR_HORIZONQML_H
#define QSTELLAR_HORIZONQML_H

#include <QHash>
#include <functional>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "qstellar_namespace.h"

class QNetworkAccessManager;
class QNetworkReply;

QSTELLAR_BEGIN_NS

class AbstractTransaction;
class AssetRegistry;
class Server;
class Response;
class SubmitTransactionResponse;

/**
 * QML facade over the parts of the SDK that talk to a Horizon server.
 *
 * It obeys the same two rules as StellarQml — nothing throws into QML, no SDK
 * pointer crosses the boundary — and adds a third that only applies once there
 * is a network involved:
 *
 * 3. EVERY OUTCOME IS A SIGNAL CARRYING A PLAIN MAP. A UI cannot hold a
 *    Response*: it does not know when the object dies, and the SDK parents
 *    responses to the Server so they outlive the request that made them. So a
 *    call here starts a request, returns an integer id immediately, and later
 *    emits exactly one of accountLoaded / funded / submitted / failed carrying
 *    that id. The Response is consumed and deleted inside this class.
 *
 * Failures are classified rather than flattened to a string. `kind` is one of:
 *
 *   "notFound"     the account does not exist yet (needs funding)
 *   "timeout"      the request ran out of time: Horizon answered 504, or no
 *                  answer came within Response::replyTimeout() (the SDK's
 *                  own limit, STELLAR_QT_REPLY_TIMEOUT; reported as
 *                  "connection" before 1.0.4). For a submit this is the
 *                  dangerous one: the transaction may still land
 *   "rateLimited"  Horizon is throttling; `retryAfter` carries the seconds
 *                  to wait when Horizon said (its Retry-After). For a submit
 *                  with `resent` it answers a POST Qt sent again, and the
 *                  transaction may still land (see failed())
 *   "badRequest"   Horizon rejected the request (4xx other than the above);
 *                  for a submit with `resent`, as "rateLimited"
 *   "badResponse"  Horizon answered something unparseable (5xx, bad JSON, or
 *                  a reply the SDK could not read, such as a balance whose
 *                  issuer is not an account id)
 *   "connection"   the connection failed: it never opened, or it was lost
 *                  before an answer came. For a submit, mayHaveReachedServer
 *                  says whether its POST may have been carried (see
 *                  failed()): one lost after the POST was written may still
 *                  land, and so may one refused when Qt sent it again
 *   "txFailed"     the network rejected the transaction itself; the map then
 *                  also carries txResultCode / opResultCodes / resultXdr.
 *                  For a submit with `resent` it rejected a POST Qt sent
 *                  again, and the first may have been applied (a tx_bad_seq
 *                  most likely means it was): see failed()
 *   "memoRequired" a destination asks for a memo (SEP-29) and the
 *                  transaction has none; nothing was submitted. The map also
 *                  carries `destination` (the G account) and `operationIndex`
 *                  (the first operation paying it)
 *   "validation"   rejected here, before the request went out: for a submit,
 *                  before its transaction did (a submit refused because
 *                  Horizon serves another network has asked Horizon which
 *                  network that is, and nothing else; see submit())
 *   "cancelled"    a submit that cancelAll() or an endpoint change stopped
 *                  after it was posted: it may still be applied (see
 *                  cancelAll())
 *   "unknown"      anything that got past the classifier
 *
 * Every failure of a submit also says whether its transaction may have
 * reached Horizon, and if so under which hash to look it up
 * (`mayHaveReachedServer`, `hash`), and whether Qt sent its POST more than
 * once (`resent`), in which case not even Horizon's own answer is final;
 * see failed().
 *
 * Destroying this object abandons every request in flight, a posted submit
 * included, and reports nothing: there is nothing left to report to. It has
 * no thread of its own, so there is nothing to wait for either; its Servers
 * go with it, and Qt aborts their replies. Do not destroy it while a submit
 * is under way.
 *
 * Registered as the `Horizon` singleton by QStellarQml::registerTypes().
 */
class HorizonQml : public QObject
{
    Q_OBJECT

    /**
     * Horizon base URL. Changing it drops the Server the queries share, and
     * its cache, and cancels every request started against the old one
     * exactly as cancelAll() does: a submit already posted (see submit()) is
     * still reported, once — failed() with kind "cancelled" and its `hash`,
     * or its own end if that came first — and its Server, a different one,
     * lives until then; everything else is forgotten. (A posted submit used
     * to be forgotten too, with the Server it was on deleted under it: a
     * wallet that switched endpoint while a payment was on the wire never
     * heard that it may land.) Requests started afterwards go to the new
     * endpoint. Which network the old one serves is forgotten too: the next
     * submit asks the new one (see submit()).
     */
    Q_PROPERTY(QString endpoint READ endpoint WRITE setEndpoint NOTIFY endpointChanged)
    /**
     * The friendbot fundAccount() asks. Empty (the default): the one of the
     * endpoint's network — SDF's for horizon-testnet.stellar.org, futurenet's
     * for horizon-futurenet.stellar.org, none for horizon.stellar.org — and
     * for any other endpoint <endpoint>/friendbot, where js-stellar-sdk's
     * Horizon.Server.friendbot() asks too (a local quickstart's
     * http://localhost:8000 serves one there). Set it for a friendbot
     * somewhere else; a query it carries is kept, with addr added. It stays
     * set when the endpoint changes.
     */
    Q_PROPERTY(QString friendbotUrl READ friendbotUrl WRITE setFriendbotUrl NOTIFY friendbotUrlChanged)

    /**
     * Requests started whose outcome may still be reported. Bind a busy
     * indicator to this. A submit cancelAll() (or an endpoint change) stopped
     * after it was posted is counted until it is reported, which is on the
     * next pass of the event loop; see cancelAll().
     */
    Q_PROPERTY(int pending READ pending NOTIFY pendingChanged)

public:
    /**
     * `assets` resolves canonical asset strings for the queries that take one.
     * Shared with the rest of the QML layer so an asset named here is the same
     * object the builder used, which is what keeps ownership answerable.
     */
    explicit HorizonQml(AssetRegistry* assets = nullptr, QObject* parent = nullptr);
    ~HorizonQml() override;

    QString endpoint() const { return m_endpoint; }
    void setEndpoint(const QString& endpoint);
    int pending() const { return m_pending.size(); }

    /** Canonical Horizon URL for "testnet" / "public". Empty if unknown. */
    Q_INVOKABLE QString horizonUrlFor(const QString& network) const;

    // ─── Requests ────────────────────────────────────────────────────

    /**
     * GET /accounts/{id}. Emits accountLoaded with, among other fields,
     * `sequence` and `nextSequence` as decimal STRINGS — a ledger sequence is
     * an int64 and JavaScript numbers silently lose precision above 2^53.
     *
     * `nextSequence` is what a transaction being built should use: it is the
     * account's current sequence plus one. Handing the UI the already-incremented
     * value is deliberate — reading the account and forgetting to add one is the
     * single most common way to produce a tx_bad_seq.
     */
    Q_INVOKABLE int loadAccount(const QString& accountId);

    /**
     * Ask friendbot to create and fund a testnet account.
     *
     * This is not an SDK feature and never will be a mainnet one: friendbot
     * exists only on test networks. It lives here because every guided walk
     * through Stellar starts with "now you need an account that exists", and
     * making the user leave the app for a web form to get one is a poor first
     * step.
     *
     * Emits funded(id, {accountId, hash}) once friendbot answers 200 with
     * `"successful": true` and the transaction's hash (and `ledger` too
     * when friendbot names one; today's does not); failed() otherwise,
     * with kind "timeout" when friendbot did not answer within
     * Response::replyTimeout(). Before 1.0.4 there was no time limit, and
     * any 200 was reported as funded, with `ledger` "0" when absent.
     *
     * A G... account only. Friendbot funds a contract (C...) as well, which
     * js-stellar-sdk does through rpc.Server.fundAddress: it asks the RPC
     * for the network's friendbot, then for the funding transaction. This
     * layer's RPC side has no friendbot call and funded() names an account,
     * so a C... is failed() with kind "validation" before friendbot is asked.
     */
    Q_INVOKABLE int fundAccount(const QString& accountId);

    QString friendbotUrl() const { return m_friendbotUrl; }
    void setFriendbotUrl(const QString& url);
    /** The friendbot fundAccount() asks for `endpoint` when friendbotUrl is
     *  empty (see there): empty for the public network, or for a value with
     *  no host. */
    Q_INVOKABLE QString friendbotUrlFor(const QString& endpoint) const;

    /**
     * POST /transactions with a signed envelope, given as base64 XDR.
     *
     * `networkPassphrase` must be the one the envelope was signed for, and
     * the one the endpoint serves. It is needed to rebuild the transaction
     * from its XDR, and it gives the transaction's hash, the one a submit
     * whose outcome is unknown is looked up by (see failed()). An envelope
     * that does not decode, or an empty passphrase, is refused before any
     * request, as "validation".
     *
     * Then, before anything else is asked, the endpoint's network: its
     * network_passphrase, from Horizon's root (GET /). It is asked once per
     * endpoint and remembered until the endpoint changes; a passphrase that
     * differs from the one remembered has Horizon asked once more first (the
     * Horizon behind the same URL may serve another network since). A
     * passphrase that differs from Horizon's own answer is refused, as
     * "validation", and nothing is sent: a network files a transaction under
     * the hash its own passphrase gives it, so the hash worked out from
     * another passphrase is one that network never gives it. A root that
     * cannot be read stops the submit too, reported as what that lookup met
     * ("connection", "timeout", "rateLimited", "badResponse", "badRequest"),
     * the message saying so; nothing is sent. (The given passphrase used to
     * be trusted, so a transaction posted to another network than meant was
     * reported under a hash that network never files it under.)
     *
     * Then, with no memo, each account paid is checked for SEP-29's
     * memo_required flag, on the network just checked; one that sets it
     * stops the submit, reported as "memoRequired" with that `destination`.
     * A lookup that fails stops it too, reported as what it was
     * ("rateLimited" with `retryAfter`, "timeout", "badResponse",
     * "badRequest", "connection"), the message naming the account and SEP-29.
     *
     * A submit is POSTED from the moment its POST /transactions is handed to
     * Qt's network stack: as soon as the endpoint's network has been checked
     * (at once when it is remembered) and, when SEP-29 lookups are needed
     * (no memo, and an account paid), the last of them has passed. Until
     * then its envelope has not left this process; from then on Horizon may
     * receive it at any moment, and the transaction may be applied. Whether
     * it was posted decides what a cancel reports (cancelAll()), and whether
     * a failure says it may have reached Horizon (failed()).
     *
     * Each submit runs on a Server of its own (so a QNetworkAccessManager,
     * and a connection, of its own), which lives until the submit has been
     * reported: that is what tells a cancel whether this submit was posted,
     * what lets a failure tell whether any of its POST was written, and
     * whether Qt sent it again (see failed()), and what keeps an endpoint
     * change from deleting the manager under a submit on the wire. Queries
     * share one Server, as before.
     */
    Q_INVOKABLE int submit(const QString& envelopeXdrBase64,
                           const QString& networkPassphrase);

    /**
     * Cancel every request started so far. What is still reported depends
     * on the kind of request, and for a submit on whether it had been POSTED
     * (see submit()) when this was called:
     *
     * - loadAccount(), fundAccount(), runQuery(): nothing. They leave
     *   `pending` at once and no signal of theirs is delivered. None of them
     *   changes the ledger (friendbot may still fund the account).
     * - A submit not yet posted (still asking the endpoint's network, or in
     *   its SEP-29 lookups): nothing, and it never is posted: its lookups
     *   are dropped with it. It leaves `pending` at once. (It used to be
     *   posted all the same once its lookup was answered, as a payment
     *   nobody was waiting for.)
     * - A request cancelled while it was being started, from a handler of
     *   the pendingChanged that starting it emits: never started. Nothing is
     *   asked of Horizon (a submit is not posted), nothing is reported, and
     *   the id the call returns is never seen again. (A submit went on to
     *   post all the same, and its outcome was dropped.)
     * - A submit posted: ALWAYS reported, exactly once, and counted in
     *   `pending` until then. On the next pass of the event loop its POST is
     *   aborted and it fails with kind "cancelled", mayHaveReachedServer true
     *   and the transaction's `hash`, and `resent` when Qt had sent it
     *   again (see failed()): Horizon may have received it, so it may
     *   still be applied — look it up by that hash
     *   before submitting it again or reusing its sequence number. If its
     *   own end (Horizon's answer, or a failure) reaches this object before
     *   that, that end is reported instead, submitted() or failed(), as
     *   without the cancel. (Its report used to be dropped with the rest,
     *   and nothing aborted the POST, so a wallet that cancelled while a
     *   submit was on the wire never heard that it may land, and could pay
     *   again.)
     * - A request refused on the spot (kind "validation", off `pending`
     *   before its id was even returned) still reports its failure.
     *
     * An endpoint change cancels the same way (see `endpoint`).
     */
    Q_INVOKABLE void cancelAll();

    // ─── Exploring ───────────────────────────────────────────────────
    //
    // The same table-driven shape the transaction builder uses: the SDK says
    // which queries exist and what each one needs, and a form draws itself
    // from that rather than keeping a second list that can drift out of step.

    /** The queries runQuery() understands, in the order to offer them. */
    Q_INVOKABLE QStringList supportedQueries() const;

    /** One line saying what a query answers. */
    Q_INVOKABLE QString describeQuery(const QString& query) const;

    /** { name, kind, label, required, note } per input the query takes. */
    Q_INVOKABLE QVariantList fieldsForQuery(const QString& query) const;

    /**
     * Runs one query. Emits queried(id, { columns, rows, count }) or failed().
     *
     * Every row is a map of column name to an already-rendered string, and
     * `columns` fixes their order. Flattening here rather than in QML is
     * deliberate: the responses are a dozen unrelated C++ types, and the
     * alternative is a dozen unrelated QML views that each have to know one.
     */
    Q_INVOKABLE int runQuery(const QVariantMap& spec);

signals:
    void friendbotUrlChanged();
    void endpointChanged();
    void pendingChanged();

    /** {id, sequence, nextSequence, balances, thresholds, signers, flags, ...} */
    void accountLoaded(int requestId, const QVariantMap& account);

    /** {accountId, hash} once friendbot has funded the account. */
    void funded(int requestId, const QVariantMap& result);

    /**
     * {successful, hash, ledger, envelopeXdr, resultXdr, txResultCode,
     * opResultCodes}. txResultCode is Horizon's name for the result's code,
     * "tx_success" or "tx_fee_bump_inner_success"; opResultCodes is empty (a
     * 200 carries none).
     */
    void submitted(int requestId, const QVariantMap& result);

    /**
     * {kind, message, status, title, detail, ...} — see the class comment
     * for kinds; the other keys are there when they apply.
     *
     * Every failure of a submit() also carries:
     *
     * - `mayHaveReachedServer`: false only when its transaction is known not
     *   to have reached Horizon, because the failure came before it was
     *   posted (see submit()): refused here ("validation", a passphrase that
     *   is not the endpoint's included), stopped by SEP-29 ("memoRequired",
     *   or a lookup that failed), or the endpoint's network could not be
     *   read; or because its POST never left: the connection failed without
     *   ever opening (refused, host not found, TLS handshake failed, proxy
     *   refused or not found, unknown protocol) AND not a byte of the POST
     *   was ever written, which is watched from the moment Qt makes the
     *   POST. True for every other failure of a posted submit: the SDK's
     *   own timeout (Response::replyTimeout()) and Horizon's 504
     *   ("timeout"), a connection lost midway ("connection"), a 5xx or an
     *   answer that cannot be read, a 200 included ("badResponse"), a
     *   cancel ("cancelled"), and Horizon's own answers ("txFailed",
     *   "badRequest", "rateLimited"). A refused connection after one that
     *   carried the POST is true as well: Qt 5 sends a POST again, by
     *   itself, when the connection it was written on closes before an
     *   answer, and when Horizon read the first one and then went away, the
     *   second is refused; the transaction may still land. (Only
     *   "cancelled" said it, so a timed-out payment and one that never left
     *   looked the same; and a refused resend said false, as if the
     *   payment had never left, though Horizon had read it.)
     * - `hash`: exactly when `mayHaveReachedServer` is true, the
     *   transaction's hash, in lowercase hex, on the network the endpoint
     *   serves (the passphrase given was checked against it before posting).
     *   That is the hash Horizon files it under: look it up by it at the
     *   endpoint it was posted to (GET /transactions/{hash}, or runQuery's
     *   "transaction") before submitting it again or reusing its sequence
     *   number.
     * - `resent`: true when Qt sent the POST again, by itself, after it had
     *   written it whole (Qt 5 does, on a new connection, when the one the
     *   POST was written on closes before an answer), whatever the kind:
     *   what ended the submit then came of a later send, while Horizon may
     *   have read the first and applied it. mayHaveReachedServer is then
     *   true, with the hash, and the message starts "not final" and names
     *   the hash, "cancelled" included (a cancel's message was its own, and
     *   the others named no hash). Absent
     *   otherwise. Horizon's 200 to a later send is still submitted(): it
     *   names the ledger the transaction is in, and a transaction is applied
     *   once at most. (Horizon's answer to a later send used to be reported
     *   as its answer to the submit, final, with nothing to tell it apart.)
     *
     * mayHaveReachedServer says whether Horizon may have received the
     * transaction, not whether it may still be applied. Horizon's own
     * answers ("txFailed", "badRequest", "rateLimited") are final for this
     * submit ONLY WITHOUT `resent`: they then answer the one POST it made.
     * With "txFailed" Horizon did receive it, and the network ruled on it:
     * this submit will not make it succeed. It was either applied in a
     * ledger as a failed transaction (its fee charged, its sequence number
     * used) or refused before any ledger (nothing charged); the result codes
     * say why, and looking its hash up says which (found: in a ledger). With
     * "badRequest" or "rateLimited" Horizon refused the request without
     * passing it on. WITH `resent` true they are not final: they answer a
     * POST Qt sent again, and the one before it, which Horizon may have read
     * whole, may have been applied. A tx_bad_seq then most likely means it
     * was (Horizon answers that to a transaction sent again once the first
     * has been applied, before it has taken in that ledger). Look it up by
     * its hash before anything else: building the payment again with a
     * fresh sequence number may pay twice. After "timeout", "connection"
     * (mayHaveReachedServer true), "badResponse", "cancelled" or "unknown"
     * the outcome is not known either: it may still land. (A "txFailed"
     * tx_bad_seq for an envelope submitted again after such an unknown
     * outcome can mean the first one landed: look it up by hash.)
     *
     * No other request's failure carries these three keys.
     */
    void failed(int requestId, const QVariantMap& error);

    /** {query, columns, rows, count} — one table, ready to render. */
    void queried(int requestId, const QVariantMap& result);

private:
    Server* server();
    QNetworkAccessManager* directManager();
    int nextRequestId();
    /** Takes a request off `pending`; a submit's own Server goes too (later). */
    void finish(int requestId);

    /**
     * Wires ready/error on a Response so that exactly one of them is acted on,
     * and the Response is destroyed afterwards.
     *
     * On ready, `read` turns the reply into the result map and `outcome` (one
     * of the result signals) carries it. Reading happens apart from the
     * emit, inside a catch: `read` calls SDK getters that throw on data they
     * cannot parse, and it runs from Qt's signal dispatch, which an exception
     * must never cross. One that is thrown becomes a "badResponse" failure.
     *
     * The SDK parents responses to the Server specifically so QML cannot
     * collect them; that makes them this class's problem to free instead.
     */
    void wire(int requestId, Response* response, std::function<QVariantMap()> read,
              void (HorizonQml::*outcome)(int, const QVariantMap&));

    /** Turns a Response in the error state into a classified failure map. */
    QVariantMap responseError(Response* response) const;
    void failWith(int requestId, const QString& kind, const QString& message);
    /**
     * Emits failed() now, for a request already taken off `m_pending` (from
     * a reply handler, where the request id has long been returned).
     */
    void failNow(int requestId, const QString& kind, const QString& message);

    /**
     * Reports, as "cancelled", every submit a cancel stopped after it was
     * posted and that has not ended on its own since, aborting its POST.
     * Queued by cancelAll(), so nothing is emitted from inside it.
     */
    void settleCancelledSubmits();

    /**
     * Posts the submit `requestId` once the endpoint's network is known to
     * be the one it was given: at once when the one remembered is, and
     * otherwise once Horizon's root has said so (see submit()).
     */
    void checkNetwork(int requestId);
    /** Hands the submit to its Server: SEP-29 lookups, then the POST. */
    void post(int requestId);
    /** Wires the answer of a submit handed to its Server (post()). */
    void wireSubmit(int requestId, SubmitTransactionResponse* response);
    /**
     * Reports a failure of a submit still pending: `error`, with
     * mayHaveReachedServer and, when it is true, the transaction's hash;
     * and, when Qt sent its POST again, `resent` and a message saying so.
     * Queued when `queued` (from inside submit(), which has not returned the
     * id yet), emitted now otherwise.
     */
    void failSubmit(int requestId, QVariantMap error, bool mayHaveReachedServer,
                    bool queued);

    /** A submit, from submit() until it is reported or dropped unposted. */
    struct Submit
    {
        /** Its own Server (see submit()); deleted once it is reported. */
        Server* server = nullptr;
        /** Its transaction, a child of `server`, then of `response`. */
        AbstractTransaction* transaction = nullptr;
        /** The passphrase submit() was given. */
        QString passphrase;
        /** Its SubmitTransactionResponse, a child of `server`, once posting. */
        QPointer<QObject> response;
        /** Its transaction's hash on the network it was submitted for. */
        QString hash;
        /**
         * The connection has said it wrote some of its POST's body
         * (uploadProgress, watched on every reply of its Server's manager
         * from the moment the reply is made, the POST the Server makes
         * itself after the SEP-29 lookups included): the POST left, on a
         * connection that opened, whatever error the request ends with.
         */
        bool written = false;
        /**
         * The connection wrote some of its POST's body again after it had
         * written it whole once: Qt sent it again, by itself (Qt 5 does
         * when the connection closes with no answer), so what ends the
         * submit came of a later send, and Horizon may have applied an
         * earlier one (`resent`, see failed()).
         */
        bool resent = false;
        /** A cancel stopped it after it was posted: it is still to be reported. */
        bool cancelled = false;
    };

    AssetRegistry* m_assets = nullptr;
    QString m_endpoint;
    /**
     * The network passphrase `m_endpoint` serves, as its root last said;
     * empty until a submit has asked, and again after an endpoint change.
     */
    QString m_networkPassphrase;
    /** The Server queries share; submits have their own (m_submits). */
    Server* m_server = nullptr;
    QNetworkAccessManager* m_directManager = nullptr;
    int m_lastRequestId = 0;
    QHash<int, bool> m_pending;
    /** Submits started and not yet reported, by request id. */
    QHash<int, Submit> m_submits;
    QString m_friendbotUrl;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(HorizonQml)

#endif // QSTELLAR_HORIZONQML_H
