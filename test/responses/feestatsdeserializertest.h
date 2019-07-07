#ifndef OPERATIONFEESTATSDESERIALIZERTEST_H
#define OPERATIONFEESTATSDESERIALIZERTEST_H
#include <QObject>
#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/feestatsresponse.h"

class FeeStatsDeserializerTest: public QObject
{
    Q_OBJECT
    const QByteArray json = "{\n"
            "  \"last_ledger\": \"20882791\",\n"
            "  \"last_ledger_base_fee\": \"100\",\n"
            "  \"ledger_capacity_usage\": \"0.97\",\n"
            "  \"min_accepted_fee\": \"101\",\n"
            "  \"mode_accepted_fee\": \"102\",\n"
            "  \"p10_accepted_fee\": \"103\",\n"
            "  \"p20_accepted_fee\": \"104\",\n"
            "  \"p30_accepted_fee\": \"105\",\n"
            "  \"p40_accepted_fee\": \"106\",\n"
            "  \"p50_accepted_fee\": \"107\",\n"
            "  \"p60_accepted_fee\": \"108\",\n"
            "  \"p70_accepted_fee\": \"109\",\n"
            "  \"p80_accepted_fee\": \"110\",\n"
            "  \"p90_accepted_fee\": \"111\",\n"
            "  \"p95_accepted_fee\": \"112\",\n"
            "  \"p99_accepted_fee\": \"113\""
            "}";
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserialize() {
        FeeStatsResponse stats;
        stats.loadFromJson(json);
        QCOMPARE(stats.getLastLedger(),20882791);
        QCOMPARE(stats.getLastLedgerBaseFee(),100);
        QCOMPARE(stats.getLedgerCapacityUsage(),0.97f);
        QCOMPARE(stats.getMin(),101);
        QCOMPARE(stats.getMode(),102);
        QCOMPARE(stats.getP10(),103);
        QCOMPARE(stats.getP20(),104);
        QCOMPARE(stats.getP30(),105);
        QCOMPARE(stats.getP40(),106);
        QCOMPARE(stats.getP50(),107);
        QCOMPARE(stats.getP60(),108);
        QCOMPARE(stats.getP70(),109);
        QCOMPARE(stats.getP80(),110);
        QCOMPARE(stats.getP90(),111);
        QCOMPARE(stats.getP95(),112);
        QCOMPARE(stats.getP99(),113);
    }

};

ADD_TEST(FeeStatsDeserializerTest)
#endif // OPERATIONFEESTATSDESERIALIZERTEST_H
