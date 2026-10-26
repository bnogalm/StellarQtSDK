#ifndef QSTELLAR_SOROBANQML_H
#define QSTELLAR_SOROBANQML_H

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QUrl>
#include <QVariantMap>

#include "qstellar_namespace.h"

class QThread;

QSTELLAR_BEGIN_NS

class SorobanWorker;

/**
 * QML facade over the Soroban RPC server.
 *
 * Same three rules as HorizonQml — nothing throws into QML, no SDK pointer
 * crosses the boundary, every outcome is a signal carrying a plain map — and
 * one more that this class exists to enforce:
 *
 * 4. THE RPC CLIENT NEVER RUNS ON THE UI THREAD. SorobanServer is synchronous:
 *    every call blocks on a nested QEventLoop until the answer or its timeout
 *    (STELLAR_QT_REPLY_TIMEOUT, 30 s), so calling it from a QML handler would
 *    freeze the window for as long. All of it runs on one pinned worker
 *    thread, one request at a time, in the order the requests were started.
 *    A request on the wire is abandoned by cancelAll() or an endpoint change,
 *    so a stalled endpoint holds up nothing behind it.
 *
 * The thread is PINNED rather than pooled on purpose. SorobanServer builds its
 * own QNetworkAccessManager, which has thread affinity, so it has to live where
 * it is called from; a pooled thread would mean a fresh server and a fresh TLS
 * handshake per call, and a Soroban flow is many round trips — simulate, send,
 * then poll until the ledger closes.
 *
 * Registered as the `Soroban` singleton by QStellarQml::registerTypes().
 */
class SorobanQml : public QObject
{
    Q_OBJECT

    /**
     * RPC endpoint. Changing it abandons every request started against the
     * old one: the one on the wire is aborted and the ones waiting are never
     * sent. Unlike cancelAll(), it reports every one of them, whatever its
     * kind: failed() with kind "cancelled" (mayHaveReachedServer false for
     * one that never left, true, with a send's hash, for one that may have;
     * see failed()), or, for one that ended on its own before the cancel
     * could stop it, that end, as it would have been reported anyway.
     * (Requests cancelAll() cancelled before are reported as cancelAll()
     * says.)
     * Requests started afterwards go to the new endpoint, one at a time as
     * always. (A request on the wire used to run to completion first, so an
     * old endpoint that never answered held up every request to the new one
     * forever.) Which network the old one serves is forgotten too: the next
     * send asks the new one (send()).
     */
    Q_PROPERTY(QString endpoint READ endpoint WRITE setEndpoint NOTIFY endpointChanged)

    /**
     * Requests started whose outcome may still be reported. Bind a busy
     * indicator to this. A send cancelAll() cancelled is counted until the
     * worker has settled it — reported, or dropped as never started — which
     * it does at once; see cancelAll().
     */
    Q_PROPERTY(int pending READ pending NOTIFY pendingChanged)

public:
    explicit SorobanQml(QObject* parent = nullptr);
    ~SorobanQml() override;

    QString endpoint() const { return m_endpoint; }
    void setEndpoint(const QString& endpoint);
    int pending() const { return m_pending.size(); }

    /** Canonical RPC URL for "testnet" / "public". Empty if unknown. */
    Q_INVOKABLE QString rpcUrlFor(const QString& network) const;

    /**
     * Runs a transaction against the current ledger WITHOUT submitting it, and
     * reports what it would have done.
     *
     * This is the most useful thing in Soroban and the least obvious: it is not
     * a dry run for reassurance, it is a required step. The network will not
     * accept an invocation that does not declare, up front, every ledger entry
     * it will read and write, and simulation is how that footprint is
     * discovered. It also returns the function's actual return value, the
     * resource fee, and the diagnostic events — which are the only way to see
     * why an invocation failed.
     *
     * Emits simulated(id, {...}) or failed(id, {...}).
     */
    Q_INVOKABLE int simulate(const QString& envelopeXdrBase64);

    /**
     * Simulates an unsigned Soroban envelope and hands back one the network
     * will accept: footprint, resource fee and auth entries all filled in.
     *
     * This is the step that has no offline form and no Horizon equivalent. A
     * Soroban transaction carries a declaration of every ledger entry it will
     * read and write and what it will cost, and the only thing that can work
     * those out is the host actually running the call — which is what the RPC
     * server does when it simulates.
     *
     * PREPARE BEFORE SIGNING, never after. Preparing rewrites the transaction,
     * so it rewrites the hash, and a signature made first covers a transaction
     * that no longer exists. The network answers that with tx_bad_auth and
     * says nothing about why.
     *
     * The passphrase is needed because an envelope does not carry its network
     * and the transaction has to be rebuilt to be modified.
     *
     * Returns the request id; the answer arrives on prepared() as
     * { xdr (= preparedXdr), hash, fee, feeBefore, resourceFee,
     *   minResourceFee, latestLedger, authEntries, signaturesCarried,
     *   hashChanged, returnValueXdr, returnValue, contractId } (the last three
     *   when the simulation returned them) or on failed(). The footprint is
     *   in simulate()'s answer.
     */
    Q_INVOKABLE int prepare(const QString& envelopeXdrBase64,
                            const QString& networkPassphrase);

