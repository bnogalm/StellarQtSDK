#ifndef ASSEMBLEDTRANSACTION_H
#define ASSEMBLEDTRANSACTION_H

#include <QStringList>
#include <QtGlobal>
#include "../responses/sorobanrpc/simulatetransactionresponse.h"
#include "../responses/sorobanrpc/sendtransactionresponse.h"
#include "../responses/sorobanrpc/gettransactionresponse.h"
#include "xdr/scval.h"

#include "../keypair.h"
#include "../network.h"
#include "../exception/assembledtransactionexception.h"
#include "../qstellar_namespace.h"
QSTELLAR_FWD(Account)
QSTELLAR_FWD(Transaction)
QSTELLAR_FWD(SorobanServer)

namespace qstellar { namespace exception {

/**
 * Thrown by `AssembledTransaction::restoreFootprint()` when sending the
 * restore failed in a way that may have come after the RPC took it in: a
 * timeout, a cancel, a connection lost midway, an error answer or one that
 * does not parse, an answer to a send Qt made again by itself that does not
 * say the RPC has it (SorobanTransportException::resent(): an ERROR or
 * TRY_AGAIN_LATER among them) — or when the RPC took it in (PENDING or
 * DUPLICATE) and polling for its result then failed, timed out or was
 * cancelled. The transaction may still be applied, so its sequence number
 * stays taken; look it up by `getTransactionHash()` (getTransaction) before
 * building on the account again. The message carries the original failure
 * and the hash.
 *
 * Thrown by `AssembledTransaction::signAndSend()` when Qt sent the
 * transaction again, by itself (SorobanTransportException::resent()),
 * whatever ended the send: the RPC may have taken in, and applied, the first
 * send whatever it said to the later one.
 */
class SendOutcomeUnknownException : public AssembledTransactionException
{
    QString m_transactionHash;
public:
    SendOutcomeUnknownException(const QString& message, const QString& transactionHash)
        : AssembledTransactionException(message), m_transactionHash(transactionHash) {}

    /** Hex hash of the transaction whose send may have reached the server. */
    QString getTransactionHash() const { return m_transactionHash; }
};

/**
 * Thrown by `AssembledTransaction::sign()` / `signAndSend()` when the
 * simulation carries a restorePreamble (js and py ExpiredState): entries the
 * call needs are archived, and sent, it fails on chain with the fee paid.
 * `force` does not skip it, as in js and py. Restore them with
 * `restoreFootprint()`, then simulate the call again (see there).
 */
class ExpiredStateException : public AssembledTransactionException
{
public:
    explicit ExpiredStateException(const QString& message) : AssembledTransactionException(message) {}
};

/**
 * Thrown by `AssembledTransaction::sign()` / `signAndSend()` for a read call
 * (`isReadCall()`) unless `force` is true, as js and py throw
 * NoSignatureNeeded: it changes nothing, and sending it only pays the fee.
 * Its answer is `result()`; `ContractClient::read` gets it with no
 * transaction to keep.
 */
class NoSignatureNeededException : public AssembledTransactionException
{
public:
    explicit NoSignatureNeededException(const QString& message) : AssembledTransactionException(message) {}
};

/**
 * Thrown by `AssembledTransaction::sign()` / `signAndSend()` while auth
 * entries of G... addresses are unsigned, as js and py throw
 * NeedsMoreSignatures: sent, the host refuses the authorization when the
 * transaction is applied, and the fee is paid. `force` does not skip it, as
 * in js and py. `getAddresses()` is what needsNonInvokerSigningBy() listed;
 * each of them signs its entries with `signAuthEntries()`.
 */
class NeedsMoreSignaturesException : public AssembledTransactionException
{
    QStringList m_addresses;
public:
    NeedsMoreSignaturesException(const QString& message, const QStringList& addresses)
        : AssembledTransactionException(message), m_addresses(addresses) {}

    /** The G... strkeys whose entries are not signed yet. */
    QStringList getAddresses() const { return m_addresses; }
};

}} // namespace qstellar::exception

QSTELLAR_BEGIN_NS

struct SimulateTransactionOptions;   // sorobanserver.h

