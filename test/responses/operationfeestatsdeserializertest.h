#ifndef OPERATIONFEESTATSDESERIALIZERTEST_H
#define OPERATIONFEESTATSDESERIALIZERTEST_H
#include <QObject>
#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/operationfeestatsresponse.h"

class OperationFeeStatsDeserializerTest: public QObject
{
    Q_OBJECT
    const QByteArray json = "{\n"
            "  \"last_ledger\": \"20882791\",\n"
            "  \"last_ledger_base_fee\": \"100\",\n"
            "  \"min_accepted_fee\": \"101\",\n"
            "  \"mode_accepted_fee\": \"102\"\n"
            "}";
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserialize() {
        OperationFeeStatsResponse stats;
        stats.loadFromJson(json);
        QCOMPARE(stats.getLastLedger(),20882791);
        QCOMPARE(stats.getLastLedgerBaseFee(),100);
        QCOMPARE(stats.getMin(),101);
        QCOMPARE(stats.getMode(),102);
    }

};

ADD_TEST(OperationFeeStatsDeserializerTest)
#endif // OPERATIONFEESTATSDESERIALIZERTEST_H
