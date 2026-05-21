#ifndef SEP11TXREPTEST_H
#define SEP11TXREPTEST_H

#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/txrep.h"
#include "../src/transaction.h"
#include "../src/transactionbuilder.h"
#include "../src/account.h"
#include "../src/managedataoperation.h"
#include "../src/paymentoperation.h"
#include "../src/assettypenative.h"
#include "../src/asset.h"
#include "../src/keypair.h"
#include "../src/network.h"
#include "../src/timebounds.h"

/** Minimal SEP-11 Txrep round-trip tests (SEP-10-style subset). */
class Sep11TxrepTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testEmitContainsExpectedLines()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 41);
        QScopedPointer<Transaction> tx(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new ManageDataOperation("auth", "deadbeef"))
                .addTimeBounds(new TimeBounds(100, 200))
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(tx.data());
        QVERIFY(txrep.contains("type: ENVELOPE_TYPE_TX"));
        QVERIFY(txrep.contains("tx.sourceAccount: GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"));
        QVERIFY(txrep.contains("tx.fee: 100"));
        QVERIFY(txrep.contains("tx.seqNum: 42"));
        QVERIFY(txrep.contains("tx.cond.type: PRECOND_TIME"));
        QVERIFY(txrep.contains("tx.cond.timeBounds.minTime: 100"));
        QVERIFY(txrep.contains("tx.cond.timeBounds.maxTime: 200"));
        QVERIFY(txrep.contains("tx.memo.type: MEMO_NONE"));
        QVERIFY(txrep.contains("tx.operations.len: 1"));
        QVERIFY(txrep.contains("tx.operations[0].body.type: MANAGE_DATA"));
        QVERIFY(txrep.contains("tx.operations[0].body.manageDataOp.dataName: \"auth\""));

        delete acc;
    }

    void testManageDataRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new ManageDataOperation("example.com auth", "value"))
                .addTimeBounds(new TimeBounds(1000, 2000))
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));

        QCOMPARE(parsed->getSourceAccount(), original->getSourceAccount());
        QCOMPARE(parsed->getFee(),           original->getFee());
        QCOMPARE(parsed->getSequenceNumber(),original->getSequenceNumber());
        QVERIFY(parsed->getTimeBounds() != nullptr);
        QCOMPARE(parsed->getTimeBounds()->getMinTime(), qint64(1000));
        QCOMPARE(parsed->getTimeBounds()->getMaxTime(), qint64(2000));
        QCOMPARE(parsed->getOperations().size(), 1);
        auto* md = dynamic_cast<ManageDataOperation*>(parsed->getOperations().at(0));
        QVERIFY(md != nullptr);
        QCOMPARE(md->getName(), QString("example.com auth"));
        QCOMPARE(md->getValue(), QByteArray("value"));

        delete acc;
    }

    void testManageDataDeletePresentFalse()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> tx(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new ManageDataOperation("delete me")) // no value → delete
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(tx.data());
        QVERIFY(txrep.contains("tx.cond.type: PRECOND_NONE"));
        QVERIFY(txrep.contains("tx.operations[0].body.manageDataOp.dataValue._present: false"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* md = dynamic_cast<ManageDataOperation*>(parsed->getOperations().at(0));
        QVERIFY(md != nullptr);
        QCOMPARE(md->getName(), QString("delete me"));
        QVERIFY(md->getValue().isEmpty());

        delete acc;
    }

    void testPaymentRoundTrip()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 99);
        QScopedPointer<Transaction> original(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(PaymentOperation::create(
                    QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"),
                    new AssetTypeNative(),
                    "10.5"))
                .setTimeout(TransactionBuilder::TIMEOUT_INFINITE)
                .setBaseFee(100)
                .build());

        QString txrep = Txrep::toTxrep(original.data());
        QVERIFY(txrep.contains("PAYMENT"));
        QVERIFY(txrep.contains("tx.operations[0].body.paymentOp.asset: XLM"));
        QVERIFY(txrep.contains("tx.operations[0].body.paymentOp.amount: 105000000"));

        QScopedPointer<Transaction> parsed(Txrep::fromTxrep(txrep, Network::testnetNetwork()));
        auto* pay = dynamic_cast<PaymentOperation*>(parsed->getOperations().at(0));
        QVERIFY(pay != nullptr);
        QCOMPARE(pay->getDestination(),
                 QString("GABXJTV7ELEB2TQZKJYEGXBUIG6QODJULKJDI65KZMIZZG2EACJU5EA7"));
        QCOMPARE(pay->getAmount(), QString("10.5"));
        QVERIFY(dynamic_cast<AssetTypeNative*>(pay->getAsset()) != nullptr);

        delete acc;
    }

    void testEmitRejectsV2Preconditions()
    {
        KeyPair* source = KeyPair::fromAccountId("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        Account* acc = new Account(source, 0);
        QScopedPointer<Transaction> tx(
            TransactionBuilder(acc, Network::testnetNetwork())
                .addOperation(new ManageDataOperation("k", "v"))
                .addTimeBounds(new TimeBounds(0, 100))
                .setMinSeqAge(60)  // forces PRECOND_V2
                .setBaseFee(100)
                .build());

        bool threw = false;
        try { Txrep::toTxrep(tx.data()); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);

        delete acc;
    }
};

ADD_TEST(Sep11TxrepTest)
#endif // SEP11TXREPTEST_H
