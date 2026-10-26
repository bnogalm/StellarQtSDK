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
#include "../../src/account.h"
#include "../../src/network.h"
#include "../../src/transaction.h"
#include "../../src/transactionbuilder.h"
#include "../../src/contract/sorobancredentialssigner.h"
#include "../../src/keypair.h"

/** Round-trip tests for the three Soroban operations (CAP-46-8). */
class SorobanOpsTest: public QObject
{
    Q_OBJECT

    static QByteArray be32(qint32 n)
    {
        QByteArray out;
        QDataStream s(&out, QIODevice::WriteOnly);
        s << n;
        return out;
    }

    /** InvokeContractArgs on contract 01..01, function "f", `argCount` then `args`. */
    static QByteArray contractFn(const QByteArray& args, qint32 argCount)
    {
        return be32(1) + QByteArray(32, '\x01')        // SCAddress CONTRACT
             + be32(1) + QByteArray("f\0\0\0", 4)      // functionName, padded
             + be32(argCount) + args;
    }

    /** `depth` nested SorobanAuthorizedInvocations, as bytes, the innermost
     *  calling f with `innermostArg` if one is given. */
    static QByteArray invocation(int depth, const QByteArray& innermostArg = QByteArray())
    {
        QByteArray out;
        for (int i = 1; i < depth; ++i)
            out += be32(0) + contractFn(QByteArray(), 0) + be32(1);   // CONTRACT_FN, one sub-invocation
        return out + be32(0) + contractFn(innermostArg, innermostArg.isEmpty() ? 0 : 1) + be32(0);
    }

    /** `depth` nested one-element vectors around a u32, as bytes. */
    static QByteArray nestedVec(int depth)
    {
        QByteArray out;
        for (int i = 1; i < depth; ++i) out += be32(16) + be32(1) + be32(1);
        return out + be32(3) + be32(7);
    }

    /** Decodes all of `bytes` as one T: "ok", or why not. */
    template <typename T>
    static QString decodeAs(const QByteArray& bytes)
    {
        T value;
        QDataStream in(bytes);
        try { in >> value; }
        catch (const std::runtime_error& e) { return QString::fromUtf8(e.what()); }
        catch (const std::exception& e) { return QStringLiteral("not a runtime_error: ") + QString::fromUtf8(e.what()); }
        if (in.status() != QDataStream::Ok) return QStringLiteral("stream status %1").arg(int(in.status()));
        return in.atEnd() ? QStringLiteral("ok") : QStringLiteral("trailing bytes");
    }

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

