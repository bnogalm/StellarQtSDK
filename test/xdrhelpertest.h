#ifndef XDRHELPERTEST_H
#define XDRHELPERTEST_H
#include <QObject>
#include <QtTest>
#include <QByteArray>
#include <QDataStream>
#include "testcollector.h"

#include "../xdr/xdrhelper.h"
#include "../xdr/scval.h"
#include "../xdr/sorobanops.h"
#include "../xdr/sorobantxdata.h"
#include "../src/memo.h"
#include "../src/util.h"

/**
 * Tests for `xdr::Array<T,max>` and `xdr::Optional<T>` covering the gaps
 * documented in `problems-report.md`:
 *   - §3.3 Array::set bytes vs elements
 *   - §3.4 #elif with no condition (binary())
 *   - §3.5 deserializer ignores max
 *   - §3.6 padding check
 *   - §3.7 append silently drops
 *
 * Includes a minimal port of `xdr/PaddingTest.java` and
 * `XdrDataStreamTest.java` (UTF-8 round-trip via Memo) from java-stellar-sdk.
 */
class XdrHelperTest: public QObject
{
    Q_OBJECT

    static QByteArray be32(quint32 v)
    {
        QByteArray out(4, '\0');
        out[0] = char(v >> 24);
        out[1] = char(v >> 16);
        out[2] = char(v >> 8);
        out[3] = char(v);
        return out;
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    // ============ §3.5 — deserializer respects max ============

    void testDeserializerRejectsLengthAboveMax()
    {
        // Stream with an Array<quint32, 2> claiming length=5.
        QByteArray buf;
        QDataStream out(&buf, QIODevice::WriteOnly);
        const qint32 n = 5;
        out << n;
        for (qint32 i = 0; i < n; ++i)
            out << (quint32)i;
        // 5 quint32 = 20 bytes, already a multiple of 4 → no padding.

        xdr::Array<quint32, 2> arr;
        QDataStream in(&buf, QIODevice::ReadOnly);
        bool threw = false;
        try {
            in >> arr;
        } catch (const std::runtime_error& e) {
            threw = true;
            QVERIFY(QString::fromLatin1(e.what()).contains("out of bounds"));
        }
        QVERIFY2(threw, "deserializer must throw when n > max");
    }

    void testDeserializerAcceptsLengthAtMax()
    {
        QByteArray buf;
        QDataStream out(&buf, QIODevice::WriteOnly);
        const qint32 n = 3;
        out << n;
        for (qint32 i = 0; i < n; ++i)
            out << (quint32)(i * 10);

        xdr::Array<quint32, 3> arr;
        QDataStream in(&buf, QIODevice::ReadOnly);
        in >> arr;
        QCOMPARE(arr.value.size(), 3);
        QCOMPARE(arr.value[0], (quint32)0);
        QCOMPARE(arr.value[1], (quint32)10);
        QCOMPARE(arr.value[2], (quint32)20);
    }

    // ============ A count the input cannot hold ============

    /**
     * An array whose input ends before its last element is a short read. The
     * reader stopped quietly at the end of the input, so an envelope declaring
     * two signatures and carrying one decoded as complete, with one.
     */
    void testATruncatedArrayIsAShortRead()
    {
        auto status = [](const QByteArray& bytes, auto&& arr) {
            QDataStream in(bytes);
            in >> arr;
            return in.status();
        };
        QCOMPARE(status(be32(2) + be32(7), xdr::Array<quint32, 8>()), QDataStream::ReadPastEnd);  // one of two
        QCOMPARE(status(be32(2), xdr::Array<quint32, 8>()), QDataStream::ReadPastEnd);            // none of two
        QCOMPARE(status(be32(2) + be32(7) + be32(8), xdr::Array<quint32, 8>()), QDataStream::Ok);
        // 64 bytes declared, 32 present: a multiple of four, so no padding read to trip on either.
        QCOMPARE(status(be32(64) + QByteArray(32, 'x'), xdr::Array<quint8, 64>()), QDataStream::ReadPastEnd);
        QCOMPARE(status(be32(4) + QByteArray(4, 'x'), xdr::Array<quint8, 64>()), QDataStream::Ok);
    }

    /** Decoding replaces what the array held: it used to append to it. */
    void testReadingAnArrayReplacesItsContent()
    {
        xdr::Array<quint32, 8> arr;
        arr.value << 9 << 9 << 9;
        QByteArray buf = be32(2) + be32(1) + be32(2);
        QDataStream in(buf);
        in >> arr;
        QCOMPARE(in.status(), QDataStream::Ok);
        QVERIFY(arr.value == QVector<quint32>({1, 2}));
    }

    /**
     * readXdrList reads an unbounded XDR array into a QVector. The result
     * arrays used QDataStream's own QVector reader instead, which reserved
     * whatever count arrived and, from Qt 6.7, read 0xFFFFFFFE as "a 64-bit
     * count follows".
     */
    void testReadXdrListRefusesACountTheInputCannotHold()
    {
        auto read = [](const QByteArray& bytes, QVector<quint32>& list) {
            QDataStream in(bytes);
            xdr::readXdrList(in, list);
            return in.status();
        };
        QVector<quint32> list;
        QCOMPARE(read(be32(2) + be32(5) + be32(6), list), QDataStream::Ok);
        QVERIFY(list == QVector<quint32>({5, 6}));
        QVERIFY(read(be32(3) + be32(5) + be32(6), list) != QDataStream::Ok);
        QVERIFY(read(be32(0x7fffffff), list) != QDataStream::Ok);
        QVERIFY(list.isEmpty() && list.capacity() < 16);    // refused before anything was reserved
        QVERIFY(read(be32(0xfffffffe) + be32(0) + be32(1) + be32(5), list) != QDataStream::Ok);

        QByteArray out;
        QDataStream s(&out, QIODevice::WriteOnly);
        xdr::writeXdrList(s, QVector<quint32>({5, 6}));
        QCOMPARE(out, be32(2) + be32(5) + be32(6));
    }

    /**
     * The padding of an SCVal's bytes, string, symbol and executable tag, a
     * ContractExecutable's external-ref tag, an invocation's function name
     * and an uploaded wasm is zeros. Each vector is refused by js-stellar-sdk
     * 17.1.0 with "non-zero XDR padding" (and by core), and its zero-padded
     * twin decodes; they used to be read and re-encoded with zeros (as
     * py-stellar-sdk 14.1.0 still does).
     */
    void testSorobanOpaquePaddingMustBeZero()
    {
        auto refused = [](const QByteArray& hex, auto value) {
            QDataStream in(QByteArray::fromHex(hex));
            QString message;
            try { in >> value; } catch (const std::runtime_error& e) { message = QString::fromLatin1(e.what()); }
            return message.contains(QLatin1String("non-zero padding"))
                    && in.status() == QDataStream::ReadCorruptData;
        };
        auto reads = [](const QByteArray& hex, auto value) {
            QDataStream in(QByteArray::fromHex(hex));
            try { in >> value; } catch (const std::runtime_error&) { return false; }
            return in.status() == QDataStream::Ok && in.atEnd();
        };
        const QByteArray contract = "00000001" + QByteArray(64, '1');
        const QList<QPair<QByteArray, QByteArray>> scVals = {
            { "0000000d0000000141010000", "0000000d0000000141000000" },   // SCV_BYTES "A"
            { "0000000e0000000141000001", "0000000e0000000141000000" },   // SCV_STRING "A"
            { "0000000f0000000161010000", "0000000f0000000161000000" },   // SCV_SYMBOL "a"
            { "000000160000000161000100", "000000160000000161000000" },   // SCV_EXECUTABLE_TAG "a"
        };
        for (const auto& v : scVals) {
            QVERIFY2(refused(v.first, stellar::SCVal()), v.first.constData());
            QVERIFY2(reads(v.second, stellar::SCVal()), v.second.constData());
        }
        QVERIFY(refused("00000002" + contract + "0000000161000001", stellar::ContractExecutable()));
        QVERIFY(reads("00000002" + contract + "0000000161000000", stellar::ContractExecutable()));
        QVERIFY(refused(contract + "0000000161010000" + "00000000", stellar::InvokeContractArgs()));
        QVERIFY(reads(contract + "0000000161000000" + "00000000", stellar::InvokeContractArgs()));
        QVERIFY(refused("0000000200000003006173" "01", stellar::HostFunction()));
        QVERIFY(reads("0000000200000003006173" "00", stellar::HostFunction()));
    }

    /**
     * A footprint count is checked against the input but nothing is reserved
     * for it: 4 bytes per claimed key used to reserve a whole SorobanLedgerKey
     * each (~240x the input with Qt 6) before the first key was refused. js and py
     * build the lists key by key too.
     */
    void testAFootprintCountReservesNothing()
    {
        const int claimed = 1000;
        const QByteArray keys(4 * claimed, char(0xff));   // no such LedgerEntryType
        const QList<QByteArray> hostile = { be32(claimed) + keys,
                                            be32(0) + be32(claimed) + keys };
        for (const QByteArray& bytes : hostile) {
            stellar::LedgerFootprint f;
            QDataStream in(bytes);
            bool threw = false;
            try { in >> f; } catch (const std::runtime_error&) { threw = true; }
            QVERIFY(threw);
            QVERIFY(f.readOnly.isEmpty() && f.readWrite.isEmpty());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            // Qt 6's QList holds the keys themselves (Qt 5's has no capacity()).
            QVERIFY(f.readOnly.capacity() < 16 && f.readWrite.capacity() < 16);
#endif
        }
        stellar::LedgerFootprint f;
        QDataStream in(be32(0) + be32(0));
        in >> f;
        QCOMPARE(in.status(), QDataStream::Ok);
        QVERIFY(f.readOnly.isEmpty() && f.readWrite.isEmpty());
    }

    // ============ §3.6 — padding byte-by-byte check ============

    void testPaddingMustBeZero()
    {
        // Array<quint8> with 2 bytes 'a' 'b' → 2-byte padding.
        // First padding byte is 0x01 (non-zero) — must be rejected.
        // Layout: [00 00 00 02] [61 62] [01 00]  (same fixture as PaddingTest.java)
        const quint8 raw[] = {0x00, 0x00, 0x00, 0x02, 0x61, 0x62, 0x01, 0x00};
        QByteArray buf(reinterpret_cast<const char*>(raw), sizeof(raw));

        xdr::Array<quint8, 32> arr;
        QDataStream in(&buf, QIODevice::ReadOnly);
        bool threw = false;
        try {
            in >> arr;
        } catch (const std::runtime_error& e) {
            threw = true;
            QVERIFY(QString::fromLatin1(e.what()).contains("padding"));
        }
        QVERIFY2(threw, "non-zero padding must be rejected");
    }

    void testZeroPaddingAccepted()
    {
        // Same as above but padding 00 00 → must pass.
        const quint8 raw[] = {0x00, 0x00, 0x00, 0x02, 0x61, 0x62, 0x00, 0x00};
        QByteArray buf(reinterpret_cast<const char*>(raw), sizeof(raw));

        xdr::Array<quint8, 32> arr;
        QDataStream in(&buf, QIODevice::ReadOnly);
        in >> arr;
        QCOMPARE(arr.value.size(), 2);
        QCOMPARE(arr.value[0], (quint8)'a');
        QCOMPARE(arr.value[1], (quint8)'b');
    }

    // ============ §3.3 — Array::set with multi-byte T ============

    void testArraySetWithMultiByteType()
    {
        // With T = quint32 (sizeof=4) the old set() did memcpy(dest, src,
        // count) instead of count*sizeof(T) — only copied the first byte of
        // each element. Verify that all 4 bytes are copied.
        xdr::Array<quint32, 8> arr;
        const quint32 src[] = {0x11223344u, 0x55667788u, 0x99AABBCCu};
        arr.set(const_cast<quint32*>(src), 3);
        QCOMPARE(arr.value.size(), 3);
        QCOMPARE(arr.value[0], (quint32)0x11223344u);
        QCOMPARE(arr.value[1], (quint32)0x55667788u);
        QCOMPARE(arr.value[2], (quint32)0x99AABBCCu);
    }

    void testArraySetTruncatesAtMax()
    {
        xdr::Array<quint32, 2> arr;
        const quint32 src[] = {1u, 2u, 3u, 4u};
        arr.set(const_cast<quint32*>(src), 4);
        // max=2 → only the first two.
        QCOMPARE(arr.value.size(), 2);
        QCOMPARE(arr.value[0], (quint32)1u);
        QCOMPARE(arr.value[1], (quint32)2u);
    }

    // ============ Basic Array round-trip with padding ============

    void testArrayRoundTripWithPadding()
    {
        // 3 bytes → 1 byte of padding
        xdr::Array<quint8, 16> orig;
        orig.value.append('h');
        orig.value.append('i');
        orig.value.append('!');

        QByteArray buf;
        QDataStream out(&buf, QIODevice::WriteOnly);
        out << orig;
        // Length(4) + 3 bytes + 1 padding = 8 bytes
        QCOMPARE(buf.size(), 8);
        QCOMPARE((quint8)buf[7], (quint8)0); // padding byte zeroed

        xdr::Array<quint8, 16> deser;
        QDataStream in(&buf, QIODevice::ReadOnly);
        in >> deser;
        QCOMPARE(deser.value.size(), 3);
        QCOMPARE(deser.value[0], (quint8)'h');
        QCOMPARE(deser.value[1], (quint8)'i');
        QCOMPARE(deser.value[2], (quint8)'!');
    }

    // ============ Optional round-trip ============

    void testOptionalFilledRoundTrip()
    {
        xdr::Optional<quint32> opt;
        opt.filler() = 0xDEADBEEFu;
        QCOMPARE((qint32)opt.filled, (qint32)1);

        QByteArray buf;
        QDataStream out(&buf, QIODevice::WriteOnly);
        out << opt;

        xdr::Optional<quint32> deser;
        QDataStream in(&buf, QIODevice::ReadOnly);
        in >> deser;
        QVERIFY(deser.filled);
        QCOMPARE(deser.value, (quint32)0xDEADBEEFu);
    }

    void testOptionalUnfilledRoundTrip()
    {
        xdr::Optional<quint32> opt; // default: filled=0
        QByteArray buf;
        QDataStream out(&buf, QIODevice::WriteOnly);
        out << opt;
        // Only the discriminant (4 bytes).
        QCOMPARE(buf.size(), 4);

        xdr::Optional<quint32> deser;
        QDataStream in(&buf, QIODevice::ReadOnly);
        in >> deser;
        QVERIFY(!deser.filled);
    }

    // ============ §9.8 — MemoId accepts full uint64 range ============

    void testMemoIdQuint64MaxRange()
    {
        // 2^64 - 1 = 18446744073709551615
        const quint64 maxId = std::numeric_limits<quint64>::max();
        Memo* m = Memo::parse(QStringLiteral("id"),
                              QByteArray::number(static_cast<qulonglong>(maxId)));
        auto* mid = dynamic_cast<MemoId*>(m);
        QVERIFY(mid != nullptr);
        QCOMPARE(mid->getId(), maxId);
        delete m;
    }

    void testMemoIdLargeButValid()
    {
        // ID > qint64::max but valid as uint64 — used to throw.
        Memo* m = Memo::parse(QStringLiteral("id"),
                              QByteArray("10000000000000000000")); // 1e19, > qint64::max ≈ 9.22e18
        auto* mid = dynamic_cast<MemoId*>(m);
        QVERIFY(mid != nullptr);
        QCOMPARE(mid->getId(), (quint64)10000000000000000000ull);
        delete m;
    }

    // ============ Port of XdrDataStreamTest.java — UTF-8 in memo ============

    void testMemoTextUtf8RoundTripStandardAscii()
    {
        Memo* m1 = Memo::text(QStringLiteral("Dollar Sign $"));
        stellar::Memo xdr = m1->toXdr();
        Memo* m2 = Memo::fromXdr(xdr);
        QCOMPARE(dynamic_cast<MemoText*>(m2)->getText(), QStringLiteral("Dollar Sign $"));
        delete m1;
        delete m2;
    }

    void testMemoTextUtf8RoundTripEuroSign()
    {
        Memo* m1 = Memo::text(QStringLiteral("Euro Sign €"));
        stellar::Memo xdr = m1->toXdr();
        Memo* m2 = Memo::fromXdr(xdr);
        QCOMPARE(dynamic_cast<MemoText*>(m2)->getText(), QStringLiteral("Euro Sign €"));
        delete m1;
        delete m2;
    }

    // ============ XDR bools and optional flags are 0 or 1 ============

    /** An optional's present flag is 0 or 1. Any other value was read as
     *  present, and written back as 1: bytes that decoded, and re-encoded as
     *  different ones. A flag set to 2 in memory was written as 2. */
    void testAnOptionalFlagIsZeroOrOne()
    {
        for (quint32 flag : { 2u, 0xFFFFFFFFu, 0x100u }) {
            const QByteArray bytes = be32(flag) + be32(5);
            xdr::Optional<quint32> opt;
            QDataStream in(bytes);
            in >> opt;
            QCOMPARE(in.status(), QDataStream::ReadCorruptData);
            QCOMPARE(opt.filled, 0);
        }
        xdr::Optional<quint32> present;
        {
            QDataStream in(be32(1) + be32(5));
            in >> present;
            QCOMPARE(in.status(), QDataStream::Ok);
            QCOMPARE(present.filled, 1);
            QCOMPARE(present.value, 5u);
        }
        present.filled = 2;
        QByteArray out;
        QDataStream s(&out, QIODevice::WriteOnly);
        s << present;
        QCOMPARE(s.status(), QDataStream::WriteFailed);
        QVERIFY(out.isEmpty());
    }

    /** SCV_BOOL, and the present flags of SCV_VEC, SCV_MAP and a contract
     *  instance's storage, are 0 or 1 too: 2 was read as true (or present)
     *  and written back as 1. Like every other malformed SCVal they mark the
     *  stream corrupt and throw. */
    void testAnScValBoolOrPresentFlagIsZeroOrOne()
    {
        const QList<QByteArray> refused = {
            be32(0) + be32(2),                          // SCV_BOOL
            be32(16) + be32(2) + be32(0),               // SCV_VEC present flag
            be32(17) + be32(2) + be32(0),               // SCV_MAP present flag
            be32(19) + be32(1) + be32(2) + be32(0),     // SCV_CONTRACT_INSTANCE (stellar asset) storage flag
        };
        for (const QByteArray& bytes : refused) {
            stellar::SCVal value;
            QDataStream in(bytes);
            bool threw = false;
            try { in >> value; } catch (const std::runtime_error&) { threw = true; }
            QVERIFY(threw);
            QCOMPARE(in.status(), QDataStream::ReadCorruptData);
        }
        for (quint32 flag : { 0u, 1u }) {
            stellar::SCVal value;
            QDataStream in(be32(0) + be32(flag));
            in >> value;
            QCOMPARE(in.status(), QDataStream::Ok);
            QCOMPARE(value.b, flag == 1u);
        }
    }

    /** A reserved ext (`union switch (int v) { case 0: void; }`) is 0: any
     *  other value was read and written back as it came. */
    void testAReservedExtIsZero()
    {
        xdr::Reserved ext;
        QDataStream in(be32(1));
        in >> ext;
        QCOMPARE(in.status(), QDataStream::ReadCorruptData);
        ext.reserved = 1;
        QByteArray out;
        QDataStream s(&out, QIODevice::WriteOnly);
        s << ext;
        QCOMPARE(s.status(), QDataStream::WriteFailed);
        QVERIFY(out.isEmpty());
        ext.reserved = 0;
        QDataStream s2(&out, QIODevice::WriteOnly);
        s2 << ext;
        QCOMPARE(out, be32(0));
    }
};

ADD_TEST(XdrHelperTest)
#endif // XDRHELPERTEST_H
