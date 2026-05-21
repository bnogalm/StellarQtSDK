#ifndef SCVBIGINTEGERSTEST_H
#define SCVBIGINTEGERSTEST_H

#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/scval/scv.h"

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
};

ADD_TEST(ScvBigIntegersTest)
#endif // SCVBIGINTEGERSTEST_H
