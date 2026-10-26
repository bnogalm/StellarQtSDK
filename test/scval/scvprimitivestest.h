#ifndef SCVPRIMITIVESTEST_H
#define SCVPRIMITIVESTEST_H

#include <functional>
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

    /**
     * A value the SCVal cannot hold throws, as java's Scv does. qint64
     * 4294967296 passed to toUint32 was signed as u32 0 by C++'s implicit
     * narrowing, and -1 passed to toUint64 as 18446744073709551615.
     */
    void testIntegersAreRangeCheckedNotNarrowed()
    {
        auto throws = [](const std::function<void()>& f) {
            try { f(); return false; } catch (const std::out_of_range&) { return true; }
        };
        QVERIFY(throws([] { Scv::toUint32(qint64(4294967296LL)); }));
        QVERIFY(throws([] { Scv::toUint32(-1); }));
        QVERIFY(throws([] { Scv::toInt32(qint64(2147483648LL)); }));
        QVERIFY(throws([] { Scv::toInt32(3000000000LL); }));
        QVERIFY(throws([] { Scv::toUint64(qint64(-1)); }));
        QVERIFY(throws([] { Scv::toInt64(quint64(9223372036854775808ULL)); }));
        QVERIFY(throws([] { Scv::toTimePoint(-1); }));
        QVERIFY(throws([] { Scv::toDuration(qint64(-5)); }));

        QCOMPARE(Scv::fromUint32(Scv::toUint32(42)), quint32(42));
        QCOMPARE(Scv::fromUint32(Scv::toUint32(qint64(4294967295LL))), quint32(4294967295u));
        QCOMPARE(Scv::fromInt32(Scv::toInt32(qint64(-2147483647LL - 1))), qint32(-2147483647 - 1));
        QCOMPARE(Scv::fromUint64(Scv::toUint64(0)), quint64(0));
        QCOMPARE(Scv::fromInt64(Scv::toInt64(quint64(9223372036854775807ULL))),
                 qint64(9223372036854775807LL));
    }

    /**
     * String, symbol and bytes, against js-stellar-sdk 17.1.0 nativeToScVal
     * (py stellar_sdk 14.1.0 scval.to_string / to_symbol / to_bytes write the
     * same bytes): text goes out as its UTF-8, a symbol of the full 32 bytes
     * is accepted, and the same three bytes are a string or a bytes by the arm
     * alone. js's string XDR decodes back to the text js was given.
     */
    void testTextAndBytesMatchJsNativeToScVal()
    {
        auto hexOf = [](const stellar::SCVal& v) {
            QByteArray b;
            QDataStream s(&b, QIODevice::WriteOnly);
            s << v;
            return b.toHex();
        };
        // "h\u00e9llo \u4f60\u597d", spelled as UTF-8 so the source encoding does not matter.
        const QString text = QString::fromUtf8("h\xC3\xA9llo \xE4\xBD\xA0\xE5\xA5\xBD");
        QCOMPARE(hexOf(Scv::toString(text)), QByteArray("0000000e0000000d68c3a96c6c6f20e4bda0e5a5bd000000"));
        QCOMPARE(hexOf(Scv::toString(QString())), QByteArray("0000000e00000000"));
        QCOMPARE(hexOf(Scv::toSymbol(QStringLiteral("transfer"))), QByteArray("0000000f000000087472616e73666572"));
        QCOMPARE(hexOf(Scv::toSymbol(QStringLiteral("AAAAAAAAAAAAAAAA________z9z9z9z9"))),
                 QByteArray("0000000f00000020414141414141414141414141414141415f5f5f5f5f5f5f5f7a397a397a397a39"));
        QCOMPARE(hexOf(Scv::toBytes(QByteArray::fromHex("0001ff"))), QByteArray("0000000d000000030001ff00"));
        // nativeToScVal(Uint8Array [ff fe 41], { type: "string" }) keeps the
        // bytes that are not UTF-8, and without a type they are a bytes.
        QCOMPARE(hexOf(Scv::toString(QByteArray::fromHex("fffe41"))), QByteArray("0000000e00000003fffe4100"));
        QCOMPARE(hexOf(Scv::toBytes(QByteArray::fromHex("fffe41"))), QByteArray("0000000d00000003fffe4100"));

        stellar::SCVal decoded;
        const QByteArray jsString = QByteArray::fromHex("0000000e0000000d68c3a96c6c6f20e4bda0e5a5bd000000");
        QDataStream in(jsString);
        in >> decoded;
        QCOMPARE(in.status(), QDataStream::Ok);
        QCOMPARE(Scv::fromString(decoded), text);

        // js and py both sign "hello world" as a symbol (0000000f0000000b68656c6c
        // 6f20776f726c6400); the host refuses a symbol outside [a-zA-Z0-9_], so
        // here it is refused before it is signed. Core wins.
        bool threw = false;
        try { Scv::toSymbol(QStringLiteral("hello world")); } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
    }

    /** Text is signed as given or refused; a symbol is what the host takes. */
    void testTextIsWhatWasGiven()
    {
        auto threw = [](const std::function<void()>& f) {
            try { f(); return false; } catch (const std::exception&) { return true; }
        };
        QVERIFY(threw([] { Scv::toString(QStringLiteral("ab") + QChar(0xD800) + QStringLiteral("cd")); }));
        QVERIFY(threw([] { Scv::toSymbol(QStringLiteral("hello world")); }));
        QVERIFY(threw([] { Scv::toSymbol(QStringLiteral("caf\u00e9")); }));
        QVERIFY(threw([] { Scv::toUint128(QByteArray()); }));
        QCOMPARE(Scv::fromSymbol(Scv::toSymbol(QStringLiteral("transfer_1"))), QStringLiteral("transfer_1"));
        QCOMPARE(Scv::fromString(Scv::toString(QString::fromUtf8("\xF0\x9F\x92\xA9"))),
                 QString::fromUtf8("\xF0\x9F\x92\xA9"));
    }
};

ADD_TEST(ScvPrimitivesTest)
#endif // SCVPRIMITIVESTEST_H
