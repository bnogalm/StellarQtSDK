#ifndef LEDGERDESERIALIZERTEST_H
#define LEDGERDESERIALIZERTEST_H
#include <QObject>
#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/ledgerresponse.h"

class LedgerDeserializerTest: public QObject
{
    Q_OBJECT
    const QByteArray json = "{\n"
            "  \"_links\": {\n"
            "    \"effects\": {\n"
            "      \"href\": \"/ledgers/898826/effects{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"operations\": {\n"
            "      \"href\": \"/ledgers/898826/operations{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"self\": {\n"
            "      \"href\": \"/ledgers/898826\"\n"
            "    },\n"
            "    \"transactions\": {\n"
            "      \"href\": \"/ledgers/898826/transactions{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    }\n"
            "  },\n"
            "  \"id\": \"686bb246db89b099cd3963a4633eb5e4315d89dfd3c00594c80b41a483847bfa\",\n"
            "  \"paging_token\": \"3860428274794496\",\n"
            "  \"hash\": \"686bb246db89b099cd3963a4633eb5e4315d89dfd3c00594c80b41a483847bfa\",\n"
            "  \"prev_hash\": \"50c8695eb32171a19858413e397cc50b504ceacc819010bdf8ff873aff7858d7\",\n"
            "  \"sequence\": 898826,\n"
            "  \"transaction_count\": 1,\n"
            "  \"operation_count\": 2,\n"
            "  \"closed_at\": \"2015-11-19T21:35:59Z\",\n"
            "  \"total_coins\": \"101343867604.8975480\",\n"
            "  \"fee_pool\": \"1908.2248818\",\n"
            "  \"base_fee\": 100,\n"
            "  \"base_reserve\": \"10.0000000\",\n"
            "  \"max_tx_set_size\": 50\n"
"}";
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserialize() {
        LedgerResponse ledger;
        ledger.loadFromJson(json);
        QCOMPARE(ledger.getHash(), QString("686bb246db89b099cd3963a4633eb5e4315d89dfd3c00594c80b41a483847bfa"));
        QCOMPARE(ledger.getPagingToken(), QString("3860428274794496"));
        QCOMPARE(ledger.getPrevHash(), QString("50c8695eb32171a19858413e397cc50b504ceacc819010bdf8ff873aff7858d7"));
        QCOMPARE(ledger.getSequence(), 898826);
        QCOMPARE(ledger.getTransactionCount(), 1);
        QCOMPARE(ledger.getOperationCount(), 2);
        QCOMPARE(ledger.getClosedAt(), QString("2015-11-19T21:35:59Z"));
        QCOMPARE(ledger.getTotalCoins(), QString("101343867604.8975480"));
        QCOMPARE(ledger.getFeePool(), QString("1908.2248818"));
        QCOMPARE(ledger.getBaseFee(), 100);
        QCOMPARE(ledger.getBaseReserve(), QString("10.0000000"));
        QCOMPARE(ledger.getMaxTxSetSize(), 50);
        QCOMPARE(ledger.getLinks().getEffects().getHref(), QString("/ledgers/898826/effects{?cursor,limit,order}"));
        QCOMPARE(ledger.getLinks().getOperations().getHref(), QString("/ledgers/898826/operations{?cursor,limit,order}"));
        QCOMPARE(ledger.getLinks().getSelf().getHref(), QString("/ledgers/898826"));
        QCOMPARE(ledger.getLinks().getTransactions().getHref(), QString("/ledgers/898826/transactions{?cursor,limit,order}"));
    }



};


ADD_TEST(LedgerDeserializerTest)
#endif // LEDGERDESERIALIZERTEST_H
