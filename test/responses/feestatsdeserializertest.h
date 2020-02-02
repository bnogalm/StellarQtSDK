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
                            "  \"last_ledger\": \"22606298\",\n"
                            "  \"last_ledger_base_fee\": \"100\",\n"
                            "  \"ledger_capacity_usage\": \"0.97\",\n"
                            "  \"max_fee\": {\n"
                            "    \"min\": \"130\",\n"
                            "    \"max\": \"8000\",\n"
                            "    \"mode\": \"250\",\n"
                            "    \"p10\": \"150\",\n"
                            "    \"p20\": \"200\",\n"
                            "    \"p30\": \"300\",\n"
                            "    \"p40\": \"400\",\n"
                            "    \"p50\": \"500\",\n"
                            "    \"p60\": \"1000\",\n"
                            "    \"p70\": \"2000\",\n"
                            "    \"p80\": \"3000\",\n"
                            "    \"p90\": \"4000\",\n"
                            "    \"p95\": \"5000\",\n"
                            "    \"p99\": \"8000\"\n"
                            "  },\n"
                            "  \"fee_charged\": {\n"
                            "    \"min\": \"100\",\n"
                            "    \"max\": \"102\",\n"
                            "    \"mode\": \"101\",\n"
                            "    \"p10\": \"103\",\n"
                            "    \"p20\": \"104\",\n"
                            "    \"p30\": \"105\",\n"
                            "    \"p40\": \"106\",\n"
                            "    \"p50\": \"107\",\n"
                            "    \"p60\": \"108\",\n"
                            "    \"p70\": \"109\",\n"
                            "    \"p80\": \"110\",\n"
                            "    \"p90\": \"111\",\n"
                            "    \"p95\": \"112\",\n"
                            "    \"p99\": \"113\"\n"
                            "  }\n"
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
        QCOMPARE(stats.getLastLedger(),22606298);
        QCOMPARE(stats.getLastLedgerBaseFee(),100);
        QCOMPARE(stats.getLedgerCapacityUsage(),0.97f);
        FeeDistribution maxFee = stats.getMaxFee();
        QCOMPARE(maxFee.getMin(),130);
        QCOMPARE(maxFee.getMax(),8000);
        QCOMPARE(maxFee.getMode(),250);
        QCOMPARE(maxFee.getP10(),150);
        QCOMPARE(maxFee.getP20(),200);
        QCOMPARE(maxFee.getP30(),300);
        QCOMPARE(maxFee.getP40(),400);
        QCOMPARE(maxFee.getP50(),500);
        QCOMPARE(maxFee.getP60(),1000);
        QCOMPARE(maxFee.getP70(),2000);
        QCOMPARE(maxFee.getP80(),3000);
        QCOMPARE(maxFee.getP90(),4000);
        QCOMPARE(maxFee.getP95(),5000);
        QCOMPARE(maxFee.getP99(),8000);

        FeeDistribution feeCharged = stats.getFeeCharged();
        QCOMPARE(feeCharged.getMin(),100);
        QCOMPARE(feeCharged.getMax(),102);
        QCOMPARE(feeCharged.getMode(),101);
        QCOMPARE(feeCharged.getP10(),103);
        QCOMPARE(feeCharged.getP20(),104);
        QCOMPARE(feeCharged.getP30(),105);
        QCOMPARE(feeCharged.getP40(),106);
        QCOMPARE(feeCharged.getP50(),107);
        QCOMPARE(feeCharged.getP60(),108);
        QCOMPARE(feeCharged.getP70(),109);
        QCOMPARE(feeCharged.getP80(),110);
        QCOMPARE(feeCharged.getP90(),111);
        QCOMPARE(feeCharged.getP95(),112);
        QCOMPARE(feeCharged.getP99(),113);
    }

};

ADD_TEST(FeeStatsDeserializerTest)
#endif // OPERATIONFEESTATSDESERIALIZERTEST_H
