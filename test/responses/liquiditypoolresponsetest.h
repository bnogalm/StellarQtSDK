#ifndef LIQUIDITYPOOLRESPONSETEST_H
#define LIQUIDITYPOOLRESPONSETEST_H
#include <QObject>
#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/liquiditypoolresponse.h"

class LiquidityPoolResponseTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    /** Deserialize a `/liquidity_pools/{id}` Horizon response. */
    void testDeserialize()
    {
        QByteArray json = "{"
            "\"id\":\"abc1234\","
            "\"paging_token\":\"abc1234\","
            "\"fee_bp\":30,"
            "\"type\":\"constant_product\","
            "\"total_trustlines\":\"42\","
            "\"total_shares\":\"100.0000000\","
            "\"reserves\":["
                "{\"asset\":\"native\",\"amount\":\"50.0000000\"},"
                "{\"asset\":\"USDC:GA5\",\"amount\":\"50.0000000\"}"
            "],"
            "\"last_modified_ledger\":12345,"
            "\"last_modified_time\":\"2021-11-08T01:46:38Z\","
            "\"_links\":{"
                "\"self\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/abc1234\"},"
                "\"operations\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/abc1234/operations\"},"
                "\"transactions\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/abc1234/transactions\"}"
            "}}";
        LiquidityPoolResponse r;
        r.loadFromJson(json);

        QCOMPARE(r.getId(), QString("abc1234"));
        QCOMPARE(r.getPagingToken(), QString("abc1234"));
        QCOMPARE(r.getFeeBp(), (qint32)30);
        QCOMPARE(r.getType(), QString("constant_product"));
        QCOMPARE(r.getTotalTrustlines(), (qint64)42);
        QCOMPARE(r.getTotalShares(), QString("100.0000000"));
        QCOMPARE(r.getReserves().size(), 2);
        QCOMPARE(r.getReserves()[0].getAsset(), QString("native"));
        QCOMPARE(r.getReserves()[0].getAmount(), QString("50.0000000"));
        QCOMPARE(r.getReserves()[1].getAsset(), QString("USDC:GA5"));
        QCOMPARE(r.getLastModifiedLedger(), (qint64)12345);
        QCOMPARE(r.getLastModifiedTime(), QString("2021-11-08T01:46:38Z"));
        QCOMPARE(r.getLinks().getSelf().getHref(),
                 QString("https://horizon.stellar.org/liquidity_pools/abc1234"));
    }
};

ADD_TEST(LiquidityPoolResponseTest)
#endif // LIQUIDITYPOOLRESPONSETEST_H
