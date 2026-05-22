#ifndef SOROBANOPSTEST_H
#define SOROBANOPSTEST_H

#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/operation.h"
#include "../../src/invokehostfunctionoperation.h"
#include "../../src/extendfootprintttloperation.h"
#include "../../src/restorefootprintoperation.h"
#include "../../src/scval/scv.h"
#include "../../src/stellaraddress.h"
#include "../../src/strkey.h"
#include "../../src/assettypenative.h"

/** Round-trip tests for the three Soroban operations (CAP-46-8). */
class SorobanOpsTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    static QString contractStrkey()
    {
        // Deterministic C-strkey: 32 fixed bytes.
        return StrKey::encodeContract(QByteArray(32, '\x01'));
    }

    // ─── ExtendFootprintTTLOp ────────────────────────────────────────

    void testExtendFootprintTtlRoundTrip()
    {
        QScopedPointer<ExtendFootprintTTLOperation> op(
            ExtendFootprintTTLOperation::create(100'000u));
        op->setSourceAccount("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");

        stellar::Operation xdr = op->toXdr();
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::OperationType::EXTEND_FOOTPRINT_TTL));
        QCOMPARE(xdr.operationExtendFootprintTtl.extendTo, quint32(100'000));

        QScopedPointer<ExtendFootprintTTLOperation> back(
            static_cast<ExtendFootprintTTLOperation*>(Operation::fromXdr(xdr)));
        QCOMPARE(back->getExtendTo(), quint32(100'000));
        QCOMPARE(back->getSourceAccount(),
                 QString("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
    }

    // ─── RestoreFootprintOp ──────────────────────────────────────────

    void testRestoreFootprintRoundTrip()
    {
        QScopedPointer<RestoreFootprintOperation> op(RestoreFootprintOperation::create());
        stellar::Operation xdr = op->toXdr();
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::OperationType::RESTORE_FOOTPRINT));

        QScopedPointer<RestoreFootprintOperation> back(
            static_cast<RestoreFootprintOperation*>(Operation::fromXdr(xdr)));
        QVERIFY(back.data() != nullptr);
    }

    // ─── InvokeHostFunctionOp: invokeContract ────────────────────────

    void testInvokeContractRoundTrip()
    {
        StellarAddress contract = StellarAddress::fromString(contractStrkey());
        QList<stellar::SCVal> args = {
            Scv::toUint32(42),
            Scv::toSymbol("ARG"),
        };

        QScopedPointer<InvokeHostFunctionOperation> op(
            InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
                contract, "transfer", args));

        stellar::Operation xdr = op->toXdr();
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::OperationType::INVOKE_HOST_FUNCTION));
        QCOMPARE(static_cast<int>(xdr.operationInvokeHostFunction.hostFunction.type),
                 static_cast<int>(stellar::HostFunctionType::HOST_FUNCTION_TYPE_INVOKE_CONTRACT));

        QScopedPointer<InvokeHostFunctionOperation> back(
            static_cast<InvokeHostFunctionOperation*>(Operation::fromXdr(xdr)));
        const auto& invoke = back->getHostFunction().invokeContract;
        QCOMPARE(invoke.functionName, QByteArray("transfer"));
        QCOMPARE(invoke.args.size(), 2);
        QCOMPARE(Scv::fromUint32(invoke.args.at(0)), quint32(42));
        QCOMPARE(Scv::fromSymbol(invoke.args.at(1)), QString("ARG"));
    }

    void testInvokeContractRejectsNonContractAddress()
    {
        StellarAddress account = StellarAddress::fromString(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        bool threw = false;
        try {
            InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
                account, "transfer", {});
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void testInvokeContractRejectsLongFunctionName()
    {
        StellarAddress contract = StellarAddress::fromString(contractStrkey());
        bool threw = false;
        try {
            InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
                contract, QString(33, 'x'), {});
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ─── InvokeHostFunctionOp: uploadWasm ────────────────────────────

    void testUploadContractWasmRoundTrip()
    {
        QByteArray wasm("\x00asm\x01\x00\x00\x00", 8);
        QScopedPointer<InvokeHostFunctionOperation> op(
            InvokeHostFunctionOperation::uploadContractWasmOperationBuilder(wasm));

        stellar::Operation xdr = op->toXdr();
        QCOMPARE(static_cast<int>(xdr.operationInvokeHostFunction.hostFunction.type),
                 static_cast<int>(stellar::HostFunctionType::HOST_FUNCTION_TYPE_UPLOAD_CONTRACT_WASM));

        QScopedPointer<InvokeHostFunctionOperation> back(
            static_cast<InvokeHostFunctionOperation*>(Operation::fromXdr(xdr)));
        QCOMPARE(back->getHostFunction().uploadWasm, wasm);
    }

    // ─── InvokeHostFunctionOp: createContract from address ───────────

    void testCreateContractFromAddressRoundTrip()
    {
        StellarAddress source = StellarAddress::fromString(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        QByteArray salt(32, '\x77');
        stellar::ContractExecutable exec;
        exec.type = stellar::ContractExecutableType::CONTRACT_EXECUTABLE_WASM;
        for (int i = 0; i < 32; ++i) exec.wasmHash[i] = static_cast<quint8>(0xAA);

        QScopedPointer<InvokeHostFunctionOperation> op(
            InvokeHostFunctionOperation::createContractOperationBuilder(source, salt, exec));

        stellar::Operation xdr = op->toXdr();
        QCOMPARE(static_cast<int>(xdr.operationInvokeHostFunction.hostFunction.type),
                 static_cast<int>(stellar::HostFunctionType::HOST_FUNCTION_TYPE_CREATE_CONTRACT));

        QScopedPointer<InvokeHostFunctionOperation> back(
            static_cast<InvokeHostFunctionOperation*>(Operation::fromXdr(xdr)));
        const auto& create = back->getHostFunction().createContract;
        QCOMPARE(static_cast<int>(create.contractIDPreimage.type),
                 static_cast<int>(stellar::ContractIDPreimageType::CONTRACT_ID_PREIMAGE_FROM_ADDRESS));
        for (int i = 0; i < 32; ++i) {
            QCOMPARE(create.contractIDPreimage.fromAddressSalt[i], quint8(0x77));
        }
    }

    // ─── InvokeHostFunctionOp: createContract from asset (SAC) ───────

    void testCreateContractFromAssetRoundTrip()
    {
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;

        QScopedPointer<InvokeHostFunctionOperation> op(
            InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(native));

        stellar::Operation xdr = op->toXdr();
        const auto& create = xdr.operationInvokeHostFunction.hostFunction.createContract;
        QCOMPARE(static_cast<int>(create.contractIDPreimage.type),
                 static_cast<int>(stellar::ContractIDPreimageType::CONTRACT_ID_PREIMAGE_FROM_ASSET));
        QCOMPARE(static_cast<int>(create.executable.type),
                 static_cast<int>(stellar::ContractExecutableType::CONTRACT_EXECUTABLE_STELLAR_ASSET));

        QScopedPointer<InvokeHostFunctionOperation> back(
            static_cast<InvokeHostFunctionOperation*>(Operation::fromXdr(xdr)));
        QCOMPARE(static_cast<int>(back->getHostFunction().createContract.contractIDPreimage.fromAssetAsset.type),
                 static_cast<int>(stellar::AssetType::ASSET_TYPE_NATIVE));
    }

    // ─── InvokeHostFunctionOp: createContractV2 ──────────────────────

    void testCreateContractV2RoundTrip()
    {
        StellarAddress source = StellarAddress::fromString(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        QByteArray salt(32, '\x55');
        stellar::ContractExecutable exec;
        exec.type = stellar::ContractExecutableType::CONTRACT_EXECUTABLE_WASM;
        QList<stellar::SCVal> ctorArgs = { Scv::toUint32(1), Scv::toSymbol("init") };

        QScopedPointer<InvokeHostFunctionOperation> op(
            InvokeHostFunctionOperation::createContractV2OperationBuilder(
                source, salt, exec, ctorArgs));

        stellar::Operation xdr = op->toXdr();
        QCOMPARE(static_cast<int>(xdr.operationInvokeHostFunction.hostFunction.type),
                 static_cast<int>(stellar::HostFunctionType::HOST_FUNCTION_TYPE_CREATE_CONTRACT_V2));

        QScopedPointer<InvokeHostFunctionOperation> back(
            static_cast<InvokeHostFunctionOperation*>(Operation::fromXdr(xdr)));
        QCOMPARE(back->getHostFunction().createContractV2.constructorArgs.size(), 2);
    }
};

ADD_TEST(SorobanOpsTest)
#endif // SOROBANOPSTEST_H
