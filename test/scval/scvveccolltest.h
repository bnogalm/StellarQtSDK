#ifndef SCVVECCOLLTEST_H
#define SCVVECCOLLTEST_H

#include <algorithm>
#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/scval/scv.h"
#include "../../src/contract/sorobancredentialssigner.h"
#include "../../src/keypair.h"
#include "../../src/network.h"

/**
 * SCV_VEC / SCV_MAP helpers and the canonical SCVal comparator.
 */
class ScvVecMapTest: public QObject
{
    Q_OBJECT

    static QByteArray xdrOf(const stellar::SCVal& v)
    {
        QByteArray bytes;
        QDataStream s(&bytes, QIODevice::WriteOnly);
        s << v;
        return bytes;
    }

    static QByteArray be32(qint32 n)
    {
        QByteArray out;
        QDataStream s(&out, QIODevice::WriteOnly);
        s << n;
        return out;
    }

    /** `depth` nested one-element containers (vec, map or contract-instance
     *  storage) around a u32, as bytes: building it as an SCVal would recurse
     *  just as deep in the writer and the destructors. */
    static QByteArray nested(int depth, stellar::SCValType container)
    {
        QByteArray level;
        switch (container) {
        case stellar::SCValType::SCV_VEC:
            level = be32(16) + be32(1) + be32(1); break;
        case stellar::SCValType::SCV_MAP:   // key u32 0, value: the next level
            level = be32(17) + be32(1) + be32(1) + be32(3) + be32(0); break;
        default:                            // stellar-asset executable, one storage entry
            level = be32(19) + be32(1) + be32(1) + be32(1) + be32(3) + be32(0); break;
        }
        QByteArray out;
        for (int i = 1; i < depth; ++i) out += level;
        return out + be32(3) + be32(7);
    }

    /** Decodes all of `bytes` as one SCVal: "ok", or why not. */
    static QString decode(const QByteArray& bytes)
    {
        stellar::SCVal v;
        QDataStream in(bytes);
        try { in >> v; }
        catch (const std::runtime_error& e) { return QString::fromUtf8(e.what()); }
        catch (const std::exception& e) { return QStringLiteral("not a runtime_error: ") + QString::fromUtf8(e.what()); }
        if (in.status() != QDataStream::Ok) return QStringLiteral("stream status %1").arg(int(in.status()));
        return in.atEnd() ? QStringLiteral("ok") : QStringLiteral("trailing bytes");
    }

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

    // ─── Vec ─────────────────────────────────────────────────────────

    void testVecEmpty()
    {
        stellar::SCVal v = Scv::toVec({});
        QCOMPARE(static_cast<int>(v.type),
                 static_cast<int>(stellar::SCValType::SCV_VEC));
        QCOMPARE(Scv::fromVec(roundtripXdr(v)).size(), 0);
    }

    void testVecHeterogeneous()
    {
        QList<stellar::SCVal> elems;
        elems.append(Scv::toUint32(1));
        elems.append(Scv::toBoolean(true));
        elems.append(Scv::toSymbol("hello"));
        elems.append(Scv::toBytes(QByteArray("\x01\x02\x03", 3)));

        stellar::SCVal v = roundtripXdr(Scv::toVec(elems));
        QList<stellar::SCVal> back = Scv::fromVec(v);
        QCOMPARE(back.size(), 4);
        QCOMPARE(Scv::fromUint32(back.at(0)), quint32(1));
        QCOMPARE(Scv::fromBoolean(back.at(1)), true);
        QCOMPARE(Scv::fromSymbol(back.at(2)), QString("hello"));
        QCOMPARE(Scv::fromBytes(back.at(3)), QByteArray("\x01\x02\x03", 3));
    }

    void testVecNested()
    {
        // [ [1, 2], [3] ]
        QList<stellar::SCVal> inner1 = { Scv::toInt32(1), Scv::toInt32(2) };
        QList<stellar::SCVal> inner2 = { Scv::toInt32(3) };
        stellar::SCVal nested = Scv::toVec({ Scv::toVec(inner1), Scv::toVec(inner2) });

        stellar::SCVal back = roundtripXdr(nested);
        QList<stellar::SCVal> outer = Scv::fromVec(back);
        QCOMPARE(outer.size(), 2);
        QList<stellar::SCVal> i1 = Scv::fromVec(outer.at(0));
        QList<stellar::SCVal> i2 = Scv::fromVec(outer.at(1));
        QCOMPARE(i1.size(), 2);
        QCOMPARE(i2.size(), 1);
        QCOMPARE(Scv::fromInt32(i1.at(0)), 1);
        QCOMPARE(Scv::fromInt32(i1.at(1)), 2);
        QCOMPARE(Scv::fromInt32(i2.at(0)), 3);
    }

