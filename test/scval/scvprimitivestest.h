#ifndef SCVPRIMITIVESTEST_H
#define SCVPRIMITIVESTEST_H

#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/scval/scv.h"

/**
 * SCVal primitive helpers (CAP-46) — round-trips for Bool / Void / U32 / I32 /
 * U64 / I64 / TimePoint / Duration / Bytes / String / Symbol, plus XDR
 * serialization round-trips for the same.
 */
class ScvPrimitivesTest: public QObject
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

    // ─── Void / Bool ─────────────────────────────────────────────────

    void testVoid()
    {
        stellar::SCVal v = Scv::toVoid();
        QCOMPARE(static_cast<int>(v.type),
                 static_cast<int>(stellar::SCValType::SCV_VOID));
        stellar::SCVal back = roundtripXdr(v);
        QCOMPARE(static_cast<int>(back.type),
                 static_cast<int>(stellar::SCValType::SCV_VOID));
    }

    void testBoolean()
    {
        QCOMPARE(Scv::fromBoolean(Scv::toBoolean(true)),  true);
        QCOMPARE(Scv::fromBoolean(Scv::toBoolean(false)), false);
        QCOMPARE(Scv::fromBoolean(roundtripXdr(Scv::toBoolean(true))), true);
    }

    // ─── Fixed-width integers ────────────────────────────────────────

    void testUint32()
    {
        QCOMPARE(Scv::fromUint32(Scv::toUint32(0)), quint32(0));
        QCOMPARE(Scv::fromUint32(Scv::toUint32(0xFFFFFFFFu)), 0xFFFFFFFFu);
        QCOMPARE(Scv::fromUint32(roundtripXdr(Scv::toUint32(123456789u))),
                 quint32(123456789u));
    }

    void testInt32()
    {
        QCOMPARE(Scv::fromInt32(Scv::toInt32(0)), qint32(0));
        QCOMPARE(Scv::fromInt32(Scv::toInt32(-1)), qint32(-1));
        QCOMPARE(Scv::fromInt32(Scv::toInt32(0x7FFFFFFF)),  qint32(0x7FFFFFFF));
        QCOMPARE(Scv::fromInt32(Scv::toInt32(-0x80000000LL)), qint32(-0x80000000LL));
        QCOMPARE(Scv::fromInt32(roundtripXdr(Scv::toInt32(-42))), qint32(-42));
    }

    void testUint64()
    {
        QCOMPARE(Scv::fromUint64(Scv::toUint64(0xFFFFFFFFFFFFFFFFULL)),
                 0xFFFFFFFFFFFFFFFFULL);
        QCOMPARE(Scv::fromUint64(roundtripXdr(Scv::toUint64(0xDEADBEEFCAFE1234ULL))),
                 0xDEADBEEFCAFE1234ULL);
    }

    void testInt64()
    {
        QCOMPARE(Scv::fromInt64(Scv::toInt64(0)), qint64(0));
        QCOMPARE(Scv::fromInt64(Scv::toInt64(-1)), qint64(-1));
        QCOMPARE(Scv::fromInt64(roundtripXdr(Scv::toInt64(0x7FFFFFFFFFFFFFFFLL))),
                 qint64(0x7FFFFFFFFFFFFFFFLL));
    }

    void testTimePoint()
    {
        QCOMPARE(Scv::fromTimePoint(roundtripXdr(Scv::toTimePoint(1234567890ULL))),
                 quint64(1234567890ULL));
    }

    void testDuration()
    {
        QCOMPARE(Scv::fromDuration(roundtripXdr(Scv::toDuration(60ULL))),
                 quint64(60ULL));
    }

    // ─── Variable-length byte payloads ───────────────────────────────

    void testBytes()
    {
        QByteArray data;
        for (int i = 0; i < 5; ++i) data.append(static_cast<char>(i + 1));
        QCOMPARE(Scv::fromBytes(roundtripXdr(Scv::toBytes(data))), data);

        QCOMPARE(Scv::fromBytes(roundtripXdr(Scv::toBytes(QByteArray()))),
                 QByteArray());
    }

    void testBytesXdrLayout()
    {
        QByteArray data("ABC");  // 3 bytes → 1 byte padding
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << Scv::toBytes(data); }
        // 4 type + 4 length + 3 data + 1 pad = 12 bytes
        QCOMPARE(bytes.size(), 12);
    }

    void testString()
    {
        QString s = QStringLiteral("hello stellar 你好");
        QCOMPARE(Scv::fromString(roundtripXdr(Scv::toString(s))), s);
    }

    void testSymbol()
    {
        QString sym = QStringLiteral("transfer");
        QCOMPARE(Scv::fromSymbol(roundtripXdr(Scv::toSymbol(sym))), sym);
    }

    void testSymbolRejectsOverflow()
    {
        bool threw = false;
        try {
            Scv::toSymbol(QString(33, 'a'));
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ─── Type-mismatch detection ─────────────────────────────────────

    void testFromMismatchedTypeThrows()
    {
        stellar::SCVal v = Scv::toUint32(1);
        bool threw = false;
        try { Scv::fromInt32(v); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }
};

ADD_TEST(ScvPrimitivesTest)
#endif // SCVPRIMITIVESTEST_H
