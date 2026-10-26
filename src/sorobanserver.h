#ifndef SOROBANSERVER_H
#define SOROBANSERVER_H

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>

#include "responses/sorobanrpc/gethealthresponse.h"
#include "responses/sorobanrpc/getnetworkresponse.h"
#include "responses/sorobanrpc/getversioninforesponse.h"
#include "responses/sorobanrpc/getlatestledgerresponse.h"
#include "responses/sorobanrpc/getfeestatsresponse.h"
#include "responses/sorobanrpc/gettransactionresponse.h"
#include "responses/sorobanrpc/gettransactionsresponse.h"
#include "responses/sorobanrpc/getledgerentriesresponse.h"
#include "responses/sorobanrpc/getledgersresponse.h"
#include "responses/sorobanrpc/geteventsresponse.h"
#include "responses/sorobanrpc/sendtransactionresponse.h"
#include "responses/sorobanrpc/simulatetransactionresponse.h"

#include "network.h"
#include "exception/sdkexception.h"

#include "qstellar_namespace.h"
QSTELLAR_FWD(AbstractTransaction)
QSTELLAR_FWD(Transaction)
class QEventLoop;
class QNetworkAccessManager;
class QNetworkReply;
namespace stellar { struct Asset; }

namespace qstellar { namespace exception {

/**
 * A Soroban RPC request that ended without an answer: the connection failed,
 * no answer arrived within the request timeout, cancelRequests() aborted it,
 * or cancelRequests() ended a pollTransaction() between two polls; or a
 * sendTransaction Qt sent again whose answer does not settle it (resent(),
 * below). Thrown by
 * SorobanServer::sendRequest, so by every call made through it,
 * sendTransaction included. It is a std::runtime_error, as these failures
 * were before. The timeout and cancel messages are unchanged, except that
 * after a resend (resent()) they end with "; Qt had sent it again ..."; a
 * network error's now also names the method and says whether the request
 * was sent.
 *
 * mayHaveReachedServer() is false only when the request is known never to
 * have left: the connection never opened (refused, host not found, proxy not
 * reachable, TLS handshake failed, unknown protocol) and not a byte of the
 * body was handed to it. For anything else it is true — a timeout or a cancel
 * whatever point the request had reached (whether its bytes had left cannot be
 * known at the moment it is aborted), a connection lost midway, an HTTP error
 * status. A sendTransaction that may have reached the server may still be
 * applied: look the transaction up by its hash (getTransaction) before sending
 * it again or giving its sequence number back.
 *
 * resent() is true when Qt sent the request again, by itself, after it had
 * written it whole once: Qt 5 does, on a new connection, when the connection
 * the request was written on closes before an answer (Qt 6 does not). What
 * ended the request then came of a later send, while the server may have read
 * the first whole and acted on it; mayHaveReachedServer() is true. For a
 * sendTransaction this is also what an ANSWER to the later send throws,
 * unless it is PENDING or DUPLICATE (see SorobanServer::sendRequest): an
 * ERROR (a txBAD_SEQ most likely means the first was applied), a
 * TRY_AGAIN_LATER, an error or an answer that does not parse answers the
 * later send only, and the transaction may have been applied. (Such an answer
 * used to be returned as the answer to the one send made: an ERROR or a
 * TRY_AGAIN_LATER read as "never taken in", and a sequence number was handed
 * back, or a payment built again, while the first send could land.)
 */
class SorobanTransportException : public SdkException
{
    bool m_mayHaveReachedServer;
    bool m_resent;
public:
    /** A request `resent` was written whole before: it may have reached the
     *  server, whatever `mayHaveReachedServer` says. */
    SorobanTransportException(const QString& message, bool mayHaveReachedServer,
                              bool resent = false)
        : SdkException(message), m_mayHaveReachedServer(mayHaveReachedServer || resent),
          m_resent(resent) {}

    bool mayHaveReachedServer() const { return m_mayHaveReachedServer; }
    /** Qt sent the request again, by itself, after it had written it whole. */
    bool resent() const { return m_resent; }
};

} } // namespaces

QSTELLAR_BEGIN_NS

/**
 * Stellar Asset Contract (SAC) balance entry for one contract holder of one
 * asset. `amount` is the canonical 16-byte big-endian two's-complement i128;
 * use `Scv::fromInt128` semantics to interpret. A balance is never negative.
 *
 * `exists == false` means the ContractData entry was not present (the
 * holder has never received this SAC). In that case `amount` is 16 zero
 * bytes and the other fields are at their defaults.
 */