    // ─── Map ─────────────────────────────────────────────────────────

    void testMapEmpty()
    {
        stellar::SCVal v = Scv::toMap(QList<stellar::SCMapEntry>{});
        QCOMPARE(static_cast<int>(v.type),
                 static_cast<int>(stellar::SCValType::SCV_MAP));
        QCOMPARE(Scv::fromMap(roundtripXdr(v)).size(), 0);
    }

    void testMapParallelLists()
    {
        QList<stellar::SCVal> keys   = { Scv::toSymbol("name"),  Scv::toSymbol("age") };
        QList<stellar::SCVal> values = { Scv::toString(QStringLiteral("alice")), Scv::toUint32(30) };
        stellar::SCVal v = roundtripXdr(Scv::toMap(keys, values));
        QList<stellar::SCMapEntry> entries = Scv::fromMap(v);
        QCOMPARE(entries.size(), 2);
    }

    /** Keys are canonicalized into the host's ascending order. */
    void testMapCanonicalOrdering()
    {
        QList<stellar::SCVal> keys   = { Scv::toUint32(3), Scv::toUint32(1), Scv::toUint32(2) };
        QList<stellar::SCVal> values = { Scv::toSymbol("c"), Scv::toSymbol("a"), Scv::toSymbol("b") };
        QList<stellar::SCMapEntry> entries = Scv::fromMap(Scv::toMap(keys, values));
        QCOMPARE(entries.size(), 3);
        QCOMPARE(Scv::fromUint32(entries.at(0).key), quint32(1));
        QCOMPARE(Scv::fromUint32(entries.at(1).key), quint32(2));
        QCOMPARE(Scv::fromUint32(entries.at(2).key), quint32(3));
        QCOMPARE(Scv::fromSymbol(entries.at(0).val), QString("a"));
        QCOMPARE(Scv::fromSymbol(entries.at(1).val), QString("b"));
        QCOMPARE(Scv::fromSymbol(entries.at(2).val), QString("c"));
    }

    /** Two semantically-equal maps (same keys/values, different input order)
     *  serialize to byte-identical XDR. */
    void testMapByteIdenticalForReorderedInput()
    {
        QList<stellar::SCVal> k1 = { Scv::toUint32(1), Scv::toUint32(2), Scv::toUint32(3) };
        QList<stellar::SCVal> v1 = { Scv::toInt32(10), Scv::toInt32(20), Scv::toInt32(30) };
        QList<stellar::SCVal> k2 = { Scv::toUint32(3), Scv::toUint32(1), Scv::toUint32(2) };
        QList<stellar::SCVal> v2 = { Scv::toInt32(30), Scv::toInt32(10), Scv::toInt32(20) };

        QByteArray a, b;
        { QDataStream s(&a, QIODevice::WriteOnly); s << Scv::toMap(k1, v1); }
        { QDataStream s(&b, QIODevice::WriteOnly); s << Scv::toMap(k2, v2); }
        QCOMPARE(a, b);
    }

