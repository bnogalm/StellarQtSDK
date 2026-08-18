#ifndef EXACTOFFERPRICETEST_H
#define EXACTOFFERPRICETEST_H

#include <QObject>
#include <QtTest>
#include <stdexcept>
#include "../testcollector.h"

#include "../../src/managesellofferoperation.h"
#include "../../src/managebuyofferoperation.h"
#include "../../src/createpassivesellofferoperation.h"
#include "../../src/price.h"
#include "../../src/asset.h"
#include "../../src/assettypenative.h"
#include "../../src/assettypecreditalphanum4.h"
#include "../../src/txrep.h"
#include "../../src/transaction.h"
#include "../../src/transactionbuilder.h"
#include "../../src/account.h"
#include "../../src/keypair.h"
#include "../../src/network.h"
#include "../../src/operation.h"

/**
 * Exact price in the three offer operations.
 *
 * A Stellar price IS an n/d fraction in the XDR. The offer classes only let
 * you build and read it as a decimal string, and that string goes through
 * `Price(QString)`, which clips to 11 characters and re-derives the fraction
 * over 1e9. Result: fractions with no short decimal representation were
 * silently corrupted on construction, on read and on the way through txrep.
 */
class ExactOfferPriceTest: public QObject
{
    Q_OBJECT

    static const char* ISSUER() { return "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"; }
    static const char* SOURCE() { return "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"; }

    static Asset* native() { return new AssetTypeNative(); }
    static Asset* usd()    { return new AssetTypeCreditAlphaNum4("USD", QString(ISSUER())); }

