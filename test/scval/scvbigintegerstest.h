#ifndef SCVBIGINTEGERSTEST_H
#define SCVBIGINTEGERSTEST_H

#include <functional>
#include <type_traits>
#include <utility>
#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/scval/scv.h"

namespace scvbigintegerstest {
// Whether a call compiles, so "a double does not compile" can be asserted.
enum Plain { PlainOne = 1 };
template <typename T, typename = void> struct TakesInt128Parts : std::false_type {};
template <typename T>
struct TakesInt128Parts<T, decltype(void(Scv::toInt128Parts(std::declval<T>(), std::declval<T>())))>
    : std::true_type {};
template <typename T, typename = void> struct TakesUint128Parts : std::false_type {};
template <typename T>
struct TakesUint128Parts<T, decltype(void(Scv::toUint128Parts(quint64(0), std::declval<T>())))>
    : std::true_type {};
template <typename T, typename = void> struct TakesNonce : std::false_type {};
template <typename T>
struct TakesNonce<T, decltype(void(Scv::toLedgerKeyNonce(std::declval<T>())))> : std::true_type {};
template <typename T, typename = void> struct TakesInt128 : std::false_type {};
template <typename T>
struct TakesInt128<T, decltype(void(Scv::toInt128(std::declval<T>())))> : std::true_type {};
template <typename T, typename = void> struct TakesUint128 : std::false_type {};
template <typename T>
struct TakesUint128<T, decltype(void(Scv::toUint128(std::declval<T>())))> : std::true_type {};
}

/**
 * SCVal 128 / 256-bit integer helpers (CAP-46). Round-trips for Uint128 /
 * Int128 / Uint256 / Int256 via both QByteArray (big-endian) and the
 * native-parts ctors. XDR layout sanity (16 / 32 bytes payload).
 */
