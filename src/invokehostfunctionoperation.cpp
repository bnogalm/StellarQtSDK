#include "invokehostfunctionoperation.h"

#include <cstring>
#include <stdexcept>

#include "scval/scv.h"

QSTELLAR_BEGIN_NS


namespace {

stellar::SCAddress toScAddressXdr(const StellarAddress& addr)
{
    stellar::SCVal v = Scv::toAddress(addr);
    return v.address;
}

} // namespace

InvokeHostFunctionOperation::InvokeHostFunctionOperation(const stellar::InvokeHostFunctionOp& op)
    : m_hostFunction(op.hostFunction)
    , m_auth(op.auth)
{
}

InvokeHostFunctionOperation* InvokeHostFunctionOperation::setAuth(
    const QList<stellar::SorobanAuthorizationEntry>& auth)
{
    m_auth = auth;
    return this;
}

InvokeHostFunctionOperation* InvokeHostFunctionOperation::addAuth(
    const stellar::SorobanAuthorizationEntry& entry)
{
    m_auth.append(entry);
    return this;
}

void InvokeHostFunctionOperation::fillOperationBody(AccountConverter& accountConverter,
                                                    stellar::Operation& operation)
{
    Q_UNUSED(accountConverter)
    stellar::InvokeHostFunctionOp& op = operation.fillInvokeHostFunctionOp();
    op.hostFunction = m_hostFunction;
    op.auth = m_auth;
}

InvokeHostFunctionOperation* InvokeHostFunctionOperation::build(const stellar::InvokeHostFunctionOp& op)
{
    return new InvokeHostFunctionOperation(op);
}

namespace {

/**
 * Refuses a claimable-balance (B...) or liquidity-pool (L...) address among
 * contract arguments, at any depth. CAP-67 made them SCAddress kinds for
 * events only: the host refuses them as contract input, so a transaction
 * carrying one was signed and then failed on chain, its fee paid. js refuses
 * them ("claimable balances and liquidity pools cannot be arguments to
 * invokeHostFunction"); it checks the top level, the host every level.
 */
void refuseEventOnlyAddresses(const stellar::SCVal& v, int depth = 0)
{
    if (depth > stellar::MAX_SCVAL_DEPTH)
        return;             // too deep to sign anyway: the encoder refuses it
    if (v.type == stellar::SCValType::SCV_ADDRESS
            && (v.address.type == stellar::SCAddressType::SC_ADDRESS_TYPE_CLAIMABLE_BALANCE
                || v.address.type == stellar::SCAddressType::SC_ADDRESS_TYPE_LIQUIDITY_POOL))
        throw std::runtime_error("claimable balances and liquidity pools cannot be arguments "
                                 "to invokeHostFunction");
    if (v.type == stellar::SCValType::SCV_VEC && v.vec) {
        for (const stellar::SCVal& item : *v.vec)
            refuseEventOnlyAddresses(item, depth + 1);
    }
    if (v.type == stellar::SCValType::SCV_MAP && v.map) {
        for (const stellar::SCMapEntry& entry : *v.map) {
            refuseEventOnlyAddresses(entry.key, depth + 1);
            refuseEventOnlyAddresses(entry.val, depth + 1);
        }
    }
}

} // namespace

// ─── Factories ──────────────────────────────────────────────────────

InvokeHostFunctionOperation* InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
    const StellarAddress& contractAddress,
    const QString& functionName,
    const QList<stellar::SCVal>& args)
{
    if (contractAddress.getType() != StellarAddress::Type::CONTRACT) {
        throw std::runtime_error("invokeContract: address must be a contract (C-strkey)");
    }
    // The name is an SCSymbol, so the host takes only what Scv::toSymbol does:
    // at most 32 of [A-Za-z0-9_]. "trans-fer" or "a b" was signed and then
    // refused by the host. No contract exports an empty name either.
    if (functionName.isEmpty()) {
        throw std::runtime_error("functionName must not be empty");
    }
    const QByteArray fnBytes = Scv::toSymbol(functionName).symbol;
    for (const stellar::SCVal& arg : args)
        refuseEventOnlyAddresses(arg);

    InvokeHostFunctionOperation* op = new InvokeHostFunctionOperation();
    op->m_hostFunction.type = stellar::HostFunctionType::HOST_FUNCTION_TYPE_INVOKE_CONTRACT;
    op->m_hostFunction.invokeContract.contractAddress = toScAddressXdr(contractAddress);
    op->m_hostFunction.invokeContract.functionName = fnBytes;
    op->m_hostFunction.invokeContract.args = args;
    return op;
}