    /** Fractions whose decimal expansion repeats: exactly the ones the
     *  approximation destroys. The last one, 1/2, does survive (its
     *  denominator divides 1e9) and acts as the control. */
    static QList<QPair<qint32,qint32>> awkwardFractions()
    {
        return {
            qMakePair(1, 3),
            qMakePair(7, 9),
            qMakePair(3, 7),
            qMakePair(22, 7),
            qMakePair(2147483647, 2147483646),   // int32 limits
            qMakePair(1, 2)                      // control: survives the approximation
        };
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    /** The starting point: the decimal approximation REALLY does lose the
     *  fraction. If this test's assertion ever stops holding, Price(QString)
     *  changed and the rest of this file is redundant. */
    void testDecimalRoundTripLosesTheFraction()
    {
        const Price exact(7, 9);
        const Price viaDecimal(exact.toString());
        QCOMPARE(exact.getNumerator(), 7);
        QCOMPARE(exact.getDenominator(), 9);
        // 7/9 -> "0.7777777778" -> 777777778/1000000000 (or similar): lost.
        QVERIFY(viaDecimal.getNumerator() != 7 || viaDecimal.getDenominator() != 9);
    }

    // ─── getPriceR(): read the exact fraction ────────────────────────

    void testManageSellOfferKeepsExactPrice()
    {
        for (const auto& f : awkwardFractions()) {
            const Price p(f.first, f.second);
            QScopedPointer<ManageSellOfferOperation> op(
                ManageSellOfferOperation::create(native(), usd(), "100", p));
            QCOMPARE(op->getPriceR().getNumerator(), f.first);
            QCOMPARE(op->getPriceR().getDenominator(), f.second);
        }
    }

    void testManageBuyOfferKeepsExactPrice()
    {
        for (const auto& f : awkwardFractions()) {
            const Price p(f.first, f.second);
            QScopedPointer<ManageBuyOfferOperation> op(
                ManageBuyOfferOperation::create(native(), usd(), "100", p));
            QCOMPARE(op->getPriceR().getNumerator(), f.first);
            QCOMPARE(op->getPriceR().getDenominator(), f.second);
        }
    }

    void testCreatePassiveSellOfferKeepsExactPrice()
    {
        for (const auto& f : awkwardFractions()) {
            const Price p(f.first, f.second);
            QScopedPointer<CreatePassiveSellOfferOperation> op(
                CreatePassiveSellOfferOperation::create(native(), usd(), "100", p));
            QCOMPARE(op->getPriceR().getNumerator(), f.first);
            QCOMPARE(op->getPriceR().getDenominator(), f.second);
        }
    }

    /** The fraction also has to survive the trip through the XDR, which is
     *  how operations arrive from the network. */
    void testExactPriceSurvivesXdrRoundTrip()
    {
        const Price p(3, 7);
        QScopedPointer<ManageSellOfferOperation> op(
            ManageSellOfferOperation::create(native(), usd(), "100", p));
        AccountConverter conv;
        stellar::Operation xdr = op->toXdr(conv);
        QScopedPointer<Operation> back(Operation::fromXdr(xdr));
        auto* mso = dynamic_cast<ManageSellOfferOperation*>(back.data());
        QVERIFY(mso != nullptr);
        QCOMPARE(mso->getPriceR().getNumerator(), 3);
        QCOMPARE(mso->getPriceR().getDenominator(), 7);
    }

    /** The string overload still exists and still approximates: that is
     *  compatibility, not a bug. It is pinned here so that changing the
     *  behavior stays a deliberate act if anyone touches it. */
    void testStringOverloadStillApproximatesByDesign()
    {
        QScopedPointer<ManageSellOfferOperation> op(
            ManageSellOfferOperation::create(native(), usd(), "100", QString("0.5")));
        QCOMPARE(op->getPriceR().getNumerator(), 1);
        QCOMPARE(op->getPriceR().getDenominator(), 2);
        QCOMPARE(op->getPrice(), QString("0.5"));
    }

    // ─── txrep: the place where the loss actually showed up ──────────

    static Transaction* buildWith(Operation* op)
    {
        KeyPair* source = KeyPair::fromAccountId(QString(SOURCE()));
        Account* acc = new Account(source, 0);
        Transaction* tx = TransactionBuilder(acc, Network::testnetNetwork())
                              .addOperation(op)
                              .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                              .setBaseFee(100)
                              .build();
        delete acc;
        return tx;
    }

    /** A txrep document carries exact n and d. They used to be turned into
     *  decimal on parse and approximated again, so `price.n: 7 / price.d: 9`
     *  produced a transaction with a DIFFERENT price and a different hash. */
    void testTxrepRoundTripPreservesExactFraction()
    {
        for (const auto& f : awkwardFractions()) {
            const Price p(f.first, f.second);
            QScopedPointer<Transaction> tx(buildWith(
                ManageSellOfferOperation::create(native(), usd(), "100", p)));

            const QString txrep = Txrep::toTxrep(tx.data());
            QVERIFY2(txrep.contains(QString("price.n: %1").arg(f.first)),
                     qPrintable(QString("missing price.n %1").arg(f.first)));
            QVERIFY2(txrep.contains(QString("price.d: %1").arg(f.second)),
                     qPrintable(QString("missing price.d %1").arg(f.second)));

            QScopedPointer<Transaction> back(
                Txrep::fromTxrep(txrep, Network::testnetNetwork()));
            auto* mso = dynamic_cast<ManageSellOfferOperation*>(back->getOperations().at(0));
            QVERIFY(mso != nullptr);
            QCOMPARE(mso->getPriceR().getNumerator(), f.first);
            QCOMPARE(mso->getPriceR().getDenominator(), f.second);
        }
    }

    void testTxrepRoundTripPreservesExactFractionBuyOffer()
    {
        const Price p(22, 7);
        QScopedPointer<Transaction> tx(buildWith(
            ManageBuyOfferOperation::create(native(), usd(), "100", p)));
        QScopedPointer<Transaction> back(
            Txrep::fromTxrep(Txrep::toTxrep(tx.data()), Network::testnetNetwork()));
        auto* mbo = dynamic_cast<ManageBuyOfferOperation*>(back->getOperations().at(0));
        QVERIFY(mbo != nullptr);
        QCOMPARE(mbo->getPriceR().getNumerator(), 22);
        QCOMPARE(mbo->getPriceR().getDenominator(), 7);
    }

    void testTxrepRoundTripPreservesExactFractionPassiveOffer()
    {
        const Price p(1, 3);
        QScopedPointer<Transaction> tx(buildWith(
            CreatePassiveSellOfferOperation::create(native(), usd(), "100", p)));
        QScopedPointer<Transaction> back(
            Txrep::fromTxrep(Txrep::toTxrep(tx.data()), Network::testnetNetwork()));
        auto* cpo = dynamic_cast<CreatePassiveSellOfferOperation*>(back->getOperations().at(0));
        QVERIFY(cpo != nullptr);
        QCOMPARE(cpo->getPriceR().getNumerator(), 1);
        QCOMPARE(cpo->getPriceR().getDenominator(), 3);
    }

    /** The transaction hash depends on the price: if the fraction changes on
     *  the way through txrep, a transaction other than the one the user
     *  reviewed gets signed. This is the consequence that really matters. */
    void testTxrepRoundTripPreservesTransactionHash()
    {
        const Price p(3, 7);
        QScopedPointer<Transaction> tx(buildWith(
            ManageSellOfferOperation::create(native(), usd(), "100", p)));
        QScopedPointer<Transaction> back(
            Txrep::fromTxrep(Txrep::toTxrep(tx.data()), Network::testnetNetwork()));
        QCOMPARE(back->hashHex(), tx->hashHex());
    }
};

ADD_TEST(ExactOfferPriceTest)
#endif // EXACTOFFERPRICETEST_H