    /**
     * Submits an already-signed, already-prepared envelope.
     *
     * A send whose outcome is unknown has to say which transaction to look
     * up, and a transaction's hash depends on the network passphrase, which
     * an envelope does not carry. So before anything is posted the endpoint
     * is asked which network it serves (getNetwork, once per endpoint and
     * again after every endpoint change, or after an answer that contradicted
     * it; see also the two-argument send), and the hash is worked out on that
     * network: the one the transaction is applied under there. A send that
     * fails after it may have reached the server says it in failed(), as
     * `hash`, exactly as send(envelope, passphrase) does. (It said only
     * mayHaveReachedServer, with nothing to look up.)
     *
     * Nothing is sent when that cannot be worked out: an envelope that does
     * not decode exactly (as AbstractTransaction::fromEnvelopeXdr reads it)
     * fails as "validation" before any request is made, the network lookup
     * included, whether the endpoint can be reached or not; and an endpoint
     * whose network cannot be read fails with that lookup's own kind
     * ("connection", "rpc", "cancelled"), mayHaveReachedServer false either
     * way. (Such an envelope used to be sent as it was.)
     *
     * What is posted is the envelope as the SDK writes it back: the same
     * bytes, without the whitespace that decoding skips (one pasted wrapped
     * over lines), which the RPC refuses inside an envelope. (It was posted
     * as given, refused there as invalid_xdr, and reported as a send that
     * may have reached the server.)
     *
     * Emits sent(id, {...}) or failed(id, {...}).
     */
    Q_INVOKABLE int send(const QString& envelopeXdrBase64);

    /**
     * The same send, given the envelope's network, which must be the one the
     * endpoint serves: the passphrase is compared, exactly, with the
     * endpoint's getNetwork answer before anything is posted, and one that
     * differs is refused (kind "validation", mayHaveReachedServer false). A
     * network applies a transaction under the hash its own passphrase gives
     * it, so the hash worked out from another passphrase is one that network
     * never gives it: a caller told to look that up would find nothing, and
     * might send again. (The given passphrase used to be trusted.)
     *
     * Only a fresh answer refuses it. When the passphrase differs from an
     * answer remembered from an earlier send, the endpoint is asked again
     * first (one more getNetwork): the RPC behind the same URL may serve
     * another network since, as a local one restarted on another network
     * does. A passphrase that matches the new answer is sent; if the network
     * cannot be read this time, nothing is sent and the failure has the
     * lookup's kind, as above. (The remembered answer was kept, and the right
     * passphrase refused, for as long as the endpoint stayed set.)
     *
     * The envelope must decode exactly or it is not sent (kind "validation",
     * before any request), and an empty passphrase is refused the same way.
     * It is posted as the SDK writes it back, as above.
     */
    Q_INVOKABLE int send(const QString& envelopeXdrBase64, const QString& networkPassphrase);

    /** Asks the server for its own health (getHealth), to prove it is there. */
    Q_INVOKABLE int checkHealth();