struct SACBalance
{
    QByteArray amount;       // 16 bytes big-endian i128
    bool authorized = true;
    bool clawbackEnabled = false;
    bool exists = false;
};

/**
 * An account's or a contract's balance of one asset, as
 * SorobanServer::getAssetBalance reads it over RPC.
 *
 * `amount` is the balance in stroops as a 16-byte big-endian i128, as in
 * SACBalance (an account's balance is an int64, a contract's an i128; either
 * is exact here). The flags are the trustline's, as js-stellar-sdk's
 * getAssetBalance reports them: `authorized` (AUTHORIZED_FLAG),
 * `authorizedToMaintainLiabilities` (AUTHORIZED_TO_MAINTAIN_LIABILITIES_FLAG
 * alone: false on a line that is fully authorized) and `clawbackEnabled`
 * (TRUSTLINE_CLAWBACK_ENABLED_FLAG). An account's XLM is authorized and
 * cannot be clawed back; a contract's balance is its asset contract's
 * {authorized, clawback}. `buyingLiabilities` / `sellingLiabilities` are
 * what the account's offers hold of it, in stroops (0 for a contract, and for
 * an entry with no liabilities extension); the balance less the selling
 * liabilities is what it can still send (less its reserve, for XLM).
 * `latestLedger` is the ledger the RPC answered at.
 *
 * `exists == false` means there is no such entry: the account does not
 * exist or has no trustline to the asset (an issuer holds none to its own),
 * or the contract never received it. `amount` is then 16 zero bytes and
 * every flag false.
 */
struct AssetBalance
{
    QByteArray amount;                          // 16 bytes big-endian i128
    bool authorized = false;
    bool authorizedToMaintainLiabilities = false;
    bool clawbackEnabled = false;
    qint64 buyingLiabilities = 0;
    qint64 sellingLiabilities = 0;
    bool exists = false;
    quint32 latestLedger = 0;
};

/**
 * What SorobanServer::simulateTransaction sends besides the transaction, as
 * js-stellar-sdk 17.1.0's simulateTransaction(tx, addlResources, authMode,
 * useUpgradedAuth) and py-stellar-sdk 14.1.0's simulate_transaction(tx,
 * addl_resources, auth_mode) send it. The defaults send the transaction
 * alone, as every simulation here did.
 *
 * `instructionLeeway`: CPU instructions the RPC adds to those the simulation
 * used (resourceConfig.instructionLeeway; js addlResources.cpuInstructions,
 * py ResourceLeeway), for a call that may cost more on chain than it did
 * simulated. 0 sends none.
 *
 * `authMode`: Default sends none, and the RPC enforces the auth entries the
 * transaction carries, or records them when it carries none. Enforce, Record
 * and RecordAllowNonroot send "enforce", "record" and "record_allow_nonroot"
 * (which records non-root authorization too).
 *
 * `useUpgradedAuth`: Requested asks a recording simulation for CAP-71
 * ADDRESS_V2 credentials, NotRequested for ADDRESS ones. NotSent, the
 * default, leaves it out, as py does, and the RPC records ADDRESS ones until
 * protocol 28 makes V2 its default. js sends true unless told otherwise; that
 * is not followed here: py 14.1.0 signs ADDRESS credentials only, and
 * Sep45Challenge takes ADDRESS ones only, so V2 entries recorded by default
 * would be unsignable by a py counterpart and break the SEP-45 challenges
 * built here. SorobanCredentialsSigner and AssembledTransaction sign either.
 */
struct SimulateTransactionOptions
{
    enum class AuthMode { Default, Enforce, Record, RecordAllowNonroot };
    enum class UpgradedAuth { NotSent, Requested, NotRequested };

    quint32 instructionLeeway = 0;
    AuthMode authMode = AuthMode::Default;
    UpgradedAuth useUpgradedAuth = UpgradedAuth::NotSent;
};

