#include "contractclient.h"

#include <memory>
#include <stdexcept>

#include "account.h"
#include "assembledtransaction.h"
#include "invokehostfunctionoperation.h"
#include "keypair.h"
#include "network.h"
#include "sorobanserver.h"
#include "transaction.h"
#include "transactionbuilder.h"

QSTELLAR_BEGIN_NS


ContractClient::ContractClient(const QString& contractId, SorobanServer* server, Network* network)
    : m_contract(StellarAddress::fromString(contractId)), m_server(server), m_network(network)
{
    if (m_contract.getType() != StellarAddress::Type::CONTRACT) {
        throw std::runtime_error("ContractClient: contractId must be a C-strkey");
    }
    if (!server)  throw std::runtime_error("ContractClient: null server");
    if (!network) throw std::runtime_error("ContractClient: null network");
}

ContractClient::ContractClient(const StellarAddress& contract, SorobanServer* server, Network* network)
    : m_contract(contract), m_server(server), m_network(network)
{
    if (contract.getType() != StellarAddress::Type::CONTRACT) {
        throw std::runtime_error("ContractClient: address must be of type CONTRACT");
    }
    if (!server)  throw std::runtime_error("ContractClient: null server");
    if (!network) throw std::runtime_error("ContractClient: null network");
}

AssembledTransaction* ContractClient::invoke(const QString& functionName,
                                             const QList<stellar::SCVal>& args,
                                             Account* sourceAccount,
                                             qint64 baseFee,
                                             KeyPair* authSigner,
                                             qint64 timeoutSeconds)
{
    if (!sourceAccount) throw std::runtime_error("ContractClient::invoke: null sourceAccount");

    // build() takes the next sequence number from the caller's account, so a
    // transaction built on it while the simulation below waits in its event
    // loop gets the one after. A simulation or auth signing that failed used
    // to keep the number, and the next transaction was refused with
    // tx_bad_seq: it is handed back — unless a later one was taken meanwhile.
    TransactionBuilder b(sourceAccount, m_network);
    b.addOperation(
        InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
            m_contract, functionName, args));
    b.setBaseFee(baseFee);
    b.setTimeout(timeoutSeconds);

    std::unique_ptr<AssembledTransaction> at(new AssembledTransaction(b.build(), m_server, m_network));
    const qint64 reserved = sourceAccount->getSequenceNumber();
    try {
        at->simulate();
        if (authSigner) {
            at->signAuthEntries(authSigner);
        }
    } catch (...) {
        if (sourceAccount->getSequenceNumber() == reserved)
            sourceAccount->decrementSequenceNumber();
        throw;
    }
    return at.release();
}

stellar::SCVal ContractClient::read(const QString& functionName,
                                    const QList<stellar::SCVal>& args,
                                    Account* sourceAccount,
                                    qint64 baseFee,
                                    qint64 timeoutSeconds)
{
    if (!sourceAccount) throw std::runtime_error("ContractClient::read: null sourceAccount");

    // Simulated only, never sent: it must not use up the caller's sequence
    // number, as every read() did.
    Account scratch(sourceAccount->getAccountId(), sourceAccount->getSequenceNumber());
    std::unique_ptr<AssembledTransaction> at(
        invoke(functionName, args, &scratch, baseFee, nullptr, timeoutSeconds));
    return at->result();
}
QSTELLAR_END_NS