InvokeHostFunctionOperation* InvokeHostFunctionOperation::uploadContractWasmOperationBuilder(
    const QByteArray& wasm)
{
    InvokeHostFunctionOperation* op = new InvokeHostFunctionOperation();
    op->m_hostFunction.type = stellar::HostFunctionType::HOST_FUNCTION_TYPE_UPLOAD_CONTRACT_WASM;
    op->m_hostFunction.uploadWasm = wasm;
    return op;
}

InvokeHostFunctionOperation* InvokeHostFunctionOperation::createContractOperationBuilder(
    const StellarAddress& fromAddress,
    const QByteArray& salt,
    const stellar::ContractExecutable& executable)
{
    if (salt.size() != 32) {
        throw std::runtime_error("salt must be 32 bytes");
    }
    InvokeHostFunctionOperation* op = new InvokeHostFunctionOperation();
    op->m_hostFunction.type = stellar::HostFunctionType::HOST_FUNCTION_TYPE_CREATE_CONTRACT;
    op->m_hostFunction.createContract.contractIDPreimage.type =
        stellar::ContractIDPreimageType::CONTRACT_ID_PREIMAGE_FROM_ADDRESS;
    op->m_hostFunction.createContract.contractIDPreimage.fromAddressAddr = toScAddressXdr(fromAddress);
    std::memcpy(op->m_hostFunction.createContract.contractIDPreimage.fromAddressSalt,
                salt.constData(), 32);
    op->m_hostFunction.createContract.executable = executable;
    return op;
}

InvokeHostFunctionOperation* InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(
    const stellar::Asset& asset)
{
    InvokeHostFunctionOperation* op = new InvokeHostFunctionOperation();
    op->m_hostFunction.type = stellar::HostFunctionType::HOST_FUNCTION_TYPE_CREATE_CONTRACT;
    op->m_hostFunction.createContract.contractIDPreimage.type =
        stellar::ContractIDPreimageType::CONTRACT_ID_PREIMAGE_FROM_ASSET;
    op->m_hostFunction.createContract.contractIDPreimage.fromAssetAsset = asset;
    // SAC always maps to the STELLAR_ASSET executable.
    op->m_hostFunction.createContract.executable.type =
        stellar::ContractExecutableType::CONTRACT_EXECUTABLE_STELLAR_ASSET;
    return op;
}

InvokeHostFunctionOperation* InvokeHostFunctionOperation::createContractV2OperationBuilder(
    const StellarAddress& fromAddress,
    const QByteArray& salt,
    const stellar::ContractExecutable& executable,
    const QList<stellar::SCVal>& constructorArgs)
{
    if (salt.size() != 32) {
        throw std::runtime_error("salt must be 32 bytes");
    }
    for (const stellar::SCVal& arg : constructorArgs)
        refuseEventOnlyAddresses(arg);
    InvokeHostFunctionOperation* op = new InvokeHostFunctionOperation();
    op->m_hostFunction.type = stellar::HostFunctionType::HOST_FUNCTION_TYPE_CREATE_CONTRACT_V2;
    op->m_hostFunction.createContractV2.contractIDPreimage.type =
        stellar::ContractIDPreimageType::CONTRACT_ID_PREIMAGE_FROM_ADDRESS;
    op->m_hostFunction.createContractV2.contractIDPreimage.fromAddressAddr = toScAddressXdr(fromAddress);
    std::memcpy(op->m_hostFunction.createContractV2.contractIDPreimage.fromAddressSalt,
                salt.constData(), 32);
    op->m_hostFunction.createContractV2.executable = executable;
    op->m_hostFunction.createContractV2.constructorArgs = constructorArgs;
    return op;
}

InvokeHostFunctionOperation* InvokeHostFunctionOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
QSTELLAR_END_NS
