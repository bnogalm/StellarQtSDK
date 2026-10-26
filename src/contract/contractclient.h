#ifndef CONTRACTCLIENT_H
#define CONTRACTCLIENT_H

#include <QList>
#include <QString>
#include <QtGlobal>

#include "../stellaraddress.h"
#include "xdr/scval.h"

#include "../keypair.h"
#include "../network.h"
#include "../qstellar_namespace.h"
QSTELLAR_FWD(Account)
QSTELLAR_FWD(AssembledTransaction)
QSTELLAR_FWD(SorobanServer)

QSTELLAR_BEGIN_NS


/**
 * High-level facade over `SorobanServer` for a single Soroban contract.
 * Mirrors Java SDK `ContractClient`. The two entry points are:
 *
 *   ContractClient::invoke — build + simulate a contract call and return
 *     an `AssembledTransaction` ready to be signed and submitted. The
 *     caller owns the returned pointer.
 *
 *   ContractClient::read — convenience wrapper around `invoke` that only
 *     simulates and returns the decoded result `SCVal`. The intermediate
 *     transaction is discarded, and the account's sequence is left alone.
 *
 * Both flavours take an `Account` with its current sequence number, which
 * the caller fetches (Horizon, a saved account, or a freshly-issued one).
 * The RPC can supply it: an account is a ledger entry, which js's
 * rpc.Server.getAccount and py's load_account read with getLedgerEntries.
 * This SDK has no such helper yet; SorobanServer::getLedgerEntries reads the
 * entry, to be decoded by hand.
 */
class ContractClient
{
    StellarAddress m_contract;
    SorobanServer* m_server = nullptr;
    Network* m_network = nullptr;

public:
    /** `contractId` is the C-strkey of the contract address. */
    ContractClient(const QString& contractId, SorobanServer* server, Network* network);
    /** Address-typed overload. Throws unless `contract.getType() == CONTRACT`. */
    ContractClient(const StellarAddress& contract, SorobanServer* server, Network* network);

    const StellarAddress& getContractAddress() const { return m_contract; }
    SorobanServer* getServer() const { return m_server; }

    /** How long a built transaction stays valid, as in js-stellar-sdk's contract client. */
    static const qint64 DEFAULT_TIMEOUT_SECONDS = 300;

    /**
     * Build a Soroban tx that invokes `functionName(args)` on this
     * contract, wrap it in an `AssembledTransaction`, simulate it, and
     * (optionally) sign any auth entries that match `authSigner`.
     *
     * The transaction expires `timeoutSeconds` after it is built; 0 means it
     * never expires, and only when asked for. (It used to be built with no
     * time bounds at all: signed, it stayed valid for as long as its sequence
     * number was unused.)
     *
     * The transaction takes `sourceAccount`'s next sequence number, as a
     * build does. If the simulation or the auth signing throws, the number
     * is handed back, unless something built on that account meanwhile (the
     * simulation waits in a nested event loop). In 1.0.3 a failed invoke()
     * and every read() kept the number: code that called
     * Account::decrementSequenceNumber() afterwards to make up for it must
     * drop that call. That holds for a simulation that timed out or was
     * cancelled as well: a simulation submits nothing, so it cannot have used
     * the number. Sending is the returned AssembledTransaction's signAndSend(),
     * which never hands the number back: a send that timed out or was
     * cancelled may still land (see its caveat).
     *
     * Caller owns the returned pointer.
     */
    AssembledTransaction* invoke(const QString& functionName,
                                 const QList<stellar::SCVal>& args,
                                 Account* sourceAccount,
                                 qint64 baseFee = 100,
                                 KeyPair* authSigner = nullptr,
                                 qint64 timeoutSeconds = DEFAULT_TIMEOUT_SECONDS);

    /**
     * Read-only invocation: build + simulate + return the result SCVal,
     * discarding the intermediate AssembledTransaction. Nothing is sent, so
     * `sourceAccount`'s sequence number is never advanced.
     */
    stellar::SCVal read(const QString& functionName,
                        const QList<stellar::SCVal>& args,
                        Account* sourceAccount,
                        qint64 baseFee = 100,
                        qint64 timeoutSeconds = DEFAULT_TIMEOUT_SECONDS);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(ContractClient)

#endif // CONTRACTCLIENT_H
