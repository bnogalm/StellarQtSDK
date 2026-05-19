#ifndef TRANSACTIONPRECONDITIONSTEST_H
#define TRANSACTIONPRECONDITIONSTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/transactionpreconditions.h"
#include "../src/ledgerbounds.h"
#include "../src/timebounds.h"
#include "../src/signerkey.h"

/** Tests for CAP-21 TransactionPreconditions + LedgerBounds + Preconditions XDR. */
class TransactionPreconditionsTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    // ─── Variant selection ───────────────────────────────────────────────

    void testEmptyPreconditionsEmitsNone()
    {
        TransactionPreconditions pre;
        QVERIFY(!pre.hasV2());

        stellar::Preconditions xdr = pre.toXdr();
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::PreconditionType::PRECOND_NONE));
    }

    void testOnlyTimeBoundsEmitsPrecondTime()
    {
        TransactionPreconditions pre;
        pre.setTimeBounds(new TimeBounds(100, 200));
        QVERIFY(!pre.hasV2());

        stellar::Preconditions xdr = pre.toXdr();
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::PreconditionType::PRECOND_TIME));
        QCOMPARE(xdr.timeBounds.minTime, quint64(100));
        QCOMPARE(xdr.timeBounds.maxTime, quint64(200));
    }

    void testLedgerBoundsForcesV2()
    {
        TransactionPreconditions pre;
        pre.setLedgerBounds(new LedgerBounds(10, 20));
        QVERIFY(pre.hasV2());

        stellar::Preconditions xdr = pre.toXdr();
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::PreconditionType::PRECOND_V2));
        QVERIFY(xdr.v2.ledgerBounds.filled);
        QCOMPARE(xdr.v2.ledgerBounds.value.minLedger, quint32(10));
        QCOMPARE(xdr.v2.ledgerBounds.value.maxLedger, quint32(20));
        QVERIFY(!xdr.v2.timeBounds.filled);
    }

    void testMinSeqFieldsForceV2()
    {
        TransactionPreconditions pre;
        pre.setMinSeqNumber(new qint64(42));
        pre.setMinSeqAge(99);
        pre.setMinSeqLedgerGap(7);
        QVERIFY(pre.hasV2());

        stellar::Preconditions xdr = pre.toXdr();
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::PreconditionType::PRECOND_V2));
        QVERIFY(xdr.v2.minSeqNum.filled);
        QCOMPARE(xdr.v2.minSeqNum.value, qint64(42));
        QCOMPARE(xdr.v2.minSeqAge, quint64(99));
        QCOMPARE(xdr.v2.minSeqLedgerGap, quint32(7));
    }

    void testExtraSignersForceV2()
    {
        TransactionPreconditions pre;
        pre.addExtraSigner(SignerKey::ed25519PublicKey(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        QVERIFY(pre.hasV2());

        stellar::Preconditions xdr = pre.toXdr();
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::PreconditionType::PRECOND_V2));
        QCOMPARE(xdr.v2.extraSigners.value.size(), 1);
    }

    void testRejectsMoreThanTwoExtraSigners()
    {
        TransactionPreconditions pre;
        pre.addExtraSigner(SignerKey::sha256Hash(QByteArray(32, '\x01')));
        pre.addExtraSigner(SignerKey::sha256Hash(QByteArray(32, '\x02')));
        bool threw = false;
        try {
            pre.addExtraSigner(SignerKey::sha256Hash(QByteArray(32, '\x03')));
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ─── XDR round-trip ──────────────────────────────────────────────────

    void testV2XdrRoundTrip()
    {
        TransactionPreconditions pre;
        pre.setTimeBounds(new TimeBounds(1000, 2000));
        pre.setLedgerBounds(new LedgerBounds(50, 100));
        pre.setMinSeqNumber(new qint64(123));
        pre.setMinSeqAge(60);
        pre.setMinSeqLedgerGap(3);
        pre.addExtraSigner(SignerKey::ed25519PublicKey(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        pre.addExtraSigner(SignerKey::ed25519SignedPayload(
            "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG",
            QByteArray(10, '\x5A')));

        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << pre.toXdr(); }
        stellar::Preconditions parsed;
        { QDataStream s(&bytes, QIODevice::ReadOnly); s >> parsed; }

        TransactionPreconditions back = TransactionPreconditions::fromXdr(parsed);
        QVERIFY(back.hasV2());
        QVERIFY(back.getTimeBounds() != nullptr);
        QCOMPARE(back.getTimeBounds()->getMinTime(), qint64(1000));
        QCOMPARE(back.getTimeBounds()->getMaxTime(), qint64(2000));
        QVERIFY(back.getLedgerBounds() != nullptr);
        QCOMPARE(back.getLedgerBounds()->getMinLedger(), quint32(50));
        QCOMPARE(back.getLedgerBounds()->getMaxLedger(), quint32(100));
        QVERIFY(back.getMinSeqNumber() != nullptr);
        QCOMPARE(*back.getMinSeqNumber(), qint64(123));
        QCOMPARE(back.getMinSeqAge(), quint64(60));
        QCOMPARE(back.getMinSeqLedgerGap(), quint32(3));
        QCOMPARE(back.getExtraSigners().size(), 2);
        QCOMPARE(static_cast<int>(back.getExtraSigners().at(0).getType()),
                 static_cast<int>(stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519));
        QCOMPARE(static_cast<int>(back.getExtraSigners().at(1).getType()),
                 static_cast<int>(stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD));
    }

    /** Wire format compat: PRECOND_NONE has the same 4-byte encoding as Optional<TimeBounds>{filled=0}. */
    void testPrecondNoneWireFormat()
    {
        TransactionPreconditions pre;
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << pre.toXdr(); }
        QCOMPARE(bytes.size(), 4);
        QCOMPARE(bytes, QByteArray("\x00\x00\x00\x00", 4));
    }

    /** Wire format compat: PRECOND_TIME with TB equals Optional{filled=1, TimeBounds}. */
    void testPrecondTimeWireFormat()
    {
        TransactionPreconditions pre;
        pre.setTimeBounds(new TimeBounds(0x0102030405060708LL, 0x1112131415161718LL));
        QByteArray bytes;
        { QDataStream s(&bytes, QIODevice::WriteOnly); s << pre.toXdr(); }
        // 4 bytes discriminant (=1) + 16 bytes TimeBounds (BE int64 x2) = 20 bytes
        QCOMPARE(bytes.size(), 20);

        QByteArray expected;
        expected.append('\0').append('\0').append('\0').append('\x01'); // PRECOND_TIME = 1
        expected.append('\x01').append('\x02').append('\x03').append('\x04')
                .append('\x05').append('\x06').append('\x07').append('\x08');
        expected.append('\x11').append('\x12').append('\x13').append('\x14')
                .append('\x15').append('\x16').append('\x17').append('\x18');
        QCOMPARE(bytes.toHex(), expected.toHex());
    }
};

ADD_TEST(TransactionPreconditionsTest)
#endif // TRANSACTIONPRECONDITIONSTEST_H
