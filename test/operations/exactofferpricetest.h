#ifndef EXACTOFFERPRICETEST_H
#define EXACTOFFERPRICETEST_H

#include <QObject>
#include <QtTest>
#include <functional>
#include <limits>
#include <stdexcept>
#include <type_traits>
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
 * `Price(QString)`. That used to clip to 11 characters and re-derive the
 * fraction over 1e9, so fractions with no short decimal representation were
 * silently corrupted on construction, on read and on the way through txrep.
 * It now reads the decimal exactly and approximates with continued fractions, which
 * recovers the fractions people write — but a decimal can still only
 * approximate a fraction in general, so the exact n/d overloads stay the way
 * to set a price that must not move.
 */
class ExactOfferPriceTest: public QObject
{
    Q_OBJECT

    static const char* ISSUER() { return "GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"; }
    static const char* SOURCE() { return "GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"; }

    static Asset* native() { return new AssetTypeNative(); }
    static Asset* usd()    { return new AssetTypeCreditAlphaNum4("USD", QString(ISSUER())); }

    /** Fractions whose decimal expansion repeats, plus the int32 limits,
     *  which no printed decimal pins down. The last one, 1/2, has an exact
     *  decimal and acts as the control. */
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

    /** The starting point, restated when Price(QString) changed on purpose.
     *
     *  It used to be that the decimal round trip lost even 7/9 (it came back
     *  as 777777777/1000000000). Reading the decimal exactly and expanding
     *  it as a continued fraction now brings 7/9 back — but not every
     *  fraction is promised: the decimal is rounded (to 20 places, as js
     *  prints it), and a fraction near the int32 limits may come back as
     *  another one. That is why the rest of this file reads getPriceR(). */
    void testDecimalRoundTripIsNotEnoughForEveryFraction()
    {
        const Price sevenNinths(Price(7, 9).toString());
        QCOMPARE(sevenNinths.getNumerator(), 7);
        QCOMPARE(sevenNinths.getDenominator(), 9);

        const Price limits(Price(2147483647, 2147483646).toString());
        QVERIFY(limits.getNumerator() > 0 && limits.getDenominator() > 0);
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

    /** Price(QString) used to run its input through QString::toDouble() without
     *  checking the ok flag, so "not a price" became 0.0 and produced a
     *  valid-looking offer at a price nobody asked for. Surfaced by the QML
     *  layer, where every field is a string typed by a human. */
    void testPriceRejectsNonNumericInput()
    {
        // nan and inf are accepted by QString::toDouble() with ok=true, and
        // reached an undefined double-to-integer conversion.
        for (const QString& bad : {QStringLiteral("not a price"), QStringLiteral("abc"),
                                   QStringLiteral(""), QStringLiteral("-1"),
                                   QStringLiteral("nan"), QStringLiteral("inf"),
                                   QStringLiteral("+inf"), QStringLiteral("1,5")}) {
            bool threw = false;
            try { Price p(bad); }
            catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, qPrintable(QStringLiteral("accepted: ") + bad));
        }
        // Zero is not a price (core refuses an offer at 0 as malformed; js and
        // python refuse it too), however it is written.
        for (const QString& zero : {QStringLiteral("0"), QStringLiteral("0.0"),
                                    QStringLiteral("0e5"), QStringLiteral(".0")}) {
            bool threw = false;
            try { Price p(zero); }
            catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, qPrintable(QStringLiteral("accepted: ") + zero));
        }
        // Valid input keeps working, including the integer case.
        QCOMPARE(Price(QStringLiteral("2")).getNumerator(), 2);
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

    // ─── What the builders refuse, and what decoding still reads ─────

    /** An offer built from a Price object refuses a term that is not above
     *  zero, as Core (MALFORMED) and js ("price must be positive") do. All
     *  of these used to be signed, the default Price() 0/0 included. */
    void testExactPriceMustBePositive()
    {
        const qint32 min = std::numeric_limits<qint32>::min();
        for (const Price& bad : {Price(), Price(0, 1), Price(1, 0), Price(-1, 2),
                                 Price(1, -2), Price(-1, -2), Price(min, 1), Price(min, -1)}) {
            const QByteArray what = QStringLiteral("%1/%2").arg(bad.getNumerator())
                                        .arg(bad.getDenominator()).toUtf8();
            QVERIFY2(refusedAsNotPositive([&]() {
                delete ManageSellOfferOperation::create(native(), usd(), "100", bad); }), what.constData());
            QVERIFY2(refusedAsNotPositive([&]() {
                delete new ManageSellOfferOperation(native(), usd(), "100", bad, 7); }), what.constData());
            QVERIFY2(refusedAsNotPositive([&]() {
                delete ManageBuyOfferOperation::create(native(), usd(), "100", bad); }), what.constData());
            QVERIFY2(refusedAsNotPositive([&]() {
                delete new ManageBuyOfferOperation(native(), usd(), "100", bad, 7); }), what.constData());
            QVERIFY2(refusedAsNotPositive([&]() {
                delete CreatePassiveSellOfferOperation::create(native(), usd(), "100", bad); }), what.constData());
        }
        // The smallest and largest positive prices still build.
        QScopedPointer<ManageSellOfferOperation> op(
            ManageSellOfferOperation::create(native(), usd(), "100", Price(1, 2147483647)));
        QCOMPARE(op->getPriceR(), Price(1, 2147483647));
        QScopedPointer<CreatePassiveSellOfferOperation> passive(
            CreatePassiveSellOfferOperation::create(native(), usd(), "100", Price(2147483647, 1)));
        QCOMPARE(passive->getPriceR(), Price(2147483647, 1));
    }

