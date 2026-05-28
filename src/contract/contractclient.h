#ifndef CONTRACTCLIENT_H
#define CONTRACTCLIENT_H

#include <QList>
#include <QString>
#include <QtGlobal>

#include "../stellaraddress.h"
#include "xdr/scval.h"

class Account;
class AssembledTransaction;
class KeyPair;
class Network;
class SorobanServer;

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
 *     transaction is discarded.
 *
 * Both flavours require an `Account` (with current sequence) because
 * soroban-rpc doesn't expose `getAccount`; the caller fetches it elsewhere
 * (Horizon, a saved account, or a freshly-issued one).
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

    /**
     * Build a Soroban tx that invokes `functionName(args)` on this
     * contract, wrap it in an `AssembledTransaction`, simulate it, and
     * (optionally) sign any auth entries that match `authSigner`.
     *
     * Caller owns the returned pointer.
     */
    AssembledTransaction* invoke(const QString& functionName,
                                 const QList<stellar::SCVal>& args,
                                 Account* sourceAccount,
                                 qint64 baseFee = 100,
                                 KeyPair* authSigner = nullptr);

    /**
     * Read-only invocation: build + simulate + return the result SCVal,
     * discarding the intermediate AssembledTransaction.
     */
    stellar::SCVal read(const QString& functionName,
                        const QList<stellar::SCVal>& args,
                        Account* sourceAccount,
                        qint64 baseFee = 100);
};

#endif // CONTRACTCLIENT_H
