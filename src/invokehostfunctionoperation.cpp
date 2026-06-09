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

// ─── Factories ──────────────────────────────────────────────────────

InvokeHostFunctionOperation* InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
    const StellarAddress& contractAddress,
    const QString& functionName,
    const QList<stellar::SCVal>& args)
{
    if (contractAddress.getType() != StellarAddress::Type::CONTRACT) {
        throw std::runtime_error("invokeContract: address must be a contract (C-strkey)");
    }
    QByteArray fnBytes = functionName.toUtf8();
    if (fnBytes.size() > 32) {
        throw std::runtime_error("functionName must be ≤ 32 bytes");
    }

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
