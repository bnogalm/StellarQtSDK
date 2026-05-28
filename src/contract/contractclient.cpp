#include "contractclient.h"

#include <stdexcept>

#include "account.h"
#include "assembledtransaction.h"
#include "invokehostfunctionoperation.h"
#include "keypair.h"
#include "network.h"
#include "sorobanserver.h"
#include "transaction.h"
#include "transactionbuilder.h"

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
                                             KeyPair* authSigner)
{
    if (!sourceAccount) throw std::runtime_error("ContractClient::invoke: null sourceAccount");

    TransactionBuilder b(sourceAccount, m_network);
    b.addOperation(
        InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
            m_contract, functionName, args));
    b.setBaseFee(baseFee);
    b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);

    Transaction* tx = b.build();
    auto* at = new AssembledTransaction(tx, m_server, m_network);
    at->simulate();
    if (authSigner) {
        at->signAuthEntries(authSigner);
    }
    return at;
}

stellar::SCVal ContractClient::read(const QString& functionName,
                                    const QList<stellar::SCVal>& args,
                                    Account* sourceAccount,
                                    qint64 baseFee)
{
    AssembledTransaction* at = invoke(functionName, args, sourceAccount, baseFee, nullptr);
    stellar::SCVal r = at->result();
    delete at;
    return r;
}