/**
 * High-level wrapper around a built Soroban `Transaction`. Mirrors Java SDK
 * `AssembledTransaction`. Encapsulates the lifecycle:
 *
 *     simulate → (sign auth entries) → sign → send → poll → result
 *
 * Ownership: takes ownership of the `Transaction*` passed to the
 * constructor. The `SorobanServer*` and `Network*` are borrowed (caller
 * keeps ownership).
 *
 * Supports both the no-auth path (SOROBAN_CREDENTIALS_SOURCE_ACCOUNT — the
 * caller is the transaction's source account) and non-invoker auth-entry
 * signing (needsNonInvokerSigningBy / signAuthEntries) plus footprint restore.
 *
 * Methods are chainable: `at.simulate().sign(kp)`, then `signAndSend(kp)`,
 * which returns the final GetTransactionResponse.
 *
 * Two results: result() is the simulation's; landedResult(), after
 * signAndSend(), is the value the call returned on chain, read from the
 * transaction's meta as js-stellar-sdk's SentTransaction.result reads it.
 * They differ when state changed in between (a counter, a minted id).
 */
class AssembledTransaction
{
    Transaction* m_tx = nullptr;
    SorobanServer* m_server = nullptr;
    Network* m_network = nullptr;
    bool m_simulated = false;
    SimulateTransactionResponse m_sim;
    SendTransactionResponse m_sendResponse;
    GetTransactionResponse m_finalResponse;

    AssembledTransaction& applySimulation(const SimulateTransactionResponse& sim);
    void refuseToSign(const char* method, bool force) const;

public:
    /** Takes ownership of `tx`. `server` and `network` are borrowed. */
    AssembledTransaction(Transaction* tx, SorobanServer* server, Network* network);
    ~AssembledTransaction();

    AssembledTransaction(const AssembledTransaction&) = delete;
    AssembledTransaction& operator=(const AssembledTransaction&) = delete;

    /**
     * Run `simulateTransaction` on the server and apply the result to the
     * internal transaction (fee bump, sorobanData attached, auth filled for
     * the first InvokeHostFunctionOp that has empty auth). Throws on
     * simulation error (SimulationFailedException). Chainable.
     *
     * A simulation that fails, or whose answer cannot be applied, leaves the
     * transaction unsimulated (isSimulated() is false), even after an
     * earlier one succeeded: the transaction still carries that earlier
     * simulation's soroban data, fee and auth, and sign() refuses it until a
     * simulation succeeds again. It used to be signed and sent.
     *
     * Refused once the transaction is signed (AssembledTransactionException),
     * as py's prepare(), its re-simulation of the built transaction, refuses
     * it: the soroban data and the fee it changes are in the signed body. It
     * used to go ahead and keep the signature over the old body, and
     * signAndSend then added a second one, which core refuses
     * (txBAD_AUTH_EXTRA). py's simulate() builds a fresh, unsigned
     * transaction instead, and js signs a fresh copy.
     */
    AssembledTransaction& simulate();

    /**
     * simulate(), sending what `options` asks for besides the transaction:
     * an instruction leeway, an auth mode, CAP-71 credentials (see
     * SimulateTransactionOptions in sorobanserver.h), as js's
     * simulate({useUpgradedAuth}) and py's addl_resources / auth_mode do.
     * It goes through SorobanServer::simulateTransactionXdr(envelope,
     * options), not the one-argument overload a subclass may override.
     */
    AssembledTransaction& simulate(const SimulateTransactionOptions& options);

    /** True after a successful `simulate()` call; false again once a later
     *  one fails. */
    bool isSimulated() const { return m_simulated; }
    const SimulateTransactionResponse& getSimulationResponse() const { return m_sim; }

    /**
     * Whether the call only reads, as js and py decide it (isReadCall,
     * is_read_call): an invocation whose simulation asked for no
     * authorization and whose footprint writes nothing. Its answer is
     * result(); signed and sent, it changes nothing and pays the fee, so
     * sign() and signAndSend() refuse it unless `force`. An ExtendFootprintTTL
     * or RestoreFootprint transaction is not one (it changes TTLs, or
     * restores). Throws NotYetSimulatedException before simulate().
     */
    bool isReadCall() const;

