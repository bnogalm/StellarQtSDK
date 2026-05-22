#ifndef INVOKEHOSTFUNCTIONOPERATION_H
#define INVOKEHOSTFUNCTIONOPERATION_H

#include <QList>

#include "operation.h"
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

    /** Invokes `functionName` on `contractAddress` with `args`. */
    static InvokeHostFunctionOperation* invokeContractFunctionOperationBuilder(
        const StellarAddress& contractAddress,
        const QString& functionName,
        const QList<stellar::SCVal>& args);

    /** Uploads contract wasm bytes. */
    static InvokeHostFunctionOperation* uploadContractWasmOperationBuilder(
        const QByteArray& wasm);

    /** Creates a contract instance from address+salt (no constructor args). */
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

#endif // INVOKEHOSTFUNCTIONOPERATION_H