    /** createContractV2 with six constructor args (u32, symbol, account
     *  address, bytes, string, i128 -5), deployer GC5SIC4E..., salt 0x55*32,
     *  wasm 0xAA*32, no source. Expected op XDR from js-stellar-sdk 17.1.0
     *  Operation.createCustomContract; py-stellar-sdk 14.1.0
     *  TransactionBuilder.append_create_contract_op gives the same bytes. */
    void testCreateContractV2WithConstructorArgsMatchesJs()
    {
        const QString deployer("GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF");
        stellar::ContractExecutable exec;
        exec.type = stellar::ContractExecutableType::CONTRACT_EXECUTABLE_WASM;
        for (int i = 0; i < 32; ++i) exec.wasmHash[i] = static_cast<quint8>(0xAA);
        const QList<stellar::SCVal> ctorArgs = {
            Scv::toUint32(1u), Scv::toSymbol(QStringLiteral("init")),
            Scv::toAddress(deployer), Scv::toBytes(QByteArray("\x01\x02\x03", 3)),
            Scv::toString(QStringLiteral("hello")), Scv::toInt128(qint64(-5)) };

        QScopedPointer<InvokeHostFunctionOperation> op(
            InvokeHostFunctionOperation::createContractV2OperationBuilder(
                StellarAddress::fromString(deployer), QByteArray(32, '\x55'), exec, ctorArgs));
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << op->toXdr(); }
        QCOMPARE(bytes.toBase64(), QByteArray(
            "AAAAAAAAABgAAAADAAAAAAAAAAAAAAAAuyQLhN176rjp27IL9kkc7Y7pOEaAVXn0NK5axTDEL3xVVVVV"
            "VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVQAAAACqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq"
            "qqqqqgAAAAYAAAADAAAAAQAAAA8AAAAEaW5pdAAAABIAAAAAAAAAALskC4Tde+q46duyC/ZJHO2O6ThG"
            "gFV59DSuWsUwxC98AAAADQAAAAMBAgMAAAAADgAAAAVoZWxsbwAAAAAAAAr////////////////////7"
            "AAAAAA=="));
    }

    /** createContractV2 with no constructor args and an op source still writes
     *  HOST_FUNCTION_TYPE_CREATE_CONTRACT_V2 with an empty args vector. Expected
     *  op XDR from js-stellar-sdk 17.1.0 Operation.createCustomContract
     *  ({..., source: GCACCFMI...}); py-stellar-sdk 14.1.0
     *  append_create_contract_op(..., source=GCACCFMI...) gives the same bytes. */
    void testCreateContractV2EmptyArgsWithSourceMatchesJs()
    {
        stellar::ContractExecutable exec;
        exec.type = stellar::ContractExecutableType::CONTRACT_EXECUTABLE_WASM;
        for (int i = 0; i < 32; ++i) exec.wasmHash[i] = static_cast<quint8>(0xAA);
        QScopedPointer<InvokeHostFunctionOperation> op(
            InvokeHostFunctionOperation::createContractV2OperationBuilder(
                StellarAddress::fromString(
                    QStringLiteral("GC5SIC4E3V56VOHJ3OZAX5SJDTWY52JYI2AFK6PUGSXFVRJQYQXXZBZF")),
                QByteArray(32, '\x55'), exec, QList<stellar::SCVal>()));
        op->setSourceAccount(QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << op->toXdr(); }
        QCOMPARE(bytes.toBase64(), QByteArray(
            "AAAAAQAAAACAIRWIskB6UBKRNWF+vq1GvEVyTjOxpJcRu62WR5tQjwAAABgAAAADAAAAAAAAAAAAAAAA"
            "uyQLhN176rjp27IL9kkc7Y7pOEaAVXn0NK5axTDEL3xVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV"
            "VVVVVQAAAACqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqgAAAAAAAAAA"));
    }

    // ─── CAP-71 / Protocol 27 credential arms ────────────────────────

    static stellar::SorobanAddressCredentials sampleAddressCredentials()
    {
        stellar::SorobanAddressCredentials c;
        c.address.type = stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT;
        for (int i = 0; i < 32; ++i) c.address.contractId[i] = static_cast<quint8>(i);
        c.nonce = Q_INT64_C(0x0102030405060708);
        c.signatureExpirationLedger = 987654;
        c.signature = Scv::toUint32(7);
        return c;
    }

    static stellar::SorobanCredentials roundtrip(const stellar::SorobanCredentials& src)
    {
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << src; }
        stellar::SorobanCredentials back;
        { QDataStream s(&bytes, QIODevice::ReadOnly); s >> back; }
        return back;
    }

    /** ADDRESS_V2 carries the same payload as ADDRESS — only the discriminant
     *  and the signature preimage differ — so it must encode to the ADDRESS
     *  bytes with a different tag, and must not be silently read as ADDRESS. */
    void testSorobanCredentialsAddressV2RoundTrip()
    {
        stellar::SorobanCredentials c;
        c.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2;
        c.address = sampleAddressCredentials();

        stellar::SorobanCredentials back = roundtrip(c);
        QCOMPARE(static_cast<int>(back.type),
                 static_cast<int>(stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2));
        QCOMPARE(back.address.nonce, Q_INT64_C(0x0102030405060708));
        QCOMPARE(back.address.signatureExpirationLedger, quint32(987654));
        QCOMPARE(static_cast<int>(back.address.address.type),
                 static_cast<int>(stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT));
    }

    /** Delegates are self-recursive in the XDR, so exercise a nested chain. */
    void testSorobanCredentialsWithDelegatesRoundTrip()
    {
        stellar::SorobanDelegateSignature nested;
        nested.address.type = stellar::SCAddressType::SC_ADDRESS_TYPE_ACCOUNT;
        for (int i = 0; i < 32; ++i) nested.address.accountId[i] = static_cast<quint8>(0xAA);
        nested.signature = Scv::toUint32(2);

        stellar::SorobanDelegateSignature top;
        top.address.type = stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT;
        for (int i = 0; i < 32; ++i) top.address.contractId[i] = static_cast<quint8>(0xBB);
        top.signature = Scv::toUint32(1);
        top.nestedDelegates = QSharedPointer<QList<stellar::SorobanDelegateSignature>>::create();
        top.nestedDelegates->append(nested);

        stellar::SorobanCredentials c;
        c.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_WITH_DELEGATES;
        c.addressWithDelegates.addressCredentials = sampleAddressCredentials();
        c.addressWithDelegates.delegates.append(top);

        stellar::SorobanCredentials back = roundtrip(c);
        QCOMPARE(static_cast<int>(back.type),
                 static_cast<int>(stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_WITH_DELEGATES));
        QCOMPARE(back.addressWithDelegates.addressCredentials.nonce, Q_INT64_C(0x0102030405060708));
        QCOMPARE(back.addressWithDelegates.delegates.size(), 1);

        const stellar::SorobanDelegateSignature& d = back.addressWithDelegates.delegates.at(0);
        QCOMPARE(Scv::fromUint32(d.signature), 1u);
        QVERIFY(!d.nestedDelegates.isNull());
        QCOMPARE(d.nestedDelegates->size(), 1);
        QCOMPARE(Scv::fromUint32(d.nestedDelegates->at(0).signature), 2u);
        QCOMPARE(static_cast<int>(d.nestedDelegates->at(0).address.type),
                 static_cast<int>(stellar::SCAddressType::SC_ADDRESS_TYPE_ACCOUNT));
    }

    /** The same unsigned auth entry (signer GC5SIC4E..., nonce
     *  0x0102030405060708, expiration 0, signature void, root invocation
     *  C(01*32).transfer(u32 7, signer)) with the credentials tag `tag`. */
    static stellar::SorobanAuthorizationEntry jsAuthEntry(char tag)
    {
        QByteArray b = QByteArray::fromBase64(
            "AAAAAQAAAAAAAAAAuyQLhN176rjp27IL9kkc7Y7pOEaAVXn0NK5axTDEL3wBAgMEBQYHCAAAAAAAAAAB"
            "AAAAAAAAAAEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQAAAAh0cmFuc2ZlcgAAAAIAAAAD"
            "AAAABwAAABIAAAAAAAAAALskC4Tde+q46duyC/ZJHO2O6ThGgFV59DSuWsUwxC98AAAAAA==");
        b[3] = tag;
        stellar::SorobanAuthorizationEntry e;
        QDataStream s(b);
        s >> e;
        return e;
    }

    static QByteArray authEntryBase64(const stellar::SorobanAuthorizationEntry& e)
    {
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << e; }
        return bytes.toBase64();
    }

    /** ADDRESS (CAP-46) authorizeEntry for ledger 123456 on testnet with seed
     *  SC4CGETA...: payload hash and the whole signed entry. Expected values
     *  from js-stellar-sdk 17.1.0 buildAuthorizationEntryPreimage /
     *  authorizeEntry; py-stellar-sdk 14.1.0 auth.authorize_entry gives the
     *  same signed entry. */
    void testAuthorizeEntryAddressMatchesJsAndPy()
    {
        QScopedPointer<KeyPair> kp(KeyPair::fromSecretSeed(
            QStringLiteral("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK")));
        const QString passphrase = QStringLiteral("Test SDF Network ; September 2015");
        stellar::SorobanAuthorizationEntry entry = jsAuthEntry('\x01');
        QCOMPARE(static_cast<int>(entry.credentials.type),
                 static_cast<int>(stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS));

        stellar::SorobanAuthorizationEntry stamped = entry;
        stamped.credentials.address.signatureExpirationLedger = 123456;
        QCOMPARE(SorobanCredentialsSigner::hashedSignaturePayload(stamped, 123456, passphrase).toHex(),
                 QByteArray("e502a1a594dae14879f92d98121bbe8b7f322e608463097710e382283692f33e"));

        QCOMPARE(authEntryBase64(SorobanCredentialsSigner::authorizeEntry(
                     entry, kp.data(), 123456, passphrase)), QByteArray(
            "AAAAAQAAAAAAAAAAuyQLhN176rjp27IL9kkc7Y7pOEaAVXn0NK5axTDEL3wBAgMEBQYHCAAB4kAAAAAQ"
            "AAAAAQAAAAEAAAARAAAAAQAAAAIAAAAPAAAACnB1YmxpY19rZXkAAAAAAA0AAAAguyQLhN176rjp27IL"
            "9kkc7Y7pOEaAVXn0NK5axTDEL3wAAAAPAAAACXNpZ25hdHVyZQAAAAAAAA0AAABAs9iJus74RnZ2T+Xa"
            "XCYbnqIR1Yz6UthsXI2tvP4w+E+cT0RIyEeqwxelhmzvKW8TFfijDmtugu5gBSZ1TAyQCQAAAAAAAAAB"
            "AQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEAAAAIdHJhbnNmZXIAAAACAAAAAwAAAAcAAAAS"
            "AAAAAAAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAA="));
    }

    /** ADDRESS_V2 (CAP-71) authorizeEntry: the preimage is
     *  ENVELOPE_TYPE_SOROBAN_AUTHORIZATION_WITH_ADDRESS, so the payload and
     *  the signature differ from the ADDRESS case for the same entry. Expected
     *  payload and signed entry from js-stellar-sdk 17.1.0
     *  buildAuthorizationEntryPreimage / authorizeEntry (py-stellar-sdk 14.1.0
     *  has no ADDRESS_V2 arm). */
    void testAuthorizeEntryAddressV2MatchesJs()
    {
        QScopedPointer<KeyPair> kp(KeyPair::fromSecretSeed(
            QStringLiteral("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK")));
        const QString passphrase = QStringLiteral("Test SDF Network ; September 2015");
        stellar::SorobanAuthorizationEntry entry = jsAuthEntry('\x02');
        QCOMPARE(static_cast<int>(entry.credentials.type),
                 static_cast<int>(stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2));

        stellar::SorobanAuthorizationEntry stamped = entry;
        stamped.credentials.address.signatureExpirationLedger = 123456;
        QCOMPARE(SorobanCredentialsSigner::hashedSignaturePayload(stamped, 123456, passphrase).toHex(),
                 QByteArray("6050ababa96b5d3627609a6740d4efafa5253b8c4885c7e86c0cceddfe9fa2c0"));

        QCOMPARE(authEntryBase64(SorobanCredentialsSigner::authorizeEntry(
                     entry, kp.data(), 123456, passphrase)), QByteArray(
            "AAAAAgAAAAAAAAAAuyQLhN176rjp27IL9kkc7Y7pOEaAVXn0NK5axTDEL3wBAgMEBQYHCAAB4kAAAAAQ"
            "AAAAAQAAAAEAAAARAAAAAQAAAAIAAAAPAAAACnB1YmxpY19rZXkAAAAAAA0AAAAguyQLhN176rjp27IL"
            "9kkc7Y7pOEaAVXn0NK5axTDEL3wAAAAPAAAACXNpZ25hdHVyZQAAAAAAAA0AAABAOcqI5+6MjSeomqby"
            "lMhgdwlVCTbO/gg+YPVSowqzyDxD6xjuBkK6F5pwE8CKpaJeFoXee4eRy55s/0zUi8UuBAAAAAAAAAAB"
            "AQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEAAAAIdHJhbnNmZXIAAAACAAAAAwAAAAcAAAAS"
            "AAAAAAAAAAC7JAuE3XvquOnbsgv2SRztjuk4RoBVefQ0rlrFMMQvfAAAAAA="));
    }

    /** An unknown credentials arm must throw, not consume the discriminant
     *  alone and desync the rest of the auth entry. */
    void testUnknownSorobanCredentialsTypeThrows()
    {
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << static_cast<qint32>(9); }
        stellar::SorobanCredentials c;
        QDataStream s(&bytes, QIODevice::ReadOnly);
        bool threw = false;
        try { s >> c; } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
    }

    /** The function name is an SCSymbol and is checked as Scv::toSymbol checks
     *  one. "trans-fer", "héllo" or "a b" used to be built and signed, and then
     *  refused by the host. */
    void testInvokeContractFunctionNameIsASymbol()
    {
        const StellarAddress contract = StellarAddress::fromString(contractStrkey());
        auto builds = [&](const QString& name) {
            try {
                QScopedPointer<InvokeHostFunctionOperation> op(
                    InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(contract, name, {}));
                return true;
            } catch (const std::runtime_error&) {
                return false;
            }
        };
        QVERIFY(!builds(QStringLiteral("trans-fer")));
        QVERIFY(!builds(QString::fromUtf8("h\xC3\xA9llo")));
        QVERIFY(!builds(QStringLiteral("a b")));
        QVERIFY(!builds(QStringLiteral("bad name!")));
        QVERIFY(!builds(QStringLiteral("hello.world")));
        QVERIFY(!builds(QStringLiteral(" transfer")));
        QVERIFY(!builds(QStringLiteral("transfer\n")));
        QVERIFY(!builds(QStringLiteral("a") + QChar(0) + QStringLiteral("b")));
        QVERIFY(!builds(QString()));
        QVERIFY(!builds(QString(33, QLatin1Char('x'))));
        QVERIFY(builds(QStringLiteral("transfer_1")));
        QVERIFY(builds(QStringLiteral("f")));
        QVERIFY(builds(QString(32, QLatin1Char('x'))));
    }

    /**
     * subInvocations nest like SCVal and are depth-limited the same way:
     * ~1000 nested levels of a hostile auth entry (80 KB) overflowed the
     * stack. The limit decodes even with an argument nested to the SCVal
     * limit at the bottom — the worst the two allow — in this debug build.
     */
    void testInvocationNestingIsLimited()
    {
        using Invocation = stellar::SorobanAuthorizedInvocation;
        const int max = stellar::MAX_SOROBAN_INVOCATION_DEPTH;
        const QString tooDeep = QStringLiteral("SorobanAuthorizedInvocation: nesting too deep");
        QCOMPARE(max, 128);
        QCOMPARE(decodeAs<Invocation>(invocation(max)), QStringLiteral("ok"));
        QCOMPARE(decodeAs<Invocation>(invocation(max + 1)), tooDeep);
        QCOMPARE(decodeAs<Invocation>(invocation(5000)), tooDeep);
        QCOMPARE(decodeAs<Invocation>(invocation(max, nestedVec(stellar::MAX_SCVAL_DEPTH))),
                 QStringLiteral("ok"));
        QCOMPARE(decodeAs<Invocation>(invocation(max, nestedVec(stellar::MAX_SCVAL_DEPTH + 1))),
                 QStringLiteral("SCVal: nesting too deep"));
        QCOMPARE(stellar::sorobanInvocationDepth(), 0);
        QCOMPARE(stellar::scValDepth(), 0);
    }

    /**
     * Counts are checked against the bytes left before anything is allocated
     * for them: an args count of 4,194,304 in a 124-byte envelope committed
     * 2 GB, and 1,048,576 auth entries 2.8 GB, before the read ran dry. A
     * loop also stops at the first element that runs dry.
     */
    void testSorobanListCountsAreBoundedByTheInput()
    {
        const QString tooLong = QStringLiteral(": length exceeds the remaining input");
        QCOMPARE(decodeAs<stellar::InvokeContractArgs>(contractFn(QByteArray(), 100000)),
                 QStringLiteral("invokeContract args") + tooLong);
        // FROM_ASSET native, a STELLAR_ASSET executable, then the constructor args.
        QCOMPARE(decodeAs<stellar::CreateContractArgsV2>(be32(1) + be32(0) + be32(1) + be32(100000)),
                 QStringLiteral("constructorArgs") + tooLong);
        QCOMPARE(decodeAs<stellar::SorobanAuthorizedInvocation>(
                     be32(0) + contractFn(QByteArray(), 0) + be32(100000)),
                 QStringLiteral("subInvocations") + tooLong);
        // An upload of no bytes, then the auth entries.
        QCOMPARE(decodeAs<stellar::InvokeHostFunctionOp>(be32(2) + be32(0) + be32(100000)),
                 QStringLiteral("auth array") + tooLong);
        QCOMPARE(decodeAs<stellar::HostFunction>(be32(2) + be32(100000000)),
                 QStringLiteral("uploadWasm") + tooLong);
        // Two args claimed, eight bytes left: enough by count, but the first
        // (a u64) runs dry.
        QCOMPARE(decodeAs<stellar::InvokeContractArgs>(contractFn(be32(5) + be32(0), 2)),
                 QStringLiteral("invokeContract args: truncated or malformed element"));
        QCOMPARE(decodeAs<stellar::InvokeContractArgs>(contractFn(be32(3) + be32(7) + be32(1), 2)),
                 QStringLiteral("ok"));
    }

    /** An arm of HostFunction, ContractIDPreimage or SorobanAuthorizedFunction
     *  that does not exist was read as nothing, and its payload then as the
     *  fields after it. The stream is marked corrupt instead. */
    void testUnknownSorobanArmsMarkTheStreamCorrupt()
    {
        const QByteArray payload(64, '\x11');
        const QString corrupt = QStringLiteral("stream status %1").arg(int(QDataStream::ReadCorruptData));
        QCOMPARE(decodeAs<stellar::HostFunction>(be32(4) + payload), corrupt);
        QCOMPARE(decodeAs<stellar::ContractIDPreimage>(be32(2) + payload), corrupt);
        QCOMPARE(decodeAs<stellar::SorobanAuthorizedFunction>(be32(3) + payload), corrupt);
    }

    /**
     * And the writers: an arm of HostFunction, ContractIDPreimage or
     * SorobanAuthorizedFunction that does not exist used to go out as a bare
     * discriminant — into a signed InvokeHostFunction, an authorization
     * preimage, a contract id. Nothing is written and the stream is marked
     * failed, so the envelope that would carry it is never produced.
     */
    void testUnknownSorobanArmsAreNotWritten()
    {
        auto written = [](const auto& value) {
            QByteArray bytes;
            QDataStream out(&bytes, QIODevice::WriteOnly);
            out << value;
            return qMakePair(bytes, out.status());
        };
        stellar::HostFunction hostFunction;
        hostFunction.type = static_cast<stellar::HostFunctionType>(4);
        stellar::ContractIDPreimage preimage;
        preimage.type = static_cast<stellar::ContractIDPreimageType>(2);
        stellar::SorobanAuthorizedFunction authorized;
        authorized.type = static_cast<stellar::SorobanAuthorizedFunctionType>(3);

        const QList<QPair<QByteArray, QDataStream::Status>> results = {
            written(hostFunction), written(preimage), written(authorized) };
        for (const auto& result : results) {
            QVERIFY(result.first.isEmpty());
            QCOMPARE(result.second, QDataStream::WriteFailed);
        }

        // Nested, the whole encoding fails, not only the arm.
        stellar::CreateContractArgs create;
        create.contractIDPreimage = preimage;
        QCOMPARE(written(create).second, QDataStream::WriteFailed);
        bool threw = false;
        try { xdr::encodeOrThrow(create, "the arguments"); }
        catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);

        // In an operation of a transaction, there is no envelope to sign.
        stellar::InvokeHostFunctionOp op;
        op.hostFunction = hostFunction;
        Account account(QStringLiteral("GD65DGZZEC5ADUA5QVCA2OA6P5DKDDKOB2FMEMHCNJPCPT6XN3HQQYUG"), 1);
        TransactionBuilder builder(&account, Network::testnetNetwork());
        builder.addOperation(InvokeHostFunctionOperation::build(op));
        builder.setBaseFee(100);
        builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        threw = false;
        try {
            QScopedPointer<Transaction> tx(builder.build());
            (void)tx->toEnvelopeXdrBase64();
        } catch (const std::runtime_error&) {
            threw = true;
        }
        QVERIFY(threw);
    }
};

ADD_TEST(SorobanOpsTest)
#endif // SOROBANOPSTEST_H