    /**
     * Return the decoded host-function result (SCVal) from the SIMULATION,
     * not the value the call returned on chain (see the class comment).
     * Throws if not simulated or if the simulation produced no result.
     */
    stellar::SCVal result() const;

    /**
     * The value the call returned ON CHAIN, after signAndSend() landed it
     * (js SentTransaction.result): GetTransactionResponse::getReturnValue()
     * of the final response. Throws AssembledTransactionException before a
     * successful signAndSend(), or when the meta carries no value.
     */
    stellar::SCVal landedResult() const;

    /**
     * Attach a signature for `signer` to the internal transaction.
     * Required before `signAndSend`. Chainable.
     *
     * Refused before simulate() (NotYetSimulatedException), as js and py
     * refuse it: an unsimulated Soroban transaction has no footprint and
     * fails. Refused while the simulation asks for a restore
     * (ExpiredStateException; see restoreFootprint()). Both used to be signed.
     *
     * A read call (isReadCall()) is refused unless `force` is true
     * (NoSignatureNeededException), as js's and py's sign({force}) refuse it.
     * It used to be signed, and signAndSend() sent it and paid the fee.
     *
     * Refused while auth entries of G... addresses are unsigned
     * (needsNonInvokerSigningBy() is not empty, ADDRESS_WITH_DELEGATES
     * entries included; NeedsMoreSignaturesException),
     * `force` or not, as js and py refuse it: the host would refuse the
     * authorization and the fee be paid. It used to be signed and sent.
     *
     * Refused after a simulate() that failed, even when an earlier one
     * succeeded (SimulationFailedException; NotYetSimulatedException when the
     * answer was not an error but could not be applied), `force` or not, as
     * js refuses it (SimulationFailed); py refuses it unless forced. The
     * transaction carried the earlier simulation's footprint and fee, and it
     * used to be signed and sent.
     *
     * signAndSend() refuses what sign() refuses.
     */
    AssembledTransaction& sign(KeyPair* signer, bool force = false);

    /**
     * Return the distinct list of `G…` strkeys whose auth entries the
     * simulation is requesting a signature for: entries with
     * `SOROBAN_CREDENTIALS_ADDRESS` or `_ADDRESS_V2` credentials whose
     * signature is SCV_VOID, the placeholder the RPC records (py counts that
     * one only too, and ADDRESS entries only; js also counts an empty vec).
     * An `_ADDRESS_WITH_DELEGATES` entry is listed when its top-level
     * signature is SCV_VOID, as js lists it (py has no such entries); its
     * delegates' signatures are not looked at, as in js. It used to be
     * skipped, and sign() signed and sent a call the host then refused.
     * signAuthEntries() cannot sign one (SorobanCredentialsSigner::
     * authorizeEntry refuses delegates; js signs the top-level credentials):
     * sign it elsewhere and put it back with InvokeHostFunctionOperation::
     * setAuth.
     * An entry with SOURCE_ACCOUNT credentials is the invoker's, covered by
     * the envelope signature, and is not listed.
     *
     * Two differences from js's and py's needsNonInvokerSigningBy. Contract
     * (C...) addresses are left out, since signAuthEntries() signs with a
     * KeyPair only; js and py list them, and a smart wallet's entry is still
     * to be signed (see signAuthEntries()). sign() refuses on this list, so
     * on the G... ones only, as js does; py refuses on C... ones too. And
     * there is no includeAlreadySigned, which both have.
     *
     * Throws if `simulate()` has not been called yet.
     */
    QStringList needsNonInvokerSigningBy() const;