    /**
     * Cancel every request started so far. A request still waiting behind
     * the one on the wire is never sent, and the one on the wire is aborted,
     * so the next request starts at once. (It used to run to completion
     * first, which for an endpoint that never answered was never.)
     *
     * What is still reported depends on the kind of request, and for a send
     * on whether the worker had STARTED it — taken it up, rather than still
     * waiting behind another request — when this was called:
     *
     * - simulate(), prepare(), checkHealth(): nothing, started or not. They
     *   leave `pending` at once and no signal of theirs is delivered. None of
     *   them can change the ledger.
     * - A send that had not started: nothing. It is never sent.
     * - A send that had started: ALWAYS reported, exactly once, and counted
     *   in `pending` until then. When the cancel stopped it, as failed() with
     *   kind "cancelled": mayHaveReachedServer false ("cancelled before it
     *   was sent") when it was stopped before its sendTransaction was posted
     *   (during the endpoint's network lookup, see send(), or before it), and
     *   otherwise mayHaveReachedServer true with the transaction's `hash`
     *   (see failed()): it may still be applied, so look it up by that hash
     *   before sending it again or reusing its sequence number. When it ended
     *   on its own before the cancel could stop it (its answer already on its
     *   way back, or its envelope refused), with that end, sent() or
     *   failed(), as without the cancel. (Its report used to be dropped with
     *   the rest, so a wallet that cancelled while a send was on the wire
     *   never heard that it may land, and could pay again.)
     * - A request refused on the spot, which never becomes pending
     *   (simulate() of nothing, send() given an empty passphrase), still
     *   reports its failed(), kind "validation".
     *
     * An endpoint change cancels the same way but reports every request (see
     * `endpoint`). Destroying this object cancels as this does and reports
     * nothing, since nothing is left to report to: do not destroy it while a
     * send is under way.
     *
     * Destroying it also stops the worker: the cancel aborts the request on
     * the wire and every event loop the worker waits in is ended, so it
     * normally stops at once, and the destructor waits for that, up to 2 s. A
     * worker still busy after that (in a step that waits in no event loop,
     * or starved of CPU) is left to finish on its own thread, with a
     * warning, and the destructor returns: it ends as soon as that step
     * does, and deletes itself, its server and its network objects there,
     * and the thread object is deleted afterwards by this thread's event
     * loop, if one still runs. It is never terminated. (It was, after the
     * 2 s: that tore its network objects down from this thread, which a Qt 6
     * debug build can abort on.)
     */
    Q_INVOKABLE void cancelAll();

    // ─── Reading a contract, which touches no network ────────────────
    //
    // These two are synchronous and stay on the calling thread on purpose:
    // opening a local file and walking its sections takes microseconds, and
    // pushing that onto the worker would mean a signal round trip for an
    // answer that is already available. Only the RPC calls above need a
    // thread, and they need it because they block on a socket.

    /**
     * What a compiled contract says about itself, and the bytes it is made of.
     *
     * Returns { isWasm, hasSpec, protocolVersion, functions, metadata,
     * sections, problem } and, when the file really is WebAssembly,
     * { wasmSize, wasmHex, wasmHash } as well — the module as hex, ready for
     * an `uploadContractWasm` operation, and its SHA-256, which is both the
     * hash the ledger files the code under and the `wasmHash` a later
     * `createContract` points at. Uploaded code is content-addressed, so the
     * hash is known before a single byte is sent.
     *
     * `functions` is a list of
     * { name, doc, returns, inputs: [{ name, type, scvType, doc }] }, where
     * `scvType` is the builder's argument type (u128/i128/u256/i256 are given
     * as exact decimal text) — EMPTY when the contract wants something the
     * builder cannot build, such as a tuple, an Option or a user-defined
     * struct. Empty means that function cannot be called through the
     * builder's argument list yet; it does not mean substitute something
     * close.
     *
     * `problem` distinguishes the three failures that need different answers:
     * not WebAssembly at all, WebAssembly that is not a contract, and a real
     * contract that simply published no interface.
     */
    Q_INVOKABLE QVariantMap readContract(const QUrl& fileUrl) const;

    /**
     * The same, given a plain filesystem path.
     *
     * Separate from the QUrl overload because a Windows path is not a URL:
     * QUrl reads a path's drive letter as a URL scheme, which leaves no file
     * at the other end.
     */
    Q_INVOKABLE QVariantMap readContractPath(const QString& path) const;

    /** The same, for bytes pasted as hex rather than opened from disk. */
    Q_INVOKABLE QVariantMap readContractHex(const QString& hex) const;

    /**
     * A skeleton argument list for one function, ready to be edited.
     *
     * Generated from the contract's own interface, so the types are the ones
     * the contract declared rather than the ones a user guessed.
     */
    Q_INVOKABLE QString argumentTemplate(const QVariantMap& function) const;

signals:
    void endpointChanged();
    void pendingChanged();

    /**
     * { ok, error, returnValue, returnValueXdr, minResourceFee, resourceFee,
     *   instructions, readBytes, writeBytes, footprintReadOnly,
     *   footprintReadWrite, events, stateChanges, latestLedger,
     *   transactionData, auth }
     */
    void simulated(int requestId, const QVariantMap& result);

    /** An envelope with its footprint and resource fee filled in, ready to sign. */
    void prepared(int requestId, const QVariantMap& result);

