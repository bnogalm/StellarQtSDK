#ifndef LIQUIDITYPOOLWITHDRAWOPERATIONTEST_H
#define LIQUIDITYPOOLWITHDRAWOPERATIONTEST_H

#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/operation.h"
#include "../../src/liquiditypoolwithdrawoperation.h"

/** Tests for LiquidityPoolWithdrawOperation (CAP-38). */
class LiquidityPoolWithdrawOperationTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testRoundTrip()
    {
        QByteArray poolId(32, '\0');
        for (int i = 0; i < 32; ++i) poolId[i] = static_cast<char>(0xAA ^ i);

        QScopedPointer<LiquidityPoolWithdrawOperation> op(
            LiquidityPoolWithdrawOperation::create(
                poolId, "50", "10", "20"));
        op->setSourceAccount("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");

        stellar::Operation xdr = op->toXdr();
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::OperationType::LIQUIDITY_POOL_WITHDRAW));

        QScopedPointer<LiquidityPoolWithdrawOperation> parsed(
            static_cast<LiquidityPoolWithdrawOperation*>(Operation::fromXdr(xdr)));

        QCOMPARE(parsed->getLiquidityPoolID(), poolId);
        QCOMPARE(parsed->getAmount(), QString("50"));
        QCOMPARE(parsed->getMinAmountA(), QString("10"));
        QCOMPARE(parsed->getMinAmountB(), QString("20"));
        QCOMPARE(parsed->getSourceAccount(),
                 QString("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
    }

    void testRejectsBadPoolIdSize()
    {
        bool threw = false;
        try {
            LiquidityPoolWithdrawOperation::create(
                QByteArray(33, '\0'),
                "10", "1", "1");
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void testAmountsStroopsRoundTrip()
    {
        QByteArray poolId(32, '\0');
        QScopedPointer<LiquidityPoolWithdrawOperation> op(
            LiquidityPoolWithdrawOperation::create(
                poolId, "100.0000001", "0", "0.0000001"));

        stellar::Operation xdr = op->toXdr();
        QCOMPARE(xdr.operationLiquidityPoolWithdraw.amount, qint64(1000000001));
        QCOMPARE(xdr.operationLiquidityPoolWithdraw.minAmountA, qint64(0));
        QCOMPARE(xdr.operationLiquidityPoolWithdraw.minAmountB, qint64(1));
    }
};

ADD_TEST(LiquidityPoolWithdrawOperationTest)
#endif // LIQUIDITYPOOLWITHDRAWOPERATIONTEST_H
