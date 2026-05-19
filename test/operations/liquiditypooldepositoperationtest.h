#ifndef LIQUIDITYPOOLDEPOSITOPERATIONTEST_H
#define LIQUIDITYPOOLDEPOSITOPERATIONTEST_H

#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/operation.h"
#include "../../src/liquiditypooldepositoperation.h"
#include "../../src/price.h"

/** Tests for LiquidityPoolDepositOperation (CAP-38). */
class LiquidityPoolDepositOperationTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testRoundTrip()
    {
        QByteArray poolId(32, '\0');
        for (int i = 0; i < 32; ++i) poolId[i] = static_cast<char>(i);

        QScopedPointer<LiquidityPoolDepositOperation> op(
            LiquidityPoolDepositOperation::create(
                poolId, "100", "200",
                Price(1, 2),
                Price(3, 4)));
        op->setSourceAccount("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");

        stellar::Operation xdr = op->toXdr();
        QCOMPARE(static_cast<int>(xdr.type),
                 static_cast<int>(stellar::OperationType::LIQUIDITY_POOL_DEPOSIT));

        QScopedPointer<LiquidityPoolDepositOperation> parsed(
            static_cast<LiquidityPoolDepositOperation*>(Operation::fromXdr(xdr)));

        QCOMPARE(parsed->getLiquidityPoolID(), poolId);
        QCOMPARE(parsed->getMaxAmountA(), QString("100"));
        QCOMPARE(parsed->getMaxAmountB(), QString("200"));
        QCOMPARE(parsed->getMinPrice(), Price(1, 2));
        QCOMPARE(parsed->getMaxPrice(), Price(3, 4));
        QCOMPARE(parsed->getSourceAccount(),
                 QString("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
    }

    void testRejectsBadPoolIdSize()
    {
        bool threw = false;
        try {
            LiquidityPoolDepositOperation::create(
                QByteArray(31, '\0'),
                "100", "200", Price(1, 1), Price(1, 1));
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    void testAmountsStroopsRoundTrip()
    {
        QByteArray poolId(32, '\0');
        QScopedPointer<LiquidityPoolDepositOperation> op(
            LiquidityPoolDepositOperation::create(
                poolId, "123.4500001", "0.0000001",
                Price(1, 100), Price(100, 1)));

        stellar::Operation xdr = op->toXdr();
        QCOMPARE(xdr.operationLiquidityPoolDeposit.maxAmountA, qint64(1234500001));
        QCOMPARE(xdr.operationLiquidityPoolDeposit.maxAmountB, qint64(1));
    }
};

ADD_TEST(LiquidityPoolDepositOperationTest)
#endif // LIQUIDITYPOOLDEPOSITOPERATIONTEST_H