/**
 * Soroban RPC client (CAP-46).
 *
 * Mirrors the Java SDK `SorobanServer`. Methods are synchronous: each call
 * issues a JSON-RPC 2.0 POST to the configured endpoint, blocks on the
 * reply via a local event loop, and returns the parsed result by value
 * (throwing `qstellar::exception::SorobanRpcException` on RPC errors).
 *
 * The wait is bounded: a request with no complete answer within
 * `getRequestTimeout()` is aborted and throws, and `cancelRequests()` aborts
 * the ones waiting at once. (It used to wait forever: an endpoint that took
 * the connection and never answered blocked the calling thread for good.)
 * Both throw `qstellar::exception::SorobanTransportException`, which says
 * whether the request may have reached the server.
 *
 * The JSON-RPC plumbing is split between two layers:
 *   - HTTP layer: `sendRequest(method, params)` posts to the RPC URL
 *     and returns the raw response bytes.
 *   - Parse layer: `parseRpcResult(bytes)` validates the envelope, throws
 *     on error, and returns the `result` payload as a QJsonValue. Tests
 *     can exercise the parse layer directly with fixture JSON.
 */
class SorobanServer : public QObject
{
    Q_OBJECT
    QUrl m_endpoint;
    QNetworkAccessManager* m_http;
public:
    explicit SorobanServer(const QUrl& endpoint, QObject* parent = nullptr);
    ~SorobanServer() override;

    QUrl getEndpoint() const { return m_endpoint; }

    /**
     * How long one request may take, in milliseconds, from being posted to
     * its answer arriving in full: STELLAR_QT_REPLY_TIMEOUT (30000) unless
     * set. A request past it is aborted and throws SorobanTransportException
     * "Soroban RPC HTTP error: no answer within N ms", with
     * mayHaveReachedServer() true: a sendTransaction that timed out may still
     * have reached the server, so look the transaction up by its hash before
     * sending it again. `milliseconds` must be positive (std::invalid_argument).
     */
    int getRequestTimeout() const { return m_requestTimeout; }
    void setRequestTimeout(int milliseconds);

    /**
     * Sends a JSON-RPC 2.0 request and returns the `result` JSON value.
     * Throws `qstellar::exception::SorobanRpcException` if the envelope
     * carries an `error` instead, and `qstellar::exception::
     * SorobanTransportException` (a std::runtime_error) when the request
     * fails in transport, gets no answer within the request timeout or is
     * cancelled; its mayHaveReachedServer() is false only for a connection
     * that never opened. (Every one of these was a plain std::runtime_error,
     * so a caller could not tell a request that never left from one the
     * server may have acted on.)
     *
     * Qt 5 sends a request again, by itself, on a new connection, when the
     * one it was written on closes before an answer; the body is written
     * from the start. That is watched here (what the connection reports
     * written: the whole body once, then some of it again), and said by
     * SorobanTransportException::resent() on whatever such a request throws.
     * An answer to it is the answer to the later send. That is taken as it
     * comes for every method but `sendTransaction`, whose answer to a later
     * send is taken only when it says the RPC has the transaction (PENDING or
     * DUPLICATE). Anything else — ERROR, TRY_AGAIN_LATER, an error envelope,
     * an answer that does not parse — throws SorobanTransportException with
     * resent() and mayHaveReachedServer() true: the RPC may have taken in,
     * and applied, the first send, whatever it said to the later one (an
     * ERROR with txBAD_SEQ most likely means the first was applied). Look the
     * transaction up by its hash. (It was returned as the answer to the one
     * send made, and an ERROR or TRY_AGAIN_LATER read as "never taken in".)
     *
     * `virtual` so tests can subclass `SorobanServer` and capture the
     * (method, params) a high-level call assembles — or feed a scripted
     * result — without a live RPC (mirrors `sendTransactionXdr` /
     * `simulateTransactionXdr`).
     */
    virtual QJsonValue sendRequest(const QString& method, const QJsonValue& params = QJsonValue());

    /**
     * Parses a JSON-RPC 2.0 response envelope (`{jsonrpc, id, result}` or
     * `{jsonrpc, id, error}`). Returns the `result` value on success;
     * throws `SorobanRpcException` on an error envelope.
     */
    static QJsonValue parseRpcResult(const QByteArray& jsonBytes);

    /**
     * Builds a JSON-RPC 2.0 request envelope for the given method/params,
     * with `id = "<incrementing>"`. Stable id generation is a per-instance
     * counter so tests can predict the wire bytes.
     */
    QByteArray buildRpcRequest(const QString& method, const QJsonValue& params);

    // ─── Soroban RPC methods (more land in Iter 2–4) ─────────────────