    /**
     * For every auth entry in the internal transaction whose credentials
     * are `SOROBAN_CREDENTIALS_ADDRESS` with an empty signature AND whose
     * address matches `signer`, run `SorobanCredentialsSigner::authorizeEntry`.
     * `_ADDRESS_V2` entries are signed too; `_ADDRESS_WITH_DELEGATES` ones
     * are left as they are (see needsNonInvokerSigningBy()).
     *
     * `validUntilLedger == 0` defaults to `simulationResponse.latestLedger + 100`.
     *
     * Refused once the transaction is signed, as simulate() is and as py
     * refuses it ("Authorization entries must be signed before transaction
     * signing"): the auth entries are in the signed body.
     *
     * Nothing to sign is not an error: a key with no unsigned entry (another
     * account's key, an entry already signed, the invoker's own key when its
     * entry carries SOURCE_ACCOUNT credentials) leaves the transaction as it
     * was. py does the same, except that it signs an entry already signed
     * again, with the new validUntilLedger; js throws
     * (NoUnsignedNonInvokerAuthEntries, NoSignatureNeeded). needsNonInvokerSigningBy() says who is left, and
     * sign() refuses while anyone is. Only a KeyPair signs here, so only G...
     * entries: js's signAuthEntry / authorizeEntry callbacks, py's
     * AuthorizationSigner (a contract account, an outside signer) and
     * authorizeInvocation have no counterpart. An entry signed elsewhere is
     * signed over SorobanCredentialsSigner::hashedSignaturePayload and put
     * back with InvokeHostFunctionOperation::setAuth.
     *
     * Chainable.
     */
    AssembledTransaction& signAuthEntries(KeyPair* signer, quint32 validUntilLedger = 0);

    /**
     * Build, sign, send and poll a `RestoreFootprint` transaction whose
     * `SorobanTransactionData` is taken from the simulation's
     * `restorePreamble`. The caller supplies the source `Account` (with
     * the current sequence number — we can't refresh it from soroban-rpc
     * directly) and the signer. Returns the polled `GetTransactionResponse`.
     *
     * Throws if the simulation didn't request a restore, or if its
     * transactionData does not decode in full. Once it has landed, simulate()
     * the call again: sign() and signAndSend() refuse it while the simulation
     * asks for a restore (ExpiredStateException). When the restore took its
     * number from the account the call was built on, the call's number is
     * used up, and simulate() keeps a transaction's number: build the call
     * again instead (ContractClient::invoke). js and py can restore and
     * rebuild by themselves (restore: true); that is not done here.
     *
     * Since Protocol 23 an RPC restores what an invocation needs by itself
     * (the simulation lists it in archivedSorobanEntries) and returns no
     * restorePreamble for it; a preamble comes, for instance, when simulating
     * an ExtendFootprintTTL of an archived entry. Pass an Account whose
     * number is the one on the ledger: the Account an invocation was just
     * built from has already moved on, and the restore then takes N + 2,
     * which the RPC refuses (txBAD_SEQ; the number is handed back).
     *
     * The restore transaction takes `sourceAccount`'s next sequence number,
     * as a build does. It is handed back only for a failure known to come
     * before the RPC could take the transaction in — signing or encoding
     * throws, the RPC answers the one send made with ERROR or
     * TRY_AGAIN_LATER (SendFailedException), or the send throws
     * SorobanTransportException with mayHaveReachedServer() false (a
     * connection that never opened) — and only if nothing was built on that
     * account meanwhile (the send waits in a nested event loop). Any other
     * failure of the send — a timeout, a cancel, a connection lost midway, an
     * RPC error, an answer that does not parse — may have come after the RPC
     * took it in, and the restore may still land: the number stays taken and
     * SendOutcomeUnknownException says so, with the transaction's hash to
     * look up (getTransaction). So does an ERROR or TRY_AGAIN_LATER that
     * answers a send Qt made again, by itself, after the connection that
     * carried the restore whole closed with no answer (Qt 5 does; see
     * SorobanTransportException::resent()): it answers the later send only,
     * and the RPC may have applied the first (an ERROR with txBAD_SEQ most
     * likely means it did). It was taken as the answer to the send, and the
     * number handed back while the restore could land. Once
     * the RPC has answered PENDING or DUPLICATE the number stays taken too,
     * even if polling then fails, times out or is cancelled: that throws
     * SendOutcomeUnknownException as well, with the hash. (It came through as
     * the poll threw it, without the hash — and a poll whose own connection
     * never opened as a SorobanTransportException with mayHaveReachedServer()
     * false, which is what a handed-back number is told by.) In 1.0.3 the
     * number was always kept, and TRY_AGAIN_LATER was polled until the
     * timeout; it now throws SendFailedException at once. (Handing the
     * number back after a send that may have reached the RPC would let the
     * next transaction built on the account take the number of a restore
     * that then lands, and be refused with tx_bad_seq.)
     *
     * `baseFee` is 100 to 4294967295 stroops (it was an int, so a 64-bit fee
     * wrapped silently). The restore transaction expires `txTimeoutSeconds`
     * after it is built; 0 means never, and only when asked for.
     * `timeoutMs` / `intervalMs` are how long and how often to poll.
     */
    GetTransactionResponse restoreFootprint(Account* sourceAccount,
                                            KeyPair* signer,
                                            qint64 baseFee = 100,
                                            int timeoutMs = 60000,
                                            int intervalMs = 1000,
                                            qint64 txTimeoutSeconds = 300);

