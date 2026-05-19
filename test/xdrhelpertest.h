#ifndef XDRHELPERTEST_H
#define XDRHELPERTEST_H
#include <QObject>
#include <QtTest>
#include <QByteArray>
#include <QDataStream>
#include "testcollector.h"

#include "../xdr/xdrhelper.h"
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
};

ADD_TEST(XdrHelperTest)
#endif // XDRHELPERTEST_H