    /** `getHealth` — status, ledger window. */
    GetHealthResponse getHealth();
    /** `getNetwork` — passphrase, protocolVersion, friendbotUrl. */
    GetNetworkResponse getNetwork();
    /** `getVersionInfo` — RPC + captive-core build info. */
    GetVersionInfoResponse getVersionInfo();
    /** `getLatestLedger` — id / protocolVersion / sequence / closeTime,
     *  and the ledger's header and close meta as base64 XDR. */
    GetLatestLedgerResponse getLatestLedger();
    /** `getFeeStats` — inclusionFee + sorobanInclusionFee distributions. */
    GetFeeStatsResponse getFeeStats();

    /** `getTransaction(hash)` — single transaction lookup. */
    GetTransactionResponse getTransaction(const QString& hash);

    /** `getLedgerEntries(keys)` — keys are base64 XDR LedgerKey strings. */
    GetLedgerEntriesResponse getLedgerEntries(const QStringList& base64Keys);

    /**
     * `getLedgers` and `getTransactions` paginated lookups: give a
     * startLedger within the RPC's retention window (the RPC refuses 0,
     * "start ledger (0) must be between the oldest ledger ... and the latest
     * ledger"), or a cursor from a previous page with startLedger 0. A
     * startLedger above 0 is sent with the cursor, as js and py send both, and
     * the RPC refuses the two together ("startLedger (...) and cursor (...)
     * cannot both be set"). This said the cursor took precedence; nothing
     * here gives it any.
     */
    GetLedgersResponse getLedgers(quint32 startLedger, const QString& cursor = QString(),
                                  qint32 limit = 0);
    GetTransactionsResponse getTransactions(quint32 startLedger, const QString& cursor = QString(),
                                            qint32 limit = 0);

    /**
     * `getEvents` — accepts a `startLedger` plus optional `endLedger`,
     * `filters` (already-built as a QJsonArray to keep the surface tight
     * for 0.11.0), `cursor`, and `limit`.
     */
    GetEventsResponse getEvents(quint32 startLedger,
                                quint32 endLedger = 0,
                                const class QJsonArray& filters = QJsonArray(),
                                const QString& cursor = QString(),
                                qint32 limit = 0);

    /**
     * `sendTransaction` — submits an already-signed transaction to the
     * Soroban-aware node. Returns the immediate ack (PENDING / DUPLICATE /
     * TRY_AGAIN_LATER / ERROR). Use `getTransaction(hash)` to poll for
     * the final result.
     *
     * TRY_AGAIN_LATER and ERROR are returned only as the answer to the one
     * send made. When Qt sent the request again, by itself, after it had
     * written it whole (Qt 5 does when that connection closes with no
     * answer), only PENDING or DUPLICATE is returned; any other answer throws
     * SorobanTransportException with resent() and mayHaveReachedServer()
     * true (see sendRequest): the first send may have been taken in and
     * applied.
     *
     * A send that throws SorobanTransportException with
     * mayHaveReachedServer() false never left: nothing was submitted. Any
     * other exception — that one with true (a timeout, a cancel, a connection
     * lost midway), a SorobanRpcException, an answer that does not parse —
     * may have come after the server took the transaction in, and it may
     * still be applied: look it up by its hash before sending it again or
     * reusing its sequence number.
     */
    SendTransactionResponse sendTransaction(AbstractTransaction* tx);
    /** Overload that takes the base64 envelope directly.
     *  `virtual` so tests can subclass `SorobanServer` and feed scripted
     *  responses without an actual RPC endpoint. */
    virtual SendTransactionResponse sendTransactionXdr(const QString& envelopeXdrBase64);

    /**
     * `simulateTransaction` — runs the transaction in the host without
     * committing. Returns transactionData (a base64
     * SorobanTransactionData), minResourceFee, host-function results,
     * generated auth entries, etc. Used as a building block for
     * `prepareTransaction()` (Iter 4).
     */
    SimulateTransactionResponse simulateTransaction(AbstractTransaction* tx);
    /** Overload that takes the base64 envelope directly.
     *  `virtual` so tests can subclass `SorobanServer` and feed scripted
     *  responses without an actual RPC endpoint. */
    virtual SimulateTransactionResponse simulateTransactionXdr(const QString& envelopeXdrBase64);

