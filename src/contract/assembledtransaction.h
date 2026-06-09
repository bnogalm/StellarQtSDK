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
#include "../qstellar_namespace.h"
QSTELLAR_FWD(Account)
QSTELLAR_FWD(Transaction)
QSTELLAR_FWD(SorobanServer)

QSTELLAR_BEGIN_NS


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
 * Iter 2 covers the no-auth path:
 *   - SOROBAN_CREDENTIALS_SOURCE_ACCOUNT credentials (the caller is the
 *     transaction's source account).
 *   - auth-entry signing + footprint restore land in Iter 3.
 *
 * Methods are chainable: `at.simulate().sign(kp).signAndSend(kp).result()`.
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
     * simulation error. Chainable.
     */
    AssembledTransaction& simulate();

    /** True after a successful `simulate()` call. */
    bool isSimulated() const { return m_simulated; }
    const SimulateTransactionResponse& getSimulationResponse() const { return m_sim; }

    /**
     * Return the decoded host-function result (SCVal) from the simulation.
     * Throws if not simulated or if the simulation produced no result.
     */
    stellar::SCVal result() const;

    /** Attach a signature for `signer` to the internal transaction.
     *  Required before `signAndSend`. Chainable. */
    AssembledTransaction& sign(KeyPair* signer);

    /**
     * Return the distinct list of `G…` strkeys whose auth entries the
     * simulation is requesting a signature for (i.e. entries with
     * `SOROBAN_CREDENTIALS_ADDRESS` and an empty signature). Only
     * `SC_ADDRESS_TYPE_ACCOUNT` entries are reported — contract addresses
     * cannot sign with a `KeyPair`.
     *
     * Throws if `simulate()` has not been called yet.
     */
    QStringList needsNonInvokerSigningBy() const;

    /**
     * For every auth entry in the internal transaction whose credentials
     * are `SOROBAN_CREDENTIALS_ADDRESS` with an empty signature AND whose
     * address matches `signer`, run `SorobanCredentialsSigner::authorizeEntry`.
     *
     * `validUntilLedger == 0` defaults to `simulationResponse.latestLedger + 100`.
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
     * Throws if the simulation didn't request a restore. After this call
     * completes successfully, the original transaction's sequence number
     * is stale; the caller is expected to refresh + re-simulate.
     */
    GetTransactionResponse restoreFootprint(Account* sourceAccount,
                                            KeyPair* signer,
                                            int baseFee = 100,
                                            int timeoutMs = 60000,
                                            int intervalMs = 1000);

    /**
     * Sign + sendTransaction + pollTransaction until SUCCESS/FAILED.
     * Returns the final `GetTransactionResponse`. Caches it; subsequent
     * `getFinalResponse()` returns the same value.
     */
    GetTransactionResponse signAndSend(KeyPair* signer,
                                       int timeoutMs = 60000,
                                       int intervalMs = 1000);

    /** Pre-cached responses for inspection / testing. */
    const SendTransactionResponse& getSendResponse() const { return m_sendResponse; }
    const GetTransactionResponse& getFinalResponse() const { return m_finalResponse; }

    /** Internal `Transaction*` — ownership stays with this object. */
    Transaction* getTransaction() const { return m_tx; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(AssembledTransaction)

#endif // ASSEMBLEDTRANSACTION_H
