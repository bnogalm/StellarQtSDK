#ifndef SCVADDRESSERRORINSTANCETEST_H
#define SCVADDRESSERRORINSTANCETEST_H

#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/scval/scv.h"
#include "../../src/stellaraddress.h"
#include "../../src/strkey.h"

/** SCAddress / SCError / SCNonceKey / SCContractInstance helpers (CAP-46). */
class ScvAddressErrorInstanceTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    static stellar::SCVal roundtripXdr(const stellar::SCVal& src)
    {
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << src; }
        stellar::SCVal back;
        { QDataStream s(&bytes, QIODevice::ReadOnly); s >> back; }
        return back;
    }

    // ─── SCAddress ───────────────────────────────────────────────────

    void testAccountAddressRoundTrip()
    {
        const QString g = "GA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVSGZ";
        stellar::SCVal v = Scv::toAddress(g);
        QCOMPARE(static_cast<int>(v.type),
                 static_cast<int>(stellar::SCValType::SCV_ADDRESS));
        QCOMPARE(static_cast<int>(v.address.type),
                 static_cast<int>(stellar::SCAddressType::SC_ADDRESS_TYPE_ACCOUNT));

        StellarAddress back = Scv::fromAddress(roundtripXdr(v));
        QCOMPARE(back.toString(), g);
    }

    void testContractAddressRoundTrip()
    {
        QByteArray contractBytes(32, '\xCA');
        QString c = StrKey::encodeContract(contractBytes);

        stellar::SCVal v = Scv::toAddress(c);
        QCOMPARE(static_cast<int>(v.address.type),
                 static_cast<int>(stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT));

        StellarAddress back = Scv::fromAddress(roundtripXdr(v));
        QCOMPARE(static_cast<int>(back.getType()),
                 static_cast<int>(StellarAddress::Type::CONTRACT));
        QCOMPARE(back.toString(), c);
    }

    void testMuxedAddressRoundTrip()
    {
        const QString m = "MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK";
        stellar::SCVal v = Scv::toAddress(m);
        QCOMPARE(static_cast<int>(v.address.type),
                 static_cast<int>(stellar::SCAddressType::SC_ADDRESS_TYPE_MUXED_ACCOUNT));
        QCOMPARE(v.address.muxedId, quint64(0x8000000000000000ULL));

        StellarAddress back = Scv::fromAddress(roundtripXdr(v));
        QCOMPARE(back.toString(), m);
    }

    void testClaimableBalanceAddressRoundTrip()
    {
        QByteArray body;
        body.append(static_cast<char>(0)); // V0 discriminant
        for (int i = 0; i < 32; ++i) body.append(static_cast<char>(0x77 ^ i));
        QString b = StrKey::encodeClaimableBalance(body);

        stellar::SCVal v = Scv::toAddress(b);
        QCOMPARE(static_cast<int>(v.address.type),
                 static_cast<int>(stellar::SCAddressType::SC_ADDRESS_TYPE_CLAIMABLE_BALANCE));

        StellarAddress back = Scv::fromAddress(roundtripXdr(v));
        QCOMPARE(back.toString(), b);
    }

    void testLiquidityPoolAddressRoundTrip()
    {
        QByteArray poolId(32, '\x42');
        QString l = StrKey::encodeLiquidityPool(poolId);

        stellar::SCVal v = Scv::toAddress(l);
        QCOMPARE(static_cast<int>(v.address.type),
                 static_cast<int>(stellar::SCAddressType::SC_ADDRESS_TYPE_LIQUIDITY_POOL));

        StellarAddress back = Scv::fromAddress(roundtripXdr(v));
        QCOMPARE(back.toString(), l);
    }

    // ─── SCError ─────────────────────────────────────────────────────

    void testContractErrorRoundTrip()
    {
        stellar::SCVal v = Scv::toError(stellar::SCErrorType::SCE_CONTRACT, 42);
        stellar::SCError back = Scv::fromError(roundtripXdr(v));
        QCOMPARE(static_cast<int>(back.type),
                 static_cast<int>(stellar::SCErrorType::SCE_CONTRACT));
        QCOMPARE(back.contractCode, quint32(42));
    }

    void testEnumErrorRoundTrip()
    {
        stellar::SCVal v = Scv::toError(stellar::SCErrorType::SCE_BUDGET,
                                        static_cast<quint32>(stellar::SCErrorCode::SCEC_EXCEEDED_LIMIT));
        stellar::SCError back = Scv::fromError(roundtripXdr(v));
        QCOMPARE(static_cast<int>(back.type),
                 static_cast<int>(stellar::SCErrorType::SCE_BUDGET));
        QCOMPARE(static_cast<int>(back.code),
                 static_cast<int>(stellar::SCErrorCode::SCEC_EXCEEDED_LIMIT));
    }

    // ─── NonceKey / LedgerKeyContractInstance ────────────────────────

    void testNonceKeyRoundTrip()
    {
        stellar::SCVal v = Scv::toLedgerKeyNonce(qint64(0xDEADBEEFCAFE1234LL));
        QCOMPARE(Scv::fromLedgerKeyNonce(roundtripXdr(v)),
                 qint64(0xDEADBEEFCAFE1234LL));
    }

    void testLedgerKeyContractInstanceHasNoPayload()
    {
        stellar::SCVal v = Scv::toLedgerKeyContractInstance();
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << v; }
        // 4 bytes type discriminant, no payload.
        QCOMPARE(bytes.size(), 4);

        stellar::SCVal back = roundtripXdr(v);
        QCOMPARE(static_cast<int>(back.type),
                 static_cast<int>(stellar::SCValType::SCV_LEDGER_KEY_CONTRACT_INSTANCE));
    }

    // ─── SCContractInstance ──────────────────────────────────────────

    void testContractInstanceWasmNoStorage()
    {
        stellar::ContractExecutable exec;
        exec.type = stellar::ContractExecutableType::CONTRACT_EXECUTABLE_WASM;
        for (int i = 0; i < 32; ++i) exec.wasmHash[i] = static_cast<quint8>(i);

        stellar::SCVal v = roundtripXdr(Scv::toContractInstance(exec));
        stellar::SCContractInstance ci = Scv::fromContractInstance(v);
        QCOMPARE(static_cast<int>(ci.executable.type),
                 static_cast<int>(stellar::ContractExecutableType::CONTRACT_EXECUTABLE_WASM));
        for (int i = 0; i < 32; ++i) {
            QCOMPARE(ci.executable.wasmHash[i], static_cast<quint8>(i));
        }
        QVERIFY(ci.storage.isNull());
    }

    void testContractInstanceStellarAssetWithStorage()
    {
        stellar::ContractExecutable exec;
        exec.type = stellar::ContractExecutableType::CONTRACT_EXECUTABLE_STELLAR_ASSET;

        QList<stellar::SCVal> keys   = { Scv::toSymbol("k") };
        QList<stellar::SCVal> values = { Scv::toUint32(7) };
        stellar::SCVal storage = Scv::toMap(keys, values);

        stellar::SCVal v = roundtripXdr(Scv::toContractInstance(exec, storage));
        stellar::SCContractInstance ci = Scv::fromContractInstance(v);
        QCOMPARE(static_cast<int>(ci.executable.type),
                 static_cast<int>(stellar::ContractExecutableType::CONTRACT_EXECUTABLE_STELLAR_ASSET));
        QVERIFY(!ci.storage.isNull());
        QCOMPARE(Scv::fromMap(*ci.storage).size(), 1);
    }

    // ─── ContractExecutable EXTERNAL_REF (CAP-85, Protocol 28) ───────

    void testContractInstanceExternalRefRoundTrip()
    {
        QByteArray ownerBytes(32, '\x5A');
        stellar::ContractExecutable exec;
        exec.type = stellar::ContractExecutableType::CONTRACT_EXECUTABLE_EXTERNAL_REF;
        exec.externalRef.executableOwner =
            Scv::toAddress(StrKey::encodeContract(ownerBytes)).address;
        exec.externalRef.tag = QByteArray("shared-v1");   // 9 bytes -> 3 pad bytes

        stellar::SCContractInstance ci =
            Scv::fromContractInstance(roundtripXdr(Scv::toContractInstance(exec)));

        QCOMPARE(static_cast<int>(ci.executable.type),
                 static_cast<int>(stellar::ContractExecutableType::CONTRACT_EXECUTABLE_EXTERNAL_REF));
        QCOMPARE(static_cast<int>(ci.executable.externalRef.executableOwner.type),
                 static_cast<int>(stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT));
        for (int i = 0; i < 32; ++i) {
            QCOMPARE(ci.executable.externalRef.executableOwner.contractId[i],
                     static_cast<quint8>('\x5A'));
        }
        QCOMPARE(ci.executable.externalRef.tag, QByteArray("shared-v1"));
    }

    /** The regression that matters. Contract instances arrive as XDR produced
     *  by Core, so the reader must handle a wire-correct external-ref payload.
     *  Before CAP-85 support it consumed only the discriminant for any
     *  non-WASM executable, leaving `executable_owner` + `tag` in the stream,
     *  and every field decoded after them — here the instance storage — came
     *  out garbage. */
    void testExternalRefDoesNotDesyncTrailingStorage()
    {
        stellar::ContractExecutable exec;
        exec.type = stellar::ContractExecutableType::CONTRACT_EXECUTABLE_EXTERNAL_REF;
        exec.externalRef.executableOwner =
            Scv::toAddress(StrKey::encodeContract(QByteArray(32, '\x11'))).address;
        exec.externalRef.tag = QByteArray("tag");

        QList<stellar::SCVal> keys   = { Scv::toSymbol("k") };
        QList<stellar::SCVal> values = { Scv::toUint32(42) };
        stellar::SCVal storage = Scv::toMap(keys, values);

        stellar::SCContractInstance ci =
            Scv::fromContractInstance(roundtripXdr(Scv::toContractInstance(exec, storage)));

        QVERIFY(!ci.storage.isNull());
        QList<stellar::SCMapEntry> entries = Scv::fromMap(*ci.storage);
        QCOMPARE(entries.size(), 1);
        QCOMPARE(Scv::fromSymbol(entries.at(0).key), QString("k"));
        QCOMPARE(Scv::fromUint32(entries.at(0).val), 42u);
    }

    /** An executable arm we don't know must fail loudly rather than leave its
     *  payload in the stream. */
    void testUnknownContractExecutableTypeThrows()
    {
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << static_cast<qint32>(7); }

        stellar::ContractExecutable e;
        QDataStream s(&bytes, QIODevice::ReadOnly);
        bool threw = false;
        try { s >> e; } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
    }
};

ADD_TEST(ScvAddressErrorInstanceTest)
#endif // SCVADDRESSERRORINSTANCETEST_H