    /**
     * simulateTransaction sending what `options` asks for besides the
     * transaction (SimulateTransactionOptions); the overloads above send the
     * default options. Not virtual: a subclass overriding the one above is
     * not called by these, which post through sendRequest (virtual) as it
     * does.
     */
    SimulateTransactionResponse simulateTransaction(AbstractTransaction* tx,
                                                    const SimulateTransactionOptions& options);
    SimulateTransactionResponse simulateTransactionXdr(const QString& envelopeXdrBase64,
                                                       const SimulateTransactionOptions& options);

    /**
     * `prepareTransaction` — runs the transaction through `simulateTransaction`
     * and then mutates it in place so it is ready to sign + send:
     *   - decodes the simulated SorobanTransactionData into the transaction's
     *     `ext.sorobanData`
     *   - if the first InvokeHostFunctionOperation has no auth, fills in
     *     the auth entries returned by the simulation
     *   - sets the fee to the inclusion fee the caller bid plus the
     *     `resourceFee` of the simulated SorobanTransactionData — the one that
     *     goes into the envelope — as js-stellar-sdk's assembleTransaction
     *     does. The bid is the fee less the resource fee of the footprint the
     *     transaction already carries (none: the whole fee), so re-simulating
     *     recovers it exactly and replaces the resource fee rather than adding
     *     another. `minResourceFee` must be a non-negative whole number when
     *     present, but is not charged: in 1.0.3 it was, and an RPC whose
     *     minResourceFee differed from transactionData.resourceFee moved the
     *     bid on every re-simulation. With no transactionData the fee and the
     *     footprint are left as they are
     * Throws on simulation error, on a simulation it cannot decode in full
     * (strict base64 included), and on a transaction the bid cannot be
     * recovered from: a fee below its own footprint's resource fee, a
     * footprint whose resource fee is negative (fee 100 with -4000000000 was
     * signed as 4000000400 stroops), or a fee outside 0..4294967295; either
     * way the transaction is left untouched.
     */
    void prepareTransaction(Transaction* tx);
    /** prepareTransaction, simulated with `options` (SimulateTransactionOptions). */
    void prepareTransaction(Transaction* tx, const SimulateTransactionOptions& options);

    /**
     * Pure helper used by `prepareTransaction` once it has the simulation
     * result. Mutates `tx` in place. Static + reachable so tests can drive
     * the assembly logic with a hand-crafted `SimulateTransactionResponse`
     * (no RPC endpoint needed).
     */
    static void applyTransactionSimulation(Transaction* tx,
                                           const SimulateTransactionResponse& sim);

    /**
     * Block until `getTransaction(hash)` returns SUCCESS or FAILED, or
     * `timeoutMs` elapses. Polls every `intervalMs`. Throws on timeout.
     *
     * cancelRequests() ends it, during a poll or between two: it throws
     * SorobanTransportException "pollTransaction: cancelled ...". The wait
     * between polls used to ignore it, so a cancel (or SorobanQml's endpoint
     * change) left the caller waiting out the rest of `timeoutMs`. Either way
     * the transaction may still be applied: look it up by its hash.
     *
     * A SorobanTransportException from a poll is about that getTransaction
     * request: mayHaveReachedServer() false says the poll never left, not the
     * transaction, which was sent before polling began and may still be
     * applied.
     */
    GetTransactionResponse pollTransaction(const QString& hash,
                                           int timeoutMs = 30000,
                                           int intervalMs = 1000);

    /**
     * Hook used by `pollTransaction` to fetch a transaction. Default
     * implementation calls `getTransaction(hash)`. Made virtual so tests
     * can subclass SorobanServer and feed scripted responses without an
     * actual RPC endpoint.
     */
    virtual GetTransactionResponse fetchTransaction(const QString& hash) {
        return getTransaction(hash);
    }

    /**
     * SAC balance lookup. Builds the canonical Soroban SAC balance
     * LedgerKey for the contract `accountId` (a C-strkey) holding `asset`
     * on `network`, fetches the entry via `getLedgerEntries`, and parses
     * the i128 amount + authorized/clawback flags.
     *
     * Returns `SACBalance{exists:false}` if the contract has never
     * received this SAC (no ledger entry exists for that key).
     *
     * An account (G-strkey) is refused: the asset contract keeps an
     * account's balance in its trustline (its XLM in the account entry),
     * never in a contract data entry, so there is none to read. It used to
     * be looked up all the same and read as a balance of zero.
     * js-stellar-sdk's getSACBalance refuses one too.
     *
     * What the RPC answers is decoded as stellar-xdr lays it out (see
     * parseSACBalanceFromLedgerEntryXdr), and must be the entry for this
     * key: anything else throws. In 1.0.3 every real balance threw here.
     */
    SACBalance getSACBalance(const QString& accountId,
                             const stellar::Asset& asset,
                             Network* network);

