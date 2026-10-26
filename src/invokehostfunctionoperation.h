#ifndef INVOKEHOSTFUNCTIONOPERATION_H
#define INVOKEHOSTFUNCTIONOPERATION_H

#include <QList>

#include "operation.h"
#include "qstellar_namespace.h"
#include "xdr/sorobanops.h"
#include "stellaraddress.h"

/**
 * CAP-46-8 — invokes a Soroban host function. Four flavours, exposed via
 * static factory methods that mirror the Java SDK naming:
 *
 *   InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(...)
 *   InvokeHostFunctionOperation::uploadContractWasmOperationBuilder(wasm)
 *   InvokeHostFunctionOperation::createContractOperationBuilder(...)
 *   InvokeHostFunctionOperation::createContractV2OperationBuilder(...)
 *
 * Auth entries can be appended after construction. For caller-equals-invoker
 * invocations, leave the auth list empty (Stellar core implicitly applies a
 * SOURCE_ACCOUNT credentials entry).
 */
QSTELLAR_BEGIN_NS

class InvokeHostFunctionOperation : public Operation
{
    stellar::HostFunction m_hostFunction;
    QList<stellar::SorobanAuthorizationEntry> m_auth;

    InvokeHostFunctionOperation() = default;

public:
    explicit InvokeHostFunctionOperation(const stellar::InvokeHostFunctionOp& op);
    ~InvokeHostFunctionOperation() override = default;

    const stellar::HostFunction& getHostFunction() const { return m_hostFunction; }
    const QList<stellar::SorobanAuthorizationEntry>& getAuth() const { return m_auth; }

    InvokeHostFunctionOperation* setAuth(const QList<stellar::SorobanAuthorizationEntry>& auth);
    InvokeHostFunctionOperation* addAuth(const stellar::SorobanAuthorizationEntry& entry);

    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation& operation) override;

    static InvokeHostFunctionOperation* build(const stellar::InvokeHostFunctionOp& op);

    // ─── Factories (Java-aligned) ────────────────────────────────────

    /** Invokes `functionName` on `contractAddress` with `args`. The name must
     *  be a symbol the host accepts (1 to 32 of [A-Za-z0-9_]), or this throws. */
    static InvokeHostFunctionOperation* invokeContractFunctionOperationBuilder(
        const StellarAddress& contractAddress,
        const QString& functionName,
        const QList<stellar::SCVal>& args);

    /** Uploads contract wasm bytes. */
    static InvokeHostFunctionOperation* uploadContractWasmOperationBuilder(
        const QByteArray& wasm);

    /**
     * Creates a contract instance from address+salt with the v1 host function
     * (HOST_FUNCTION_TYPE_CREATE_CONTRACT, no constructor). js and py never
     * sign it for a wasm deploy: they sign createContractV2OperationBuilder
     * with an empty argument list, which is another host function, hash and
     * auth tree. Use that one to match them; this is kept for 1.0.x callers.
     */
    static InvokeHostFunctionOperation* createContractOperationBuilder(
        const StellarAddress& fromAddress,
        const QByteArray& salt,
        const stellar::ContractExecutable& executable);

    /** Creates a contract instance from Stellar Asset Contract (SAC) preimage. */
    static InvokeHostFunctionOperation* createContractFromAssetOperationBuilder(
        const stellar::Asset& asset);

    /** Creates a contract instance with constructor args (Protocol 22). */
    static InvokeHostFunctionOperation* createContractV2OperationBuilder(
        const StellarAddress& fromAddress,
        const QByteArray& salt,
        const stellar::ContractExecutable& executable,
        const QList<stellar::SCVal>& constructorArgs);

    InvokeHostFunctionOperation* setSourceAccount(QString sourceAccount);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(InvokeHostFunctionOperation)

#endif // INVOKEHOSTFUNCTIONOPERATION_H
