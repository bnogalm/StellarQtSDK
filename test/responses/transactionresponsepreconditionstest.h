#ifndef TRANSACTIONRESPONSEPRECONDITIONSTEST_H
#define TRANSACTIONRESPONSEPRECONDITIONSTEST_H

#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/responses/transactionresponse.h"

/**
 * Tests that TransactionResponse deserializes Horizon's `preconditions` block
 * (CAP-21 / Protocol 19).
 */
class TransactionResponsePreconditionsTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testDeserializeFullV2Preconditions()
    {
        QByteArray json = "{"
            "\"hash\":\"abc\",\"ledger\":1,\"created_at\":\"2026-01-01T00:00:00Z\","
            "\"source_account\":\"GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\","
            "\"fee_account\":\"GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\","
            "\"successful\":true,"
            "\"paging_token\":\"1\","
            "\"source_account_sequence\":1,"
            "\"max_fee\":100,\"fee_charged\":100,"
            "\"operation_count\":1,"
            "\"envelope_xdr\":\"\",\"result_xdr\":\"\",\"result_meta_xdr\":\"\","
            "\"memo_type\":\"none\","
            "\"signatures\":[\"sig1\"],"
            "\"preconditions\":{"
                "\"timebounds\":{\"min_time\":\"1626888681\",\"max_time\":\"1626888881\"},"
                "\"ledgerbounds\":{\"min_ledger\":10,\"max_ledger\":99},"
                "\"min_account_sequence\":\"5\","
                "\"min_account_sequence_age\":\"60\","
                "\"min_account_sequence_ledger_gap\":3,"
                "\"extra_signers\":["
                    "\"GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\""
                "]"
            "},"
            "\"_links\":{}"
        "}";

        TransactionResponse r;
        r.loadFromJson(json);

        const auto& pre = r.getPreconditions();
        QCOMPARE(pre.getTimeBounds().getMinTime(), QString("1626888681"));
        QCOMPARE(pre.getTimeBounds().getMaxTime(), QString("1626888881"));
        QCOMPARE(pre.getLedgerBounds().getMinLedger(), quint32(10));
        QCOMPARE(pre.getLedgerBounds().getMaxLedger(), quint32(99));
        QCOMPARE(pre.getMinAccountSequence(), QString("5"));
        QCOMPARE(pre.getMinAccountSequenceAge(), QString("60"));
        QCOMPARE(pre.getMinAccountSequenceLedgerGap(), quint32(3));
        QCOMPARE(pre.getExtraSigners().size(), 1);
        QCOMPARE(pre.getExtraSigners().first(),
                 QString("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
    }

    void testDeserializeTimeBoundsOnly()
    {
        QByteArray json = "{"
            "\"hash\":\"abc\",\"ledger\":1,\"created_at\":\"2026-01-01T00:00:00Z\","
            "\"source_account\":\"GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\","
            "\"fee_account\":\"GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\","
            "\"successful\":true,"
            "\"paging_token\":\"1\","
            "\"source_account_sequence\":1,"
            "\"max_fee\":100,\"fee_charged\":100,"
            "\"operation_count\":1,"
            "\"envelope_xdr\":\"\",\"result_xdr\":\"\",\"result_meta_xdr\":\"\","
            "\"memo_type\":\"none\","
            "\"signatures\":[\"sig1\"],"
            "\"preconditions\":{"
                "\"timebounds\":{\"min_time\":\"100\",\"max_time\":\"200\"}"
            "},"
            "\"_links\":{}"
        "}";

        TransactionResponse r;
        r.loadFromJson(json);

        const auto& pre = r.getPreconditions();
        QCOMPARE(pre.getTimeBounds().getMinTime(), QString("100"));
        QCOMPARE(pre.getTimeBounds().getMaxTime(), QString("200"));
        QCOMPARE(pre.getLedgerBounds().getMinLedger(), quint32(0));
        QCOMPARE(pre.getLedgerBounds().getMaxLedger(), quint32(0));
        QVERIFY(pre.getMinAccountSequence().isEmpty());
        QCOMPARE(pre.getMinAccountSequenceLedgerGap(), quint32(0));
        QVERIFY(pre.getExtraSigners().isEmpty());
    }

    void testDeserializeAbsentPreconditions()
    {
        // No "preconditions" key at all — fields stay default.
        QByteArray json = "{"
            "\"hash\":\"abc\",\"ledger\":1,\"created_at\":\"2026-01-01T00:00:00Z\","
            "\"source_account\":\"GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\","
            "\"fee_account\":\"GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG\","
            "\"successful\":true,"
            "\"paging_token\":\"1\","
            "\"source_account_sequence\":1,"
            "\"max_fee\":100,\"fee_charged\":100,"
            "\"operation_count\":1,"
            "\"envelope_xdr\":\"\",\"result_xdr\":\"\",\"result_meta_xdr\":\"\","
            "\"memo_type\":\"none\","
            "\"signatures\":[\"sig1\"],"
            "\"_links\":{}"
        "}";

        TransactionResponse r;
        r.loadFromJson(json);

        const auto& pre = r.getPreconditions();
        QVERIFY(pre.getTimeBounds().getMinTime().isEmpty());
        QVERIFY(pre.getTimeBounds().getMaxTime().isEmpty());
        QCOMPARE(pre.getLedgerBounds().getMinLedger(), quint32(0));
        QCOMPARE(pre.getMinAccountSequenceLedgerGap(), quint32(0));
        QVERIFY(pre.getExtraSigners().isEmpty());
    }
};

ADD_TEST(TransactionResponsePreconditionsTest)
#endif // TRANSACTIONRESPONSEPRECONDITIONSTEST_H