    /**
     * `address`'s balance of `asset`, read over RPC with one getLedgerEntries
     * — js-stellar-sdk 17.1.0's getAssetBalance, which getSACBalance is not:
     * that one refuses an account, so an account's balance had no way to be
     * read here. For an account (G...) it is the trustline of an issued asset
     * (its balance, flags and liabilities), and for XLM the account entry's
     * balance and liabilities (js looks for an XLM trustline, which no
     * account has, and throws); for a contract (C...) it is getSACBalance's,
     * whose asset contract id needs `network` (for an account `network` is
     * not used and may be null). No entry is `exists == false`, not an error.
     *
     * Refused, with std::runtime_error: a muxed account (M...), whose balance
     * is its account's (ask for the G... address), any other kind of address,
     * a pool share or an asset code no ledger can hold (Core's rule: 1 to 4,
     * or 5 to 12, letters and digits, then zero bytes). The answer must be the
     * one entry asked for, under the key asked for, decoded as stellar-xdr
     * lays it out — a LedgerEntryData holding an AccountEntry (its ext chain
     * v1 liabilities, v2 sponsorships, v3 sequence ledger) or a TrustLineEntry
     * (ext v1 liabilities, v2 pool use count) — strict base64, every byte
     * read; a negative balance or liability, a trustline flag Core does not
     * define or both authorization flags at once are refused as well.
     */
    AssetBalance getAssetBalance(const QString& address,
                                 const stellar::Asset& asset,
                                 Network* network);

    /**
     * Decode the entry `getLedgerEntries` returns for an SAC balance key —
     * its `xdr`, a base64 LedgerEntryData: the entry type (CONTRACT_DATA),
     * then the ContractDataEntry {ext, contract, key, durability, val} — and
     * extract the SAC balance fields. Strict: base64 with nothing that is not
     * base64, every byte read, a PERSISTENT entry keyed
     * ["Balance", address], and a value that is exactly the map
     * {amount: i128 >= 0, authorized: bool, clawback: bool}; anything else
     * throws std::runtime_error. (It read a layout of its own — a LedgerEntry
     * prefix and no ext — so every real entry was refused as not contract
     * data, and a missing or mistyped flag was taken as a default.) Exposed
     * as a static helper so tests can drive it directly without an RPC
     * endpoint.
     */
    static SACBalance parseSACBalanceFromLedgerEntryXdr(const QString& base64LedgerEntry);

    /** Build the canonical base64-encoded LedgerKey for an SAC balance lookup.
     *  Only a contract holder has such an entry (see getSACBalance). */
    static QString buildSACBalanceLedgerKey(const QString& accountId,
                                            const stellar::Asset& asset,
                                            const QString& networkPassphrase);

public slots:
    /**
     * Aborts every request of this server still waiting for its answer; each
     * throws SorobanTransportException "Soroban RPC request cancelled ...",
     * with mayHaveReachedServer() true: a cancelled sendTransaction may still
     * have reached the server. A pollTransaction under way ends too, between
     * two polls as well as during one. Requests (and polls) made afterwards
     * are sent as usual.
     *
     * Call it on the server's own thread. From another one, queue it —
     * QMetaObject::invokeMethod(server, "cancelRequests", Qt::QueuedConnection)
     * — and the event loop the request waits in delivers it.
     */
    void cancelRequests();

private:
    quint64 m_nextId = 1;
    int m_requestTimeout = 0;               // set by the constructor
    QList<QNetworkReply*> m_inFlight;       // posted, answer not yet read
    QList<QEventLoop*> m_pollWaits;         // pollTransaction waits between polls
    quint64 m_cancels = 0;                  // cancelRequests() calls so far
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(SACBalance)
QSTELLAR_ALIAS(AssetBalance)
QSTELLAR_ALIAS(SimulateTransactionOptions)
QSTELLAR_ALIAS(SorobanServer)

#endif // SOROBANSERVER_H
