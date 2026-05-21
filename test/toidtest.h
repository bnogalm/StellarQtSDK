#ifndef TOIDTEST_H
#define TOIDTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/toid.h"

/** Tests for SEP-35 TOID packing. */
class TOIDTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testEncodeDecodeRoundTrip()
    {
        TOID original(0xDEADBEEFu, 0x12345u, 0xABCu);
        qint64 packed = original.toInt64();

        TOID decoded = TOID::fromInt64(packed);
        QCOMPARE(decoded.getLedger(),  original.getLedger());
        QCOMPARE(decoded.getTxIndex(), original.getTxIndex());
        QCOMPARE(decoded.getOpIndex(), original.getOpIndex());
        QVERIFY(decoded == original);
    }

    void testBitLayout()
    {
        // Ledger=1, txIndex=2, opIndex=3 → 0x0000_0001 0000_2003
        TOID t(1, 2, 3);
        qint64 packed = t.toInt64();
        QCOMPARE(packed, qint64(0x0000000100002003LL));

        TOID t2 = TOID::fromInt64(0x0000000100002003LL);
        QCOMPARE(t2.getLedger(),  quint32(1));
        QCOMPARE(t2.getTxIndex(), quint32(2));
        QCOMPARE(t2.getOpIndex(), quint32(3));
    }

    void testMaxValues()
    {
        TOID t(0xFFFFFFFFu, TOID::MAX_TX_INDEX, TOID::MAX_OP_INDEX);
        // 0xFFFFFFFF_FFFFFFFF — interpreted as signed qint64 → -1
        QCOMPARE(t.toInt64(), qint64(-1));

        TOID back = TOID::fromInt64(qint64(-1));
        QCOMPARE(back.getLedger(),  0xFFFFFFFFu);
        QCOMPARE(back.getTxIndex(), TOID::MAX_TX_INDEX);
        QCOMPARE(back.getOpIndex(), TOID::MAX_OP_INDEX);
    }

    void testRejectsOversizedTxIndex()
    {
        bool threw = false;
        try { TOID t(1, TOID::MAX_TX_INDEX + 1, 0); Q_UNUSED(t); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    void testRejectsOversizedOpIndex()
    {
        bool threw = false;
        try { TOID t(1, 0, TOID::MAX_OP_INDEX + 1); Q_UNUSED(t); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    void testZeroToid()
    {
        QCOMPARE(TOID().toInt64(), qint64(0));
        TOID t = TOID::fromInt64(0);
        QCOMPARE(t.getLedger(),  quint32(0));
        QCOMPARE(t.getTxIndex(), quint32(0));
        QCOMPARE(t.getOpIndex(), quint32(0));
    }
};

ADD_TEST(TOIDTest)
#endif // TOIDTEST_H
