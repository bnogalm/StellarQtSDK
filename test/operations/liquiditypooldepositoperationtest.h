#ifndef LIQUIDITYPOOLDEPOSITOPERATIONTEST_H
#define LIQUIDITYPOOLDEPOSITOPERATIONTEST_H

#include <QObject>
#include <QtTest>
#include <functional>
#include <limits>
#include <stdexcept>
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

    /** A price term that is not above zero is refused, for either bound, as
     *  Core (MALFORMED) and js ("price must be positive") refuse it. These
     *  used to be signed, the default Price() 0/0 included. */
    void testRefusesNonPositivePrices()
    {
        const QByteArray poolId(32, '\0');
        const qint32 min = std::numeric_limits<qint32>::min();
        for (const Price& bad : {Price(), Price(0, 1), Price(1, 0), Price(-1, 2),
                                 Price(1, -2), Price(min, 1)}) {
            const QByteArray what = QStringLiteral("%1/%2").arg(bad.getNumerator())
                                        .arg(bad.getDenominator()).toUtf8();
            QVERIFY2(refusedWith("minPrice must be positive", [&]() {
                delete LiquidityPoolDepositOperation::create(poolId, "1", "1", bad, Price(1, 1)); }),
                     what.constData());
            QVERIFY2(refusedWith("maxPrice must be positive", [&]() {
                delete LiquidityPoolDepositOperation::create(poolId, "1", "1", Price(1, 1), bad); }),
                     what.constData());
        }
    }

    /** minPrice above maxPrice is refused with Core's own 64-bit comparison;
     *  it used to be signed and then refused by Core as MALFORMED. */
    void testRefusesMinPriceAboveMaxPrice()
    {
        const QByteArray poolId(32, '\0');
        const QList<QPair<Price, Price>> inverted = {
            {Price(2, 1), Price(1, 1)},
            {Price(2147483646, 2147483645), Price(2147483647, 2147483646)},
            {Price(2147483647, 1), Price(1, 2147483647)},
        };
        for (const auto& p : inverted) {
            QVERIFY2(refusedWith("minPrice must not be greater than maxPrice", [&]() {
                delete LiquidityPoolDepositOperation::create(poolId, "1", "1", p.first, p.second); }),
                     qPrintable(p.first.toString() + QStringLiteral(" > ") + p.second.toString()));
        }
        // Equal, ordered, and ordered by one part in 2^62, all build exactly.
        const QList<QPair<Price, Price>> ordered = {
            {Price(1, 1), Price(1, 1)},
            {Price(2, 4), Price(1, 2)},
            {Price(1, 2147483647), Price(2147483647, 1)},
            {Price(2147483647, 2147483646), Price(2147483646, 2147483645)},
        };
        for (const auto& p : ordered) {
            QScopedPointer<LiquidityPoolDepositOperation> op(
                LiquidityPoolDepositOperation::create(poolId, "1", "1", p.first, p.second));
            QCOMPARE(op->getMinPrice(), p.first);
            QCOMPARE(op->getMaxPrice(), p.second);
        }
    }

    /** A zero maximum amount is refused, as Core (MALFORMED) refuses it; "0"
     *  used to be signed. The smallest amount, one stroop, still builds. */
    void testRefusesZeroMaxAmount()
    {
        const QByteArray poolId(32, '\0');
        for (const char* zero : {"0", "0.0000000", "0.00"}) {
            QVERIFY2(refusedWith("maxAmountA must be positive", [&]() {
                delete LiquidityPoolDepositOperation::create(poolId, zero, "1", Price(1, 1), Price(1, 1)); }),
                     zero);
            QVERIFY2(refusedWith("maxAmountB must be positive", [&]() {
                delete LiquidityPoolDepositOperation::create(poolId, "1", zero, Price(1, 1), Price(1, 1)); }),
                     zero);
        }
        QScopedPointer<LiquidityPoolDepositOperation> op(
            LiquidityPoolDepositOperation::create(poolId, "0.0000001", "0.0000001",
                                                  Price(1, 1), Price(1, 1)));
        QCOMPARE(op->getMaxAmountA(), QStringLiteral("0.0000001"));
    }

    /** The XDR path stays lenient, so an envelope carrying such a deposit
     *  can still be read back and shown. */
    void testXdrPathStillReadsAnyPrice()
    {
        stellar::LiquidityPoolDepositOp xdr{};
        xdr.minPrice.n = 2;
        xdr.minPrice.d = 1;
        xdr.maxPrice.n = 0;
        xdr.maxPrice.d = 0;
        QScopedPointer<LiquidityPoolDepositOperation> op(LiquidityPoolDepositOperation::build(xdr));
        QCOMPARE(op->getMinPrice(), Price(2, 1));
        QCOMPARE(op->getMaxPrice(), Price(0, 0));
        QCOMPARE(op->getMaxAmountA(), QStringLiteral("0"));
    }

private:
    static bool refusedWith(const char* message, const std::function<void()>& build)
    {
        try {
            build();
        } catch (const std::runtime_error& e) {
            return QString::fromUtf8(e.what()) == QLatin1String(message);
        }
        return false;
    }
};

ADD_TEST(LiquidityPoolDepositOperationTest)
#endif // LIQUIDITYPOOLDEPOSITOPERATIONTEST_H