class ScvBigIntegersTest: public QObject
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

    static QByteArray hex(const char* h) { return QByteArray::fromHex(h); }

    // ─── Uint128 ─────────────────────────────────────────────────────

    void testUint128RoundTripZero()
    {
        QCOMPARE(Scv::fromUint128(roundtripXdr(Scv::toUint128(hex("00000000000000000000000000000000")))),
                 hex("00000000000000000000000000000000"));
    }

    void testUint128RoundTripMax()
    {
        QCOMPARE(Scv::fromUint128(roundtripXdr(Scv::toUint128(hex("ffffffffffffffffffffffffffffffff")))),
                 hex("ffffffffffffffffffffffffffffffff"));
    }

    void testUint128SpecificValue()
    {
        // 0x0123456789ABCDEF_FEDCBA9876543210
        QByteArray src = hex("0123456789abcdeffedcba9876543210");
        stellar::SCVal v = Scv::toUint128(src);
        QCOMPARE(v.u128.hi, 0x0123456789ABCDEFULL);
        QCOMPARE(v.u128.lo, 0xFEDCBA9876543210ULL);
        QCOMPARE(Scv::fromUint128(v), src);
    }

    void testUint128PartsCtor()
    {
        stellar::SCVal v = Scv::toUint128Parts(0xAAULL, 0xBBULL);
        QCOMPARE(Scv::fromUint128(v),
                 hex("00000000000000aa00000000000000bb"));
    }

    void testUint128ShortInputPaddedLeft()
    {
        // 1 byte input → padded to 16 zero bytes + value.
        QCOMPARE(Scv::fromUint128(Scv::toUint128(QByteArray(1, '\x42'))),
                 hex("00000000000000000000000000000042"));
    }

    void testUint128XdrLayoutIsSixteenBytes()
    {
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly);
          s << Scv::toUint128(hex("00112233445566778899aabbccddeeff")); }
        // 4 type + 16 payload = 20 bytes
        QCOMPARE(bytes.size(), 20);
    }

    // ─── Int128 ──────────────────────────────────────────────────────

    void testInt128RoundTripNegative()
    {
        // -1 in 128-bit two's complement = all ones.
        stellar::SCVal v = Scv::toInt128Parts(qint64(-1), 0xFFFFFFFFFFFFFFFFULL);
        QCOMPARE(Scv::fromInt128(v), hex("ffffffffffffffffffffffffffffffff"));
        QCOMPARE(v.i128.hi, qint64(-1));
        QCOMPARE(v.i128.lo, 0xFFFFFFFFFFFFFFFFULL);
    }

    void testInt128ShortInputSignExtends()
    {
        // 1 byte input 0xFF (signed = -1) → all-ones 16-byte payload.
        QByteArray src(1, '\xFF');
        QCOMPARE(Scv::fromInt128(Scv::toInt128(src)),
                 hex("ffffffffffffffffffffffffffffffff"));
    }

    void testInt128RoundTripPositive()
    {
        stellar::SCVal v = Scv::toInt128Parts(qint64(1), 0);
        QCOMPARE(Scv::fromInt128(roundtripXdr(v)),
                 hex("00000000000000010000000000000000"));
    }

    // ─── Uint256 ─────────────────────────────────────────────────────

    void testUint256RoundTripZero()
    {
        QByteArray zero(32, '\0');
        QCOMPARE(Scv::fromUint256(roundtripXdr(Scv::toUint256(zero))), zero);
    }

    void testUint256RoundTripMax()
    {
        QByteArray max = hex("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        QCOMPARE(Scv::fromUint256(roundtripXdr(Scv::toUint256(max))), max);
    }

    void testUint256XdrLayoutIsThirtyTwoBytes()
    {
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly);
          s << Scv::toUint256(QByteArray(32, '\x55')); }
        // 4 type + 32 payload = 36 bytes
        QCOMPARE(bytes.size(), 36);
    }

    // ─── Int256 ──────────────────────────────────────────────────────

    void testInt256NegativeOneSignExtends()
    {
        QByteArray src(1, '\xFF');
        QCOMPARE(Scv::fromInt256(roundtripXdr(Scv::toInt256(src))),
                 hex("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
    }

    void testInt256ZeroAndPositive()
    {
        QCOMPARE(Scv::fromInt256(roundtripXdr(Scv::toInt256(QByteArray(32, '\0')))),
                 QByteArray(32, '\0'));

        QByteArray src = hex("0000000000000000000000000000000000000000000000000000000000000042");
        QCOMPARE(Scv::fromInt256(roundtripXdr(Scv::toInt256(src))), src);
    }

    // ─── Padding edge cases ──────────────────────────────────────────

    void testRejectsTooManyBytesWithNonZeroPrefix()
    {
        // 17 unsigned bytes with a non-zero leading byte → overflow.
        bool threw = false;
        try {
            Scv::toUint128(hex("01" "00000000000000000000000000000042"));
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void testAcceptsLeadingZeroForUnsigned()
    {
        // 17 unsigned bytes with leading 0x00 → trim and proceed.
        QByteArray src = hex("00" "00000000000000000000000000000042");
        QCOMPARE(Scv::fromUint128(Scv::toUint128(src)),
                 hex("00000000000000000000000000000042"));
    }

    // ─── Checked factories ───────────────────────────────────────────

    /**
     * The 128-bit Parts and nonce factories are range-checked like toUint32.
     * They took plain qint64/quint64 words, so C++ converted in silence:
     * toInt128Parts(0, qint64(-100)) was signed as +18446744073709551516, a
     * double 1.9 as 1, 19.99 * 1e7 tokens one base unit short, and
     * toLedgerKeyNonce(quint64 2^63) as INT64_MIN.
     */
    void testWideFactoriesAreRangeCheckedNotNarrowed()
    {
        auto throws = [](const std::function<void()>& f) {
            try { f(); return false; } catch (const std::out_of_range&) { return true; }
        };
        QVERIFY(throws([] { Scv::toInt128Parts(0, qint64(-100)); }));
        QVERIFY(throws([] { Scv::toInt128Parts(qint64(-1), -1); }));   // raw bits need a quint64
        QVERIFY(throws([] { Scv::toInt128Parts(quint64(0xFFFFFFFFFFFFFFFFULL), 0); }));
        QVERIFY(throws([] { Scv::toUint128Parts(qint64(-1), 0); }));
        QVERIFY(throws([] { Scv::toUint128Parts(0, -1); }));
        QVERIFY(throws([] { Scv::toLedgerKeyNonce(quint64(9223372036854775808ULL)); }));
        QVERIFY(throws([] { Scv::toUint128(-1); }));
        QVERIFY(throws([] { Scv::toUint128(qint64(-5)); }));

        // What fits is taken, from any integer type.
        QCOMPARE(Scv::fromInt128(Scv::toInt128Parts(-1, 0xFFFFFFFFFFFFFFFFULL)),
                 hex("ffffffffffffffffffffffffffffffff"));
        QCOMPARE(Scv::fromUint128(Scv::toUint128Parts(1, 2u)),
                 hex("00000000000000010000000000000002"));
        QCOMPARE(Scv::fromLedgerKeyNonce(Scv::toLedgerKeyNonce(quint64(9223372036854775807ULL))),
                 qint64(9223372036854775807LL));
        QCOMPARE(Scv::fromLedgerKeyNonce(Scv::toLedgerKeyNonce(-1)), qint64(-1));

        // Floating point, bool and enums do not compile at all.
        using namespace scvbigintegerstest;
        QVERIFY(TakesInt128Parts<int>::value);
        QVERIFY(TakesInt128Parts<qint64>::value);
        QVERIFY(!TakesInt128Parts<double>::value);
        QVERIFY(!TakesInt128Parts<float>::value);
        QVERIFY(!TakesInt128Parts<bool>::value);
        QVERIFY(!TakesInt128Parts<Plain>::value);
        QVERIFY(TakesUint128Parts<unsigned>::value);
        QVERIFY(!TakesUint128Parts<double>::value);
        QVERIFY(!TakesUint128Parts<bool>::value);
        QVERIFY(TakesNonce<qint64>::value);
        QVERIFY(!TakesNonce<double>::value);
        QVERIFY(!TakesNonce<bool>::value);
        QVERIFY(!TakesNonce<Plain>::value);
        QVERIFY(TakesInt128<int>::value);
        QVERIFY(TakesInt128<QByteArray>::value);
        QVERIFY(!TakesInt128<double>::value);
        QVERIFY(!TakesInt128<bool>::value);
        QVERIFY(TakesUint128<quint64>::value);
        QVERIFY(TakesUint128<QByteArray>::value);
        QVERIFY(!TakesUint128<double>::value);
        QVERIFY(!TakesUint128<Plain>::value);
    }

    /**
     * toInt128(n) / toUint128(n) build a token amount from a 64-bit integer,
     * sign-extended: -100 gives js nativeToScVal(-100, {type: 'i128'})'s
     * bytes. There was no such factory, and building the amount from raw
     * parts, toInt128Parts(0, -100), signed 2^64 - 100.
     */
    void testValueFactoriesSignExtend()
    {
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << Scv::toInt128(qint64(-100)); }
        QCOMPARE(bytes.toHex(), QByteArray("0000000affffffffffffffffffffffffffffff9c"));
        QCOMPARE(Scv::fromInt128(Scv::toInt128(-1)), hex("ffffffffffffffffffffffffffffffff"));
        QCOMPARE(Scv::fromInt128(Scv::toInt128(qint64(-9223372036854775807LL - 1))),
                 hex("ffffffffffffffff8000000000000000"));
        // Every u64 fits an i128, including those above 2^63 - 1.
        QCOMPARE(Scv::fromInt128(Scv::toInt128(quint64(0xFFFFFFFFFFFFFFFFULL))),
                 hex("0000000000000000ffffffffffffffff"));
        QCOMPARE(Scv::fromUint128(Scv::toUint128(quint64(0xFFFFFFFFFFFFFFFFULL))),
                 hex("0000000000000000ffffffffffffffff"));
        QCOMPARE(Scv::fromUint128(Scv::toUint128(7)), hex("00000000000000000000000000000007"));
        // The big-endian byte forms are unchanged beside them.
        QCOMPARE(Scv::fromInt128(Scv::toInt128(hex("ff9c"))), Scv::fromInt128(Scv::toInt128(-100)));
        QCOMPARE(Scv::fromUint128(Scv::toUint128(hex("07"))), Scv::fromUint128(Scv::toUint128(7)));
    }

    /**
     * The wide integers at their edges, against js-stellar-sdk 17.1.0
     * nativeToScVal(BigInt, { type }) and scValToNative (py stellar_sdk 14.1.0
     * scval.to_int128 / to_uint128 / to_int256 / to_uint256 write the same
     * bytes). The shortest two's-complement big-endian input sign- or
     * zero-extends to js's XDR; js's XDR decodes to js's decimal; and one past
     * either end, which js ("out of range") and py ("Invalid data, must be
     * between ...") both refuse, throws.
     */
    void testWideIntegerEdgesMatchJsNativeToScVal()
    {
        struct Row { const char* type; const char* minimalBigEndian; const char* jsXdr; const char* decimal; };
        const Row rows[] = {
            { "i128", "80000000000000000000000000000000",
              "0000000a80000000000000000000000000000000",
              "-170141183460469231731687303715884105728" },
            { "i128", "7fffffffffffffffffffffffffffffff",
              "0000000a7fffffffffffffffffffffffffffffff",
              "170141183460469231731687303715884105727" },
            { "i128", "ff",
              "0000000affffffffffffffffffffffffffffffff",
              "-1" },
            { "i128", "ff0000000000000000",
              "0000000affffffffffffffff0000000000000000",
              "-18446744073709551616" },
            { "i128", "010000000000000000",
              "0000000a00000000000000010000000000000000",
              "18446744073709551616" },
            { "i128", "ff7fffffffffffffff",
              "0000000affffffffffffffff7fffffffffffffff",
              "-9223372036854775809" },
            { "u128", "ffffffffffffffffffffffffffffffff",
              "00000009ffffffffffffffffffffffffffffffff",
              "340282366920938463463374607431768211455" },
            { "u128", "010000000000000000",
              "0000000900000000000000010000000000000000",
              "18446744073709551616" },
            { "u128", "00",
              "0000000900000000000000000000000000000000",
              "0" },
            { "i256", "8000000000000000000000000000000000000000000000000000000000000000",
              "0000000c8000000000000000000000000000000000000000000000000000000000000000",
              "-57896044618658097711785492504343953926634992332820282019728792003956564819968" },
            { "i256", "7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
              "0000000c7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
              "57896044618658097711785492504343953926634992332820282019728792003956564819967" },
            { "i256", "ff",
              "0000000cffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
              "-1" },
            { "i256", "ff00000000000000000000000000000000",
              "0000000cffffffffffffffffffffffffffffffff00000000000000000000000000000000",
              "-340282366920938463463374607431768211456" },
            { "i256", "01000000000000000000000000000000000000000000000001",
              "0000000c0000000000000001000000000000000000000000000000000000000000000001",
              "6277101735386680763835789423207666416102355444464034512897" },
            { "u256", "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
              "0000000bffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
              "115792089237316195423570985008687907853269984665640564039457584007913129639935" },
            { "u256", "8000000000000000000000000000000000000000000000000000000000000000",
              "0000000b8000000000000000000000000000000000000000000000000000000000000000",
              "57896044618658097711785492504343953926634992332820282019728792003956564819968" },
            { "u256", "0100000000000000000000000000000000",
              "0000000b0000000000000000000000000000000100000000000000000000000000000000",
              "340282366920938463463374607431768211456" },
        };
        for (const Row& row : rows) {
            const QByteArray type(row.type);
            const QByteArray be = QByteArray::fromHex(row.minimalBigEndian);
            stellar::SCVal built;
            if (type == "i128") built = Scv::toInt128(be);
            else if (type == "u128") built = Scv::toUint128(be);
            else if (type == "i256") built = Scv::toInt256(be);
            else built = Scv::toUint256(be);
            QByteArray written;
            { QDataStream s(&written, QIODevice::WriteOnly); s << built; }
            QCOMPARE(written.toHex(), QByteArray(row.jsXdr));

            const QByteArray jsBytes = QByteArray::fromHex(row.jsXdr);
            stellar::SCVal decoded;
            QDataStream in(jsBytes);
            in >> decoded;
            QCOMPARE(in.status(), QDataStream::Ok);
            QVERIFY(in.atEnd());
            QCOMPARE(Scv::decimalText(decoded), QString::fromLatin1(row.decimal));
            QCOMPARE(Scv::compare(decoded, built), 0);
        }

        // The word factories write the same bytes for the values that straddle
        // the 64-bit boundary.
        auto hexOf = [](const stellar::SCVal& v) {
            QByteArray b;
            QDataStream s(&b, QIODevice::WriteOnly);
            s << v;
            return b.toHex();
        };
        QCOMPARE(hexOf(Scv::toInt128Parts(qint64(-1), quint64(0))),
                 QByteArray("0000000affffffffffffffff0000000000000000"));            // -2^64
        QCOMPARE(hexOf(Scv::toInt128Parts(qint64(-1), quint64(0x7fffffffffffffffULL))),
                 QByteArray("0000000affffffffffffffff7fffffffffffffff"));            // -2^63 - 1
        QCOMPARE(hexOf(Scv::toInt128Parts(qint64(1), quint64(0))),
                 QByteArray("0000000a00000000000000010000000000000000"));            // 2^64
        QCOMPARE(hexOf(Scv::toUint128Parts(quint64(1), quint64(0))),
                 QByteArray("0000000900000000000000010000000000000000"));            // 2^64
        QCOMPARE(hexOf(Scv::toInt128(qint64(-1))),
                 QByteArray("0000000affffffffffffffffffffffffffffffff"));

        const char* const refused[][2] = {
            { "i128", "0080000000000000000000000000000000" },   // 2^127
            { "i128", "ff7fffffffffffffffffffffffffffffff" },   // -2^127 - 1
            { "u128", "0100000000000000000000000000000000" },   // 2^128
            { "i256", "008000000000000000000000000000000000000000000000000000000000000000" },   // 2^255
            { "i256", "ff7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff" },   // -2^255 - 1
            { "u256", "010000000000000000000000000000000000000000000000000000000000000000" },   // 2^256
        };
        for (const auto& r : refused) {
            const QByteArray type(r[0]);
            const QByteArray be = QByteArray::fromHex(r[1]);
            bool threw = false;
            try {
                if (type == "i128") Scv::toInt128(be);
                else if (type == "u128") Scv::toUint128(be);
                else if (type == "i256") Scv::toInt256(be);
                else Scv::toUint256(be);
            } catch (const std::runtime_error&) {
                threw = true;
            }
            QVERIFY2(threw, r[1]);
        }
    }

    /** The exact decimal text of a wide integer, which the QML layer shows: a
     *  token balance used to come back as "(type 10)". No double anywhere. */
    void testDecimalTextIsExact()
    {
        QCOMPARE(Scv::decimalText(Scv::toInt128(qint64(100000000000LL))), QStringLiteral("100000000000"));
        QCOMPARE(Scv::decimalText(Scv::toInt128(-1)), QStringLiteral("-1"));
        QCOMPARE(Scv::decimalText(Scv::toInt128Parts(qint64(-9223372036854775807LL - 1), quint64(0))),
                 QStringLiteral("-170141183460469231731687303715884105728"));
        QCOMPARE(Scv::decimalText(Scv::toInt128Parts(qint64(9223372036854775807LL), 0xFFFFFFFFFFFFFFFFULL)),
                 QStringLiteral("170141183460469231731687303715884105727"));
        QCOMPARE(Scv::decimalText(Scv::toUint128Parts(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL)),
                 QStringLiteral("340282366920938463463374607431768211455"));
        QCOMPARE(Scv::decimalText(Scv::toUint256(QByteArray(32, '\xff'))),
                 QStringLiteral("115792089237316195423570985008687907853269984665640564039457584007913129639935"));
        QCOMPARE(Scv::decimalText(Scv::toInt256(QByteArray(1, '\xfb'))), QStringLiteral("-5"));
        QCOMPARE(Scv::decimalText(Scv::toInt256(QByteArray(1, '\x80') + QByteArray(31, '\0'))),
                 QStringLiteral("-57896044618658097711785492504343953926634992332820282019728792003956564819968"));
        QCOMPARE(Scv::decimalText(Scv::toUint128(quint64(0))), QStringLiteral("0"));
        QCOMPARE(Scv::decimalText(Scv::toInt256(QByteArray(1, '\0'))), QStringLiteral("0"));
        // Around the 10^9 chunks the conversion works in.
        QCOMPARE(Scv::decimalText(Scv::toUint128(quint64(999999999))), QStringLiteral("999999999"));
        QCOMPARE(Scv::decimalText(Scv::toUint128(quint64(1000000000))), QStringLiteral("1000000000"));
        QCOMPARE(Scv::decimalText(Scv::toInt128(qint64(-1000000000000000000LL))),
                 QStringLiteral("-1000000000000000000"));
        QCOMPARE(Scv::decimalText(Scv::toUint128Parts(quint64(1), quint64(0))),
                 QStringLiteral("18446744073709551616"));
        // Anything else is refused rather than guessed at.
        bool threw = false;
        try { Scv::decimalText(Scv::toUint64(quint64(5))); } catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
    }
};

ADD_TEST(ScvBigIntegersTest)
#endif // SCVBIGINTEGERSTEST_H