    /**
     * { status, hash, latestLedger, errorResultXdr, events } — status is
     * "pending", "duplicate", "tryAgainLater" or "error"; `hash` is the RPC's,
     * and it is the one worked out for the envelope (an answer with another
     * fails instead).
     *
     * "tryAgainLater" and "error" answer the one send made. When Qt sent the
     * transaction again, by itself, after the connection that carried it
     * whole closed with no answer (Qt 5 does), they answer the later send
     * only, and the send fails instead, with `resent` (see failed()): the
     * RPC may have taken in, and applied, the first. (They were reported
     * here, as the RPC's final word.) "pending" and "duplicate" are reported
     * here either way: the RPC has the transaction.
     */
    void sent(int requestId, const QVariantMap& result);

    /**
     * { healthy, status, latestLedger, oldestLedger } (getHealth carries no
     * protocol version, and this said it did).
     */
    void health(int requestId, const QVariantMap& result);

    /**
     * { kind, message, mayHaveReachedServer, hash, rpcHash, resent }:
     *
     * - `kind`: "connection" (a timeout included), "rpc", "validation",
     *   "cancelled" (abandoned by an endpoint change, or a send that had
     *   started when cancelAll() was called) or "unknown".
     * - `message`: what happened, for a person.
     * - `mayHaveReachedServer`, on every failure: false only when the request
     *   is known never to have reached the server — refused before it was
     *   sent, cancelled before it was sent, or its connection never opened
     *   (SorobanTransportException::mayHaveReachedServer()). Anything else —
     *   a timeout, a cancel while on the wire, a connection lost midway, an
     *   RPC error, an answer that does not parse — may have come after the
     *   server took it in. For a send that means the transaction may still
     *   be applied: look it up by its hash (getTransaction) before sending it
     *   again or reusing its sequence number. For a send it is about the
     *   transaction: the network lookup made before it (see send()) may have
     *   reached the server when this is false, the transaction did not.
     * - `hash`: that hash, on every failed send that may have reached the
     *   server — the transaction's hash on the network the endpoint serves;
     *   absent on anything else.
     * - `rpcHash`: on a send the RPC answered with another hash than `hash`
     *   (kind "rpc", or "cancelled" when a cancel came in the same moment),
     *   the RPC's own. It is the one the transaction can land
     *   under — the RPC works the hash out with its network's passphrase,
     *   which is the one that network applies it under — so look THAT one up
     *   before sending again; `hash` can only be found there if the RPC's
     *   answer was wrong. An honest endpoint gives one only when its network
     *   changed since it was asked, and the next send asks again. (This
     *   failure used to carry only `hash`, and a caller looking it up got
     *   NOT_FOUND for a transaction that may have been applied.)
     * - `resent`: true on a failed send when Qt sent its sendTransaction
     *   again, by itself, after it had written it whole (Qt 5 does, on a new
     *   connection, when the one it was written on closes before an answer;
     *   SorobanTransportException::resent()), whatever the kind: what ended
     *   the send then came of a later send, while the RPC may have taken in,
     *   and applied, the first. mayHaveReachedServer is then true, with the
     *   hash, and the message starts "not final" and names the hash. That
     *   includes the RPC's own answer to the later send when it is ERROR (a
     *   txBAD_SEQ most likely means the first was applied) or TRY_AGAIN_LATER
     *   — kind "rpc", where it was sent() as final — an error answer, a
     *   failure in transport, and a cancel ("cancelled"). Absent otherwise.
     *   Look the transaction up by its hash before anything else: building
     *   it again with a fresh sequence number may pay twice. It is not set
     *   when the answer to the later send was PENDING or DUPLICATE (the RPC
     *   has the transaction), which includes the `rpcHash` failure.
     *
     * (The map once said none of it, so a send that never left and one that
     * may yet be applied looked the same.)
     */
    void failed(int requestId, const QVariantMap& error);

private slots:
    /**
     * Delivered from the worker thread by queued connection. `started` is
     * false for a request the worker dropped, cancelled, before it took it up.
     */
    void onWorkerFinished(int requestId, const QString& kind, const QVariantMap& payload,
                          bool started);

private:
    int start(const QString& method, const QString& argument,
              const QString& context = QString());
    void finish(int requestId);
    /** Tells the worker every request started so far is cancelled. */
    void cancelStarted();

    QString m_endpoint;
    QThread* m_thread = nullptr;
    SorobanWorker* m_worker = nullptr;
    int m_lastRequestId = 0;
    /** Endpoint changes so far: the network a send learned holds until the next. */
    int m_endpointChanges = 0;
    /** Requests whose outcome may still be reported, each true for a send. */
    QHash<int, bool> m_pending;
    /** Sends cancelAll() cancelled: reported only if the worker had started them. */
    QSet<int> m_cancelledSends;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(SorobanQml)

#endif // QSTELLAR_SOROBANQML_H