    /** Decoding stays lenient, so an envelope carrying such a price can
     *  still be read, and printing INT32_MIN/-1 gives 2147483648: getPrice()
     *  on such a decoded offer used to kill the process (32-bit division,
     *  STATUS_INTEGER_OVERFLOW). */
    void testDecodedOfferKeepsAnyPriceAndPrintsIt()
    {
        const qint32 min = std::numeric_limits<qint32>::min();
        stellar::ManageSellOfferOp sell{};
        sell.price.n = min;
        sell.price.d = -1;
        QScopedPointer<ManageSellOfferOperation> mso(ManageSellOfferOperation::build(sell));
        QCOMPARE(mso->getPriceR(), Price(min, -1));
        QCOMPARE(mso->getPrice(), QStringLiteral("2147483648"));

        stellar::ManageBuyOfferOp buy{};
        buy.price.n = min;
        buy.price.d = -1;
        QScopedPointer<ManageBuyOfferOperation> mbo(ManageBuyOfferOperation::build(buy));
        QCOMPARE(mbo->getPrice(), QStringLiteral("2147483648"));

        stellar::CreatePassiveSellOfferOp passive{};
        passive.price.n = -1;
        passive.price.d = 2;
        QScopedPointer<CreatePassiveSellOfferOperation> cpo(
            CreatePassiveSellOfferOperation::build(passive));
        QCOMPARE(cpo->getPriceR(), Price(-1, 2));
        QCOMPARE(cpo->getPrice(), QStringLiteral("-0.5"));
    }

    /** The default constructors left the asset pointers and the op itself
     *  uninitialised: destroying one deleted garbage (an access violation),
     *  and toXdrBase64() serialised heap bytes (0xCD... in a debug build). */
    void testDefaultConstructedOffersAreZeroedAndSafeToDestroy()
    {
        { ManageSellOfferOperation onStack; Q_UNUSED(onStack) }
        { ManageBuyOfferOperation onStack; Q_UNUSED(onStack) }

        QScopedPointer<ManageSellOfferOperation> sell(new ManageSellOfferOperation());
        QCOMPARE(sell->setOfferId(1)->toXdrBase64(),
                 QStringLiteral("AAAAAAAAAAMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQ=="));
        QVERIFY(dynamic_cast<AssetTypeNative*>(sell->getSelling()));

        QScopedPointer<ManageBuyOfferOperation> buy(new ManageBuyOfferOperation());
        QCOMPARE(buy->setOfferId(1)->toXdrBase64(),
                 QStringLiteral("AAAAAAAAAAwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQ=="));
        QVERIFY(dynamic_cast<AssetTypeNative*>(buy->getBuying()));
    }

    /** An implicit copy shared the Asset* that getSelling()/getBuying()
     *  cache, and both copies deleted it. Operations cannot be copied now. */
    void testOperationsCannotBeCopied()
    {
        // Operation is abstract, so only its assignment can be asked about.
        static_assert(!std::is_copy_assignable<Operation>::value, "Operation assign");
        static_assert(!std::is_copy_constructible<ManageSellOfferOperation>::value, "sell copy");
        static_assert(!std::is_copy_assignable<ManageSellOfferOperation>::value, "sell assign");
        static_assert(!std::is_copy_constructible<ManageBuyOfferOperation>::value, "buy copy");
        static_assert(!std::is_copy_assignable<ManageBuyOfferOperation>::value, "buy assign");
        static_assert(!std::is_copy_constructible<CreatePassiveSellOfferOperation>::value, "passive copy");
        static_assert(!std::is_copy_assignable<CreatePassiveSellOfferOperation>::value, "passive assign");
        QScopedPointer<ManageSellOfferOperation> op(
            ManageSellOfferOperation::create(native(), usd(), "100", Price(1, 2)));
        QCOMPARE(op->getSelling(), op->getSelling());      // one cached Asset, one owner
    }

    /** A whole price reads "2", not "2.", as js and java print it. */
    void testWholePriceHasNoTrailingDot()
    {
        QScopedPointer<ManageSellOfferOperation> sell(
            ManageSellOfferOperation::create(native(), usd(), "100", QString("2")));
        QCOMPARE(sell->getPrice(), QStringLiteral("2"));
        QScopedPointer<ManageBuyOfferOperation> buy(
            ManageBuyOfferOperation::create(native(), usd(), "100", Price(2147483647, 1)));
        QCOMPARE(buy->getPrice(), QStringLiteral("2147483647"));
        QScopedPointer<CreatePassiveSellOfferOperation> passive(
            CreatePassiveSellOfferOperation::create(native(), usd(), "100", Price(10, 5)));
        QCOMPARE(passive->getPrice(), QStringLiteral("2"));
    }

private:
    static bool refusedAsNotPositive(const std::function<void()>& build)
    {
        try {
            build();
        } catch (const std::runtime_error& e) {
            return QString::fromUtf8(e.what()) == QStringLiteral("price must be positive");
        }
        return false;
    }
};

ADD_TEST(ExactOfferPriceTest)
#endif // EXACTOFFERPRICETEST_H
