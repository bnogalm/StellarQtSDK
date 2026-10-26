#ifndef TOIDTEST_H
#define TOIDTEST_H

#include <QObject>
#include <QtTest>
#include <limits>
#include <QSet>
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
        TOID original(0x5EADBEEFu, 0x12345u, 0xABCu);   // a ledger below 2^31, as every real one
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
        // The largest TOID: ledger 2^31 - 1, every index at its maximum, which
        // is INT64_MAX. A ledger past it set the sign bit and encoded -1 for
        // 0xFFFFFFFF; it is refused now, as stellar/go refuses it.
        TOID t(TOID::MAX_LEDGER, TOID::MAX_TX_INDEX, TOID::MAX_OP_INDEX);
        QCOMPARE(t.toInt64(), std::numeric_limits<qint64>::max());
        QVERIFY_EXCEPTION_THROWN(TOID(TOID::MAX_LEDGER + 1u, 0, 0), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(TOID(0xFFFFFFFFu, TOID::MAX_TX_INDEX, TOID::MAX_OP_INDEX), std::runtime_error);

        // Decoding still takes any 64 bits.
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

    /**
     * A real id: operation 21623064391401473 on testnet Horizon (its only
     * operation in transaction 21623064391401472, ledger 5034512, whose own
     * paging token is 21623064391319552), read from Horizon on 2026-10-05.
     */
    void testARealHorizonId()
    {
        const TOID op = TOID::fromInt64(Q_INT64_C(21623064391401473));
        QCOMPARE(op.getLedger(), 5034512u);
        QCOMPARE(op.getTxIndex(), 20u);
        QCOMPARE(op.getOpIndex(), 1u);
        QCOMPARE(TOID(5034512u, 20u, 0u).toInt64(), Q_INT64_C(21623064391401472));   // its transaction
        QCOMPARE(TOID(5034512u, 0u, 0u).toInt64(), Q_INT64_C(21623064391319552));    // its ledger
        QCOMPARE(TOID(5034512u, 20u, 1u).toInt64(), Q_INT64_C(21623064391401473));
    }

    /** stellar/go's AfterLedger, LedgerRangeInclusive and IncOperationOrder. */
    void testGoCursorHelpers()
    {
        const qint64 op = Q_INT64_C(21623064391401473);
        const TOID after = TOID::afterLedger(5034512u);
        QCOMPARE(after.toInt64(), (Q_INT64_C(5034512) << 32) | (qint64(TOID::MAX_TX_INDEX) << 12) | TOID::MAX_OP_INDEX);
        QVERIFY(after.toInt64() > op);
        QVERIFY(after.toInt64() < TOID(5034513u, 0u, 0u).toInt64());

        const QPair<qint64, qint64> range = TOID::ledgerRangeInclusive(5034512u, 5034512u);
        QCOMPARE(range.first, Q_INT64_C(21623064391319552));
        QCOMPARE(range.second, TOID(5034513u, 0u, 0u).toInt64());
        QVERIFY(range.first <= op && op < range.second);
        QCOMPARE(TOID::ledgerRangeInclusive(1u, 1u), qMakePair(qint64(0), qint64(Q_INT64_C(2) << 32)));
        QCOMPARE(TOID::ledgerRangeInclusive(2u, 5u), qMakePair(qint64(Q_INT64_C(2) << 32), qint64(Q_INT64_C(6) << 32)));
        QVERIFY_EXCEPTION_THROWN(TOID::ledgerRangeInclusive(5u, 4u), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(TOID::ledgerRangeInclusive(0u, 4u), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(TOID::ledgerRangeInclusive(1u, TOID::MAX_LEDGER), std::runtime_error);

        TOID cursor(7u, 3u, 5u);
        cursor.incOperationOrder();
        QVERIFY(cursor == TOID(7u, 3u, 6u));
        TOID full(7u, 3u, TOID::MAX_OP_INDEX);
        full.incOperationOrder();
        QVERIFY(full == TOID(8u, 3u, 0u));   // the next ledger, as go
        TOID last(TOID::MAX_LEDGER, 0u, TOID::MAX_OP_INDEX);
        QVERIFY_EXCEPTION_THROWN(last.incOperationOrder(), std::runtime_error);
    }

    /** Horizon's text form, its order, and use as a hash key. */
    void testTextOrderAndHash()
    {
        const TOID op = TOID::fromString(QStringLiteral("21623064391401473"));
        QCOMPARE(op.toString(), QStringLiteral("21623064391401473"));
        QCOMPARE(op.toUInt64(), Q_UINT64_C(21623064391401473));
        QVERIFY(op == TOID(5034512u, 20u, 1u));
        QCOMPARE(TOID::fromString(QStringLiteral("9223372036854775807")).toInt64(),
                 std::numeric_limits<qint64>::max());
        for (const char* bad : {"", "-1", "+1", " 1", "1 ", "9223372036854775808", "12a", "0x10",
                                "12345678901234567890"}) {
            QVERIFY_EXCEPTION_THROWN(TOID::fromString(QString::fromLatin1(bad)), std::runtime_error);
        }

        QVERIFY(TOID(1u, 0u, 0u) < TOID(1u, 0u, 1u));
        QVERIFY(TOID(1u, 0u, TOID::MAX_OP_INDEX) < TOID(1u, 1u, 0u));
        QVERIFY(TOID(1u, TOID::MAX_TX_INDEX, TOID::MAX_OP_INDEX) < TOID(2u, 0u, 0u));
        QVERIFY(!(op < op));

        QSet<TOID> seen{op, TOID(5034512u, 20u, 0u)};
        seen.insert(TOID::fromInt64(Q_INT64_C(21623064391401473)));
        QCOMPARE(seen.size(), 2);
        QVERIFY(seen.contains(TOID(5034512u, 20u, 1u)));
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