    /**
     * Sign + sendTransaction + pollTransaction until SUCCESS/FAILED.
     * Returns the final `GetTransactionResponse`. Caches it; subsequent
     * `getFinalResponse()` returns the same value.
     *
     * Refuses, before anything is signed or sent, what sign() refuses; with
     * `force`, a read call is signed and sent all the same, as sign()'s.
     *
     * An answer of ERROR or TRY_AGAIN_LATER throws SendFailedException at
     * once, with the answer in `getSendResponse()`, as restoreFootprint does;
     * in 1.0.3 TRY_AGAIN_LATER was polled until `timeoutMs`, as if PENDING.
     *
     * That is for an answer to the one send made. When Qt sent the
     * transaction again, by itself, after the connection that carried it
     * whole closed with no answer (Qt 5 does; SorobanTransportException::
     * resent()), whatever ended the send came of the later send, while the
     * RPC may have taken in, and applied, the first: an ERROR (a txBAD_SEQ
     * most likely means the first was applied), a TRY_AGAIN_LATER, an error
     * or a failure in transport. That throws SendOutcomeUnknownException
     * with the transaction's hash, and `getSendResponse()` is left as it was:
     * the outcome is not known, so look the transaction up by that hash
     * before sending it again or handing its number back. (Such an ERROR or
     * TRY_AGAIN_LATER threw SendFailedException, which says the RPC did not
     * take it in.) A PENDING or DUPLICATE answer to the later send is polled
     * as usual: the RPC has the transaction.
     *
     * The sequence number is not handed back. It was taken from the source
     * Account when the transaction was built (ContractClient::invoke, or the
     * caller's own build), and this object never holds that Account. The
     * transaction keeps it and stays valid until its time bounds expire, so
     * after TRY_AGAIN_LATER call signAndSend again — the same envelope is
     * sent: a signer whose signature it already carries is not added twice
     * (a second copy is an extra signature, which the network refuses). To
     * abandon the transaction instead, and only if nothing was built on the
     * account since and the RPC never answered PENDING or DUPLICATE for it,
     * hand the number back with Account::decrementSequenceNumber(). Once the
     * RPC has answered PENDING or DUPLICATE the transaction may land whatever
     * the polling says, and the number is used.
     *
     * The same holds for a send that throws: only a
     * SorobanTransportException whose mayHaveReachedServer() is false (a
     * connection that never opened) is known not to have reached the RPC.
     * Any other — a timeout, a cancel (SorobanServer::cancelRequests), a
     * connection lost midway, an RPC error, an answer that does not parse —
     * may have come after the RPC took the transaction in, and it may still
     * land: look it up by its hash (getTransaction()->hashHex()) before
     * sending it again or handing its number back. A cancel while polling
     * throws too, and the transaction may land all the same. So does a poll
     * whose own connection never opened: it throws SorobanTransportException
     * with mayHaveReachedServer() true, since the transaction did reach the
     * RPC. (It came through with false — "the request was not sent" — which
     * said of a transaction the RPC had taken in that it never left.)
     */
    GetTransactionResponse signAndSend(KeyPair* signer,
                                       int timeoutMs = 60000,
                                       int intervalMs = 1000,
                                       bool force = false);

    /** Pre-cached responses for inspection / testing. */
    const SendTransactionResponse& getSendResponse() const { return m_sendResponse; }
    const GetTransactionResponse& getFinalResponse() const { return m_finalResponse; }

    /** Internal `Transaction*` — ownership stays with this object. */
    Transaction* getTransaction() const { return m_tx; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(AssembledTransaction)

#endif // ASSEMBLEDTRANSACTION_H
