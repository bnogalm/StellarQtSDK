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

    /** The muxed arm is `struct MuxedEd25519Account { uint64 id; uint256
     *  ed25519; }` — id FIRST. The SDK used to emit the SEP-23 strkey order
     *  (ed25519 then id); writer and reader were reversed together, so
     *  testMuxedAddressRoundTrip above passed with the bytes wrong. Decode a
     *  stream assembled by hand in canonical order: that cannot be satisfied by
     *  a self-consistent mistake. */
    void testMuxedAddressFieldOrderMatchesCanonicalXdr()
    {
        const quint64 id = Q_UINT64_C(0x0102030405060708);
        QByteArray canonical;
        {
            QDataStream s(&canonical, QIODevice::WriteOnly);
            s << static_cast<qint32>(stellar::SCValType::SCV_ADDRESS);
            s << static_cast<qint32>(stellar::SCAddressType::SC_ADDRESS_TYPE_MUXED_ACCOUNT);
            s << id;                                   // uint64 id comes first
            for (int i = 0; i < 32; ++i) s << static_cast<quint8>(0xAB);
        }
        QCOMPARE(canonical.size(), 48);   // 4 + 4 + 8 + 32

        stellar::SCVal v;
        { QDataStream s(&canonical, QIODevice::ReadOnly); s >> v; }
        QCOMPARE(static_cast<int>(v.address.type),
                 static_cast<int>(stellar::SCAddressType::SC_ADDRESS_TYPE_MUXED_ACCOUNT));
        QCOMPARE(v.address.muxedId, id);
        for (int i = 0; i < 32; ++i) {
            QCOMPARE(v.address.muxedEd25519[i], static_cast<quint8>(0xAB));
        }

        QByteArray reencoded;
        { QDataStream s(&reencoded, QIODevice::WriteOnly); s << v; }
        QCOMPARE(reencoded, canonical);
    }

    /** SCV_VEC and SCV_MAP are XDR OPTIONALS in the canonical union
     *  (`SCVec *vec;` / `SCMap *map;`), so each carries a 4-byte present flag
     *  before the array. The SDK wrote only the length, leaving every container
     *  4 bytes short of Core — invisible to a symmetric round trip, which is
     *  why the whole suite passed either way. */
    void testVecAndMapCarryTheXdrOptionalPresentFlag()
    {
        QByteArray vecBytes;
        {
            QDataStream s(&vecBytes, QIODevice::WriteOnly);
            s << static_cast<qint32>(stellar::SCValType::SCV_VEC);
            s << static_cast<qint32>(1);     // present
            s << static_cast<qint32>(1);     // length
            s << Scv::toUint32(42);
        }
        QCOMPARE(vecBytes.size(), 20);       // 16 under the old, wrong shape

        stellar::SCVal v;
        { QDataStream s(&vecBytes, QIODevice::ReadOnly); s >> v; }
        QCOMPARE(Scv::fromVec(v).size(), 1);
        QCOMPARE(Scv::fromUint32(Scv::fromVec(v).at(0)), 42u);
        QByteArray reVec;
        { QDataStream s(&reVec, QIODevice::WriteOnly); s << v; }
        QCOMPARE(reVec, vecBytes);

        QByteArray mapBytes;
        {
            QDataStream s(&mapBytes, QIODevice::WriteOnly);
            s << static_cast<qint32>(stellar::SCValType::SCV_MAP);
            s << static_cast<qint32>(1);     // present
            s << static_cast<qint32>(1);     // length
            s << Scv::toSymbol("k");
            s << Scv::toUint32(7);
        }
        QCOMPARE(mapBytes.size(), 32);       // 28 under the old, wrong shape

        stellar::SCVal m;
        { QDataStream s(&mapBytes, QIODevice::ReadOnly); s >> m; }
        QCOMPARE(Scv::fromMap(m).size(), 1);
        QByteArray reMap;
        { QDataStream s(&reMap, QIODevice::WriteOnly); s << m; }
        QCOMPARE(reMap, mapBytes);
    }

    /** An absent optional (present = 0) is distinct from an empty one. */
    void testAbsentVecIsNotAnEmptyVec()
    {
        QByteArray absent;
        {
            QDataStream s(&absent, QIODevice::WriteOnly);
            s << static_cast<qint32>(stellar::SCValType::SCV_VEC);
            s << static_cast<qint32>(0);     // absent
        }
        QCOMPARE(absent.size(), 8);

        stellar::SCVal v;
        { QDataStream s(&absent, QIODevice::ReadOnly); s >> v; }
        QVERIFY(v.vec.isNull());
        QByteArray re;
        { QDataStream s(&re, QIODevice::WriteOnly); s << v; }
        QCOMPARE(re, absent);

        // ...whereas an allocated empty list is present with zero elements.
        stellar::SCVal empty = Scv::toVec(QList<stellar::SCVal>{});
        QByteArray emptyBytes;
        { QDataStream s(&emptyBytes, QIODevice::WriteOnly); s << empty; }
        QCOMPARE(emptyBytes.size(), 12);     // type + present + length
    }

    /** SCV_EXECUTABLE_TAG = 22 is CAP-85's companion value; it carries an
     *  SCString, so it uses the SCV_STRING wire form. */
    void testExecutableTagRoundTrip()
    {
        stellar::SCVal v;
        v.type = stellar::SCValType::SCV_EXECUTABLE_TAG;
        v.executableTag = QByteArray("shared-v1");     // 9 bytes -> 3 pad

        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << v; }
        QCOMPARE(bytes.size(), 20);          // type + length + 9 + 3 pad

        stellar::SCVal back;
        { QDataStream s(&bytes, QIODevice::ReadOnly); s >> back; }
        QCOMPARE(static_cast<int>(back.type),
                 static_cast<int>(stellar::SCValType::SCV_EXECUTABLE_TAG));
        QCOMPARE(back.executableTag, QByteArray("shared-v1"));
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

    /** Canonical XDR is `SCContractInstance { ContractExecutable executable;
     *  SCMap* storage; }` — a present flag followed by a BARE length-prefixed
     *  SCMapEntry array. The SDK used to serialise `storage` as a whole SCVal,
     *  which put the SCV_MAP discriminant (4 extra bytes) on the wire, so
     *  every instance disagreed with Core while its own round-trip still
     *  passed. This assembles the framing by hand in canonical order and
     *  decodes it, then checks the encoder produces exactly those bytes. */
    void testContractInstanceStorageMatchesCanonicalXdrLayout()
    {
        QByteArray wasmHash(32, '\x0C');

        // Hand-built canonical stream. The two map entries go through the
        // proven SCVal codec; only the instance framing is written literally.
        QByteArray canonical;
        {
            QDataStream s(&canonical, QIODevice::WriteOnly);
            s << static_cast<qint32>(stellar::SCValType::SCV_CONTRACT_INSTANCE);
            s << static_cast<qint32>(stellar::ContractExecutableType::CONTRACT_EXECUTABLE_WASM);
            s.writeRawData(wasmHash.constData(), 32);
            s << static_cast<qint32>(1);      // storage present
            s << static_cast<qint32>(1);      // SCMap length — NO SCV_MAP discriminant
            s << Scv::toSymbol("k");
            s << Scv::toUint32(42);
        }
        QCOMPARE(canonical.size(), 68);       // 72 under the old, wrong shape

        stellar::SCVal decoded;
        { QDataStream s(&canonical, QIODevice::ReadOnly); s >> decoded; }
        stellar::SCContractInstance ci = Scv::fromContractInstance(decoded);
        QCOMPARE(static_cast<int>(ci.executable.type),
                 static_cast<int>(stellar::ContractExecutableType::CONTRACT_EXECUTABLE_WASM));
        for (int i = 0; i < 32; ++i) {
            QCOMPARE(ci.executable.wasmHash[i], static_cast<quint8>('\x0C'));
        }
        QVERIFY(!ci.storage.isNull());
        QList<stellar::SCMapEntry> entries = Scv::fromMap(*ci.storage);
        QCOMPARE(entries.size(), 1);
        QCOMPARE(Scv::fromSymbol(entries.at(0).key), QString("k"));
        QCOMPARE(Scv::fromUint32(entries.at(0).val), 42u);

        // ...and the encoder round-trips back to the very same bytes.
        QByteArray reencoded;
        { QDataStream s(&reencoded, QIODevice::WriteOnly); s << decoded; }
        QCOMPARE(reencoded, canonical);
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
