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

class AbstractTransaction;
class Network;
class QNetworkAccessManager;
namespace stellar { struct Asset; }

/**
 * Stellar Asset Contract (SAC) balance entry for one holder of one asset.
 * `amount` is the canonical 16-byte big-endian two's-complement i128;
 * use `Scv::fromInt128` semantics to interpret.
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
 * Soroban RPC client (CAP-46).
 *
 * Mirrors the Java SDK `SorobanServer`. Methods are synchronous: each call
 * issues a JSON-RPC 2.0 POST to the configured endpoint, blocks on the
 * reply via a local event loop, and returns the parsed result by value
 * (throwing `qstellar::exception::SorobanRpcException` on RPC errors).
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
     * Sends a JSON-RPC 2.0 request and returns the `result` JSON value.
     * Throws `qstellar::exception::SorobanRpcException` if the envelope
     * carries an `error` instead.
     */
    QJsonValue sendRequest(const QString& method, const QJsonValue& params = QJsonValue());

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
    /** `getLatestLedger` — id / protocolVersion / sequence. */
    GetLatestLedgerResponse getLatestLedger();
    /** `getFeeStats` — inclusionFee + sorobanInclusionFee distributions. */
    GetFeeStatsResponse getFeeStats();

    /** `getTransaction(hash)` — single transaction lookup. */
    GetTransactionResponse getTransaction(const QString& hash);

    /** `getLedgerEntries(keys)` — keys are base64 XDR LedgerKey strings. */
    GetLedgerEntriesResponse getLedgerEntries(const QStringList& base64Keys);

    /**
     * `getLedgers` and `getTransactions` paginated lookups. `startLedger=0`
     * means "from the latest"; non-zero cursor takes precedence.
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
     * `prepareTransaction` — runs the transaction through `simulateTransaction`
     * and then mutates it in place so it is ready to sign + send:
     *   - decodes the simulated SorobanTransactionData into the transaction's
     *     `ext.sorobanData`
     *   - if the first InvokeHostFunctionOperation has no auth, fills in
     *     the auth entries returned by the simulation
     *   - bumps the fee by `minResourceFee`
     * Throws on simulation error.
     */
    void prepareTransaction(class Transaction* tx);

    /**
     * Pure helper used by `prepareTransaction` once it has the simulation
     * result. Mutates `tx` in place. Static + reachable so tests can drive
     * the assembly logic with a hand-crafted `SimulateTransactionResponse`
     * (no RPC endpoint needed).
     */
    static void applyTransactionSimulation(class Transaction* tx,
                                           const SimulateTransactionResponse& sim);

    /**
     * Block until `getTransaction(hash)` returns SUCCESS or FAILED, or
     * `timeoutMs` elapses. Polls every `intervalMs`. Throws on timeout.
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
     * LedgerKey for `accountId` (G-strkey or C-strkey) holding `asset`
     * on `network`, fetches the entry via `getLedgerEntries`, and parses
     * the i128 amount + authorized/clawback flags.
     *
     * Returns `SACBalance{exists:false}` if the account has never
     * received this SAC (no ledger entry exists for that key).
     */
    SACBalance getSACBalance(const QString& accountId,
                             const stellar::Asset& asset,
                             Network* network);

    /**
     * Decode the LedgerEntry XDR returned by `getLedgerEntries` and
     * extract the SAC balance fields. Exposed as a static helper so tests
     * can drive it directly without an RPC endpoint.
     */
    static SACBalance parseSACBalanceFromLedgerEntryXdr(const QString& base64LedgerEntry);

    /** Build the canonical base64-encoded LedgerKey for an SAC balance lookup. */
    static QString buildSACBalanceLedgerKey(const QString& accountId,
                                            const stellar::Asset& asset,
                                            const QString& networkPassphrase);

private:
    quint64 m_nextId = 1;
};

#endif // SOROBANSERVER_H