    void testMapRejectsDuplicateKeys()
    {
        QList<stellar::SCVal> keys = { Scv::toUint32(1), Scv::toUint32(1) };
        QList<stellar::SCVal> values = { Scv::toInt32(1), Scv::toInt32(2) };
        bool threw = false;
        try { Scv::toMap(keys, values); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    void testMapKeysValuesLengthMismatch()
    {
        bool threw = false;
        try {
            Scv::toMap({ Scv::toUint32(1) }, { Scv::toInt32(1), Scv::toInt32(2) });
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ─── Comparator ──────────────────────────────────────────────────

    void testCompareSameValueIsZero()
    {
        QCOMPARE(Scv::compare(Scv::toUint32(42), Scv::toUint32(42)), 0);
        QCOMPARE(Scv::compare(Scv::toSymbol("abc"), Scv::toSymbol("abc")), 0);
    }

    void testCompareByTypeDiscriminator()
    {
        // SCV_BOOL (0) precedes SCV_VOID (1) precedes SCV_U32 (3) ...
        QVERIFY(Scv::compare(Scv::toBoolean(true), Scv::toVoid()) < 0);
        QVERIFY(Scv::compare(Scv::toVoid(), Scv::toUint32(0)) < 0);
    }

    void testCompareByContent()
    {
        QVERIFY(Scv::compare(Scv::toUint32(1), Scv::toUint32(2)) < 0);
        QVERIFY(Scv::compare(Scv::toUint32(2), Scv::toUint32(1)) > 0);
    }

    /**
     * Map keys go in the order the host checks them against (Rust's Ord on
     * ScVal), not in the order of their XDR bytes. The bytes put a symbol's
     * length first and store -1 as ffffffff, so {amount, to}, {-1, 1},
     * {"aa", "b"} and {0101, 02} were all sorted backwards and the host
     * refused the map ("ScMap was not sorted by key"). Expected bytes are py
     * stellar_sdk 14.1 to_map's, for the keys given in either order.
     */
    void testMapKeysFollowTheHostOrder()
    {
        auto u = [](quint32 n) { return Scv::toUint32(n); };
        auto both = [](const stellar::SCVal& k1, const stellar::SCVal& v1,
                       const stellar::SCVal& k2, const stellar::SCVal& v2) {
            const QByteArray a = xdrOf(Scv::toMap({ k1, k2 }, { v1, v2 }));
            const QByteArray b = xdrOf(Scv::toMap({ k2, k1 }, { v2, v1 }));
            return a == b ? a.toHex() : QByteArray("depends on the input order");
        };
        QCOMPARE(both(Scv::toSymbol("to"), u(2), Scv::toSymbol("amount"), u(1)),
                 QByteArray("0000001100000001000000020000000f00000006616d6f756e74000000000003000000010000000f00000002746f00000000000300000002"));
        QCOMPARE(both(Scv::toSymbol("signature"), u(2), Scv::toSymbol("public_key"), u(1)),
                 QByteArray("0000001100000001000000020000000f0000000a7075626c69635f6b6579000000000003000000010000000f000000097369676e61747572650000000000000300000002"));
        QCOMPARE(both(Scv::toInt32(1), u(2), Scv::toInt32(-1), u(1)),
                 QByteArray("00000011000000010000000200000004ffffffff000000030000000100000004000000010000000300000002"));
        QCOMPARE(both(Scv::toInt64(5), u(2), Scv::toInt64(-5), u(1)),
                 QByteArray("00000011000000010000000200000006fffffffffffffffb00000003000000010000000600000000000000050000000300000002"));
        QCOMPARE(both(Scv::toInt128(1), u(2), Scv::toInt128(-1), u(1)),
                 QByteArray("0000001100000001000000020000000affffffffffffffffffffffffffffffff00000003000000010000000a000000000000000000000000000000010000000300000002"));
        QCOMPARE(both(Scv::toString(QStringLiteral("b")), u(2), Scv::toString(QStringLiteral("aa")), u(1)),
                 QByteArray("0000001100000001000000020000000e000000026161000000000003000000010000000e00000001620000000000000300000002"));
        QCOMPARE(both(Scv::toBytes(QByteArray::fromHex("02")), u(2), Scv::toBytes(QByteArray::fromHex("0101")), u(1)),
                 QByteArray("0000001100000001000000020000000d000000020101000000000003000000010000000d00000001020000000000000300000002"));

        // Mixed types: by type first, then within each type as above.
        const QByteArray mixed = xdrOf(Scv::toMap(
            { Scv::toSymbol("b"), Scv::toUint32(7), Scv::toInt64(-3), Scv::toBytes(QByteArray(1, '\0')),
              Scv::toSymbol("aaa"), Scv::toInt64(2) },
            { u(1), u(2), u(3), u(4), u(5), u(6) }));
        QCOMPARE(mixed.toHex(), QByteArray(
            "0000001100000001000000060000000300000007000000030000000200000006fffffffffffffffd0000000300000003"
            "00000006000000000000000200000003000000060000000d000000010000000000000003000000040000000f00000003"
            "6161610000000003000000050000000f00000001620000000000000300000001"));
    }

    /**
     * Maps and vecs, against js-stellar-sdk 17.1.0 nativeToScVal; the two maps
     * are also py stellar_sdk 14.1.0 scval.to_map's bytes. Symbol keys sort as
     * raw bytes, so "B1" < "Z" < "_x" < "aa" < "amount" < "b" < "to" (upper
     * case and '_' before lower case), and i128 keys as numbers, -2^64 first.
     * The vec nests a typed u32 / symbol beside js's untyped defaults (true,
     * null as void, 2 and 3 as u64), and an empty vec is present, not absent.
     */
    void testMapsAndVecsMatchJsNativeToScVal()
    {
        auto u = [](quint32 n) { return Scv::toUint32(n); };
        const stellar::SCVal symbols = Scv::toMap(
            { Scv::toSymbol("to"), Scv::toSymbol("amount"), Scv::toSymbol("Z"), Scv::toSymbol("_x"),
              Scv::toSymbol("aa"), Scv::toSymbol("b"), Scv::toSymbol("B1") },
            { u(1), u(2), u(3), u(4), u(5), u(6), u(7) });
        QCOMPARE(xdrOf(symbols).toHex(), QByteArray(
            "0000001100000001000000070000000f000000024231000000000003000000070000000f000000015a00000000000003"
            "000000030000000f000000025f78000000000003000000040000000f000000026161000000000003000000050000000f"
            "00000006616d6f756e74000000000003000000020000000f000000016200000000000003000000060000000f00000002"
            "746f00000000000300000001"));
        QStringList order;
        for (const stellar::SCMapEntry& e : Scv::fromMap(symbols)) order << Scv::fromSymbol(e.key);
        QCOMPARE(order, QStringList({ "B1", "Z", "_x", "aa", "amount", "b", "to" }));

        const stellar::SCVal numbers = Scv::toMap(
            { Scv::toInt128Parts(qint64(1), quint64(0)), Scv::toInt128(-1), Scv::toInt128(0),
              Scv::toInt128Parts(qint64(-1), quint64(0)) },
            { u(1), u(2), u(3), u(4) });
        QCOMPARE(xdrOf(numbers).toHex(), QByteArray(
            "0000001100000001000000040000000affffffffffffffff000000000000000000000003000000040000000affffffff"
            "ffffffffffffffffffffffff00000003000000020000000a000000000000000000000000000000000000000300000003"
            "0000000a000000000000000100000000000000000000000300000001"));

        QCOMPARE(xdrOf(Scv::toVec({ u(1), Scv::toSymbol("a"), Scv::toBoolean(true), Scv::toVoid(),
                                    Scv::toVec({ Scv::toUint64(quint64(2)), Scv::toUint64(quint64(3)) }) }))
                     .toHex(), QByteArray(
            "00000010000000010000000500000003000000010000000f000000016100000000000000000000010000000100000010"
            "0000000100000002000000050000000000000002000000050000000000000003"));
        QCOMPARE(xdrOf(Scv::toVec({ Scv::toVec({ u(1), u(2) }), Scv::toVec({}) })).toHex(), QByteArray(
            "000000100000000100000002000000100000000100000002000000030000000100000003000000020000001000000001"
            "00000000"));
        QCOMPARE(xdrOf(Scv::toVec({ Scv::toInt64(qint64(-5)), Scv::toInt64(qint64(7)) })).toHex(),
                 QByteArray("00000010000000010000000200000006fffffffffffffffb000000060000000000000007"));
    }

    /** compare() is the host's order across every kind of key, checked against
     *  py stellar_sdk's _compare_sc_val sorting the same values. */
    void testCompareIsTheHostOrder()
    {
        auto u = [](quint32 n) { return Scv::toUint32(n); };
        stellar::SCVal absentVec;
        absentVec.type = stellar::SCValType::SCV_VEC;   // null vec: the XDR optional left out
        QVERIFY(Scv::compare(Scv::toSymbol("amount"), Scv::toSymbol("to")) < 0);
        QVERIFY(Scv::compare(Scv::toInt32(-1), Scv::toInt32(1)) < 0);
        QVERIFY(Scv::compare(Scv::toInt128(-1), Scv::toInt128(1)) < 0);
        QVERIFY(Scv::compare(Scv::toLedgerKeyNonce(qint64(-1)), Scv::toLedgerKeyNonce(qint64(1))) < 0);
        QVERIFY(Scv::compare(Scv::toSymbol("ab"), Scv::toSymbol("abc")) < 0);   // a prefix first
        QVERIFY(Scv::compare(absentVec, Scv::toVec({})) < 0);
        QCOMPARE(Scv::compare(Scv::toInt128(-7), Scv::toInt128(-7)), 0);

        QList<stellar::SCVal> keys = {
            Scv::toVec({ u(1), u(2) }), Scv::toVec({ u(1) }), Scv::toVec({ u(0), u(9) }),
            Scv::toUint128(quint64(5)), Scv::toUint128Parts(quint64(1), quint64(0)),
            Scv::toInt256(QByteArray(1, '\xff')), Scv::toInt256(QByteArray(1, '\x03')),
            Scv::toMap({ Scv::toSymbol("a") }, { u(2) }),
            Scv::toMap({ Scv::toSymbol("a"), Scv::toSymbol("b") }, { u(1), u(0) }),
            Scv::toBoolean(true), Scv::toBoolean(false), Scv::toVoid(),
            Scv::toDuration(quint64(9)), Scv::toTimePoint(quint64(10)),
        };
        std::sort(keys.begin(), keys.end(), [](const stellar::SCVal& a, const stellar::SCVal& b) {
            return Scv::compare(a, b) < 0;
        });
        QStringList order;
        for (const stellar::SCVal& k : keys) order << QString::fromLatin1(xdrOf(k).toHex());
        QCOMPARE(order, QStringList({
            "0000000000000000", "0000000000000001", "00000001",
            "00000007000000000000000a", "000000080000000000000009",
            "0000000900000000000000000000000000000005", "0000000900000000000000010000000000000000",
            "0000000cffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
            "0000000c0000000000000000000000000000000000000000000000000000000000000003",
            "00000010000000010000000200000003000000000000000300000009",
            "0000001000000001000000010000000300000001",
            "00000010000000010000000200000003000000010000000300000002",
            "0000001100000001000000020000000f000000016100000000000003000000010000000f00000001620000000000000300000000",
            "0000001100000001000000010000000f00000001610000000000000300000002" }));
    }

    /**
     * The account signature in a Soroban auth entry is the map {public_key,
     * signature}, and it was signed as {signature, public_key}: every entry
     * a G account authorized through authorizeEntry was refused by the host
     * on testnet, fee charged. Expected is js-stellar-sdk 17.1 authorizeEntry
     * for the same entry, key and expiration (a throwaway testnet account).
     */
    void testAccountSignatureMatchesJsAuthorizeEntry()
    {
        stellar::SorobanAuthorizationEntry entry;
        {
            const QByteArray raw = QByteArray::fromBase64(
                "AAAAAgAAAAAAAAAAjqjClHuXrIUFGxufjd/5qExy9vI4ZYVdxpe8FHpSQ/0v2t5u+8II0AAAAAAAAAABAAAAAAAAAAHX"
                "kotywnA8z+r365/0701QSlWouXn8m0UOoshCtNHOYQAAAAh0cmFuc2ZlcgAAAAMAAAASAAAAAAAAAACOqMKUe5eshQUb"
                "G5+N3/moTHL28jhlhV3Gl7wUelJD/QAAABIAAAAAAAAAAGO4LtAZTWJ6++GZKG0kqO94LjIi6f8hLidJG3oLjOrOAAAA"
                "CgAAAAAAAAAAAAAAAAAAAAEAAAAA");
            QDataStream in(raw);
            in >> entry;
            QCOMPARE(in.status(), QDataStream::Ok);
        }
        QScopedPointer<KeyPair> kp(KeyPair::fromSecretSeed(
            QStringLiteral("SDE7V6VRS5GEFRULWWX54FOYTCGO2ZUDLJ6VIR6CUYWTYF5KLF2GTYTE")));
        const stellar::SorobanAuthorizationEntry signed_ = SorobanCredentialsSigner::authorizeEntry(
            entry, kp.data(), 4865004, Network::testnetNetwork()->getNetworkPassphrase());
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << signed_; }
        QCOMPARE(QString::fromLatin1(bytes.toBase64()), QStringLiteral(
            "AAAAAgAAAAAAAAAAjqjClHuXrIUFGxufjd/5qExy9vI4ZYVdxpe8FHpSQ/0v2t5u+8II0ABKO+wAAAAQAAAAAQAAAAEAAAAR"
            "AAAAAQAAAAIAAAAPAAAACnB1YmxpY19rZXkAAAAAAA0AAAAgjqjClHuXrIUFGxufjd/5qExy9vI4ZYVdxpe8FHpSQ/0AAAAP"
            "AAAACXNpZ25hdHVyZQAAAAAAAA0AAABApp6vQi3aaea15A8Vkzb1x16c7sy8dYYBI/VcI63w51Xrg7HguQ1rrl36dcfyKq+k"
            "3M7qeUjQOspCOJLKX4sMCwAAAAAAAAAB15KLcsJwPM/q9+uf9O9NUEpVqLl5/JtFDqLIQrTRzmEAAAAIdHJhbnNmZXIAAAAD"
            "AAAAEgAAAAAAAAAAjqjClHuXrIUFGxufjd/5qExy9vI4ZYVdxpe8FHpSQ/0AAAASAAAAAAAAAABjuC7QGU1ievvhmShtJKjv"
            "eC4yIun/IS4nSRt6C4zqzgAAAAoAAAAAAAAAAAAAAAAAAAABAAAAAA=="));
    }

    // ─── Decoding hostile input ──────────────────────────────────────

    /**
     * Nesting is limited to MAX_SCVAL_DEPTH, one past the host's own limit of
     * 100 with room to spare. ~400 nested vectors (5 KB) used to overflow the
     * stack, STATUS_STACK_OVERFLOW, which no catch() intercepts. The limit is
     * decoded in this debug build, so its stack is known to hold it.
     */
    void testNestingIsLimited()
    {
        const int max = stellar::MAX_SCVAL_DEPTH;
        QCOMPARE(max, 128);
        for (stellar::SCValType kind : { stellar::SCValType::SCV_VEC, stellar::SCValType::SCV_MAP,
                                         stellar::SCValType::SCV_CONTRACT_INSTANCE }) {
            QCOMPARE(decode(nested(max, kind)), QStringLiteral("ok"));
            QCOMPARE(decode(nested(max + 1, kind)), QStringLiteral("SCVal: nesting too deep"));
            QCOMPARE(decode(nested(5000, kind)), QStringLiteral("SCVal: nesting too deep"));
        }
        // The counter unwinds with the exception: the limit still decodes after.
        QCOMPARE(stellar::scValDepth(), 0);
        QCOMPARE(decode(nested(max, stellar::SCValType::SCV_VEC)), QStringLiteral("ok"));
    }

    /**
     * A count is checked against the bytes left before anything is allocated
     * for it. 12 bytes claiming 3,000,000 vector elements used to commit 3 GB
     * and only then report a truncated stream, and a claimed 1 GB byte string
     * allocated 1 GB. A loop also stops at the first element that runs dry,
     * where it used to append default values for the rest of the count.
     */
    void testCountsAreBoundedByTheInput()
    {
        const QString tooLong = QStringLiteral(": length exceeds the remaining input");
        QCOMPARE(decode(be32(16) + be32(1) + be32(100000)), QStringLiteral("SCVec") + tooLong);
        QCOMPARE(decode(be32(16) + be32(1) + be32(0x7fffffff)), QStringLiteral("SCVec") + tooLong);
        QCOMPARE(decode(be32(17) + be32(1) + be32(100000)), QStringLiteral("SCMap") + tooLong);
        QCOMPARE(decode(be32(13) + be32(100000000)), QStringLiteral("SCBytes") + tooLong);
        QCOMPARE(decode(be32(14) + be32(100000000)), QStringLiteral("SCString") + tooLong);
        QCOMPARE(decode(be32(22) + be32(100000000)), QStringLiteral("SCV_EXECUTABLE_TAG") + tooLong);
        QCOMPARE(decode(be32(19) + be32(1) + be32(1) + be32(100000)),
                 QStringLiteral("SCContractInstance storage") + tooLong);
        // External-ref tag: owner is a contract address, then the tag length.
        QCOMPARE(decode(be32(19) + be32(2) + be32(1) + QByteArray(32, '\x01') + be32(100000000)),
                 QStringLiteral("ContractExecutableExternalRef tag") + tooLong);
        QCOMPARE(decode(be32(16) + be32(1) + be32(-1)), QStringLiteral("SCVec: negative length"));

        // Three elements claimed, twelve bytes left: enough by count, but the
        // first element (a u64) takes all of them.
        QCOMPARE(decode(be32(16) + be32(1) + be32(3) + be32(5) + be32(0) + be32(5)),
                 QStringLiteral("SCVec: truncated or malformed element"));
        QCOMPARE(decode(be32(17) + be32(1) + be32(2) + be32(5) + be32(0) + be32(5) + be32(3)),
                 QStringLiteral("SCMap: truncated or malformed element"));

        // What fits exactly still decodes.
        QCOMPARE(decode(be32(16) + be32(1) + be32(3) + be32(3) + be32(1) + be32(3) + be32(2)
                        + be32(1)), QStringLiteral("ok"));
        QCOMPARE(decode(be32(13) + be32(3) + QByteArray::fromHex("01020300")), QStringLiteral("ok"));
    }
};

ADD_TEST(ScvVecMapTest)
#endif // SCVVECCOLLTEST_H
