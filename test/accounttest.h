#ifndef ACCOUNTTEST_H
#define ACCOUNTTEST_H
#include <QObject>
#include "src/keypair.h"
#include "src/util.h"

#include <QtTest>
#include "testcollector.h"
#include "../src/server.h"
#include "../src/network.h"
#include "../src/transaction.h"
#include "../src/account.h"
#include "../src/createaccountoperation.h"
#include "../src/bumpsequenceoperation.h"
#include "../src/transactionbuilder.h"
#include "../src/responses/submittransactionresponse.h"
#include <functional>
#include <limits>
class AccountTest: public QObject
{
    Q_OBJECT

private slots:

    void initTestCase()
    {
    }
    void cleanupTestCase()
    {
    }
    void testNullArguments() {
        try {
            new Account(0l, 10L);
            QFAIL("expected exception");
        } catch (const std::exception& e) {
            Q_UNUSED(e)
        }
        //we dont have allow null counter
//        try {
//            new Account(KeyPair::random(), 0);
//            QFAIL("expected exception");
//        } catch (std::exception e) {}
    }


    void testGetIncrementedSequenceNumber() {
        Account* account = new Account(KeyPair::random(), 100L);
        qint64 incremented;
        incremented = account->getIncrementedSequenceNumber();
        QVERIFY(100L == account->getSequenceNumber());
        QVERIFY(101L== incremented);
        incremented = account->getIncrementedSequenceNumber();
        QVERIFY(100L== account->getSequenceNumber());
        QVERIFY(101L== incremented);
    }
    void testIncrementSequenceNumber() {
        Account* account = new Account(KeyPair::random(), 100L);
        account->incrementSequenceNumber();
        QVERIFY(account->getSequenceNumber()== 101L);
    }
    void testDecrementSequenceNumber() {
        // S3: roll back the local sequence after a build whose tx was not submitted.
        Account* account = new Account(KeyPair::random(), 100L);
        account->incrementSequenceNumber();
        QVERIFY(account->getSequenceNumber()== 101L);
        account->decrementSequenceNumber();
        QVERIFY(account->getSequenceNumber()== 100L);
    }
    void testGetters() {
        KeyPair* keypair = KeyPair::random();
        Account* account = new Account(keypair, 100L);
        QVERIFY(account->getKeypair()->getAccountId()== keypair->getAccountId());
        QVERIFY(account->getSequenceNumber()== 100L);
    }

    /** At INT64_MAX the +1 overflowed: build() signed a transaction with
     *  sequence INT64_MIN and left the account there. */
    void testTheSequenceNumberDoesNotWrap() {
        const qint64 max = std::numeric_limits<qint64>::max();
        Account account(KeyPair::random(), max);
        auto throws = [](const std::function<void()>& f) {
            try { f(); return false; } catch (const std::exception&) { return true; }
        };
        QVERIFY(throws([&] { account.getIncrementedSequenceNumber(); }));
        QVERIFY(throws([&] { account.incrementSequenceNumber(); }));
        QCOMPARE(account.getSequenceNumber(), max);

        QVERIFY(throws([&] {
            TransactionBuilder builder(&account, Network::testnetNetwork());
            builder.addOperation(BumpSequenceOperation::create(qint64(2)))
                   .setTimeout(180).setBaseFee(100);
            delete builder.build();
        }));
        QCOMPARE(account.getSequenceNumber(), max);

        // One below still signs INT64_MAX itself.
        Account last(KeyPair::random(), max - 1);
        QScopedPointer<Transaction> tx(TransactionBuilder(&last, Network::testnetNetwork())
            .addOperation(BumpSequenceOperation::create(qint64(2)))
            .setTimeout(180).setBaseFee(100).build());
        QCOMPARE(tx->getSequenceNumber(), max);
        QCOMPARE(last.getSequenceNumber(), max);

        Account lowest(KeyPair::random(), std::numeric_limits<qint64>::min());
        QVERIFY(throws([&] { lowest.decrementSequenceNumber(); }));
        QCOMPARE(lowest.getSequenceNumber(), std::numeric_limits<qint64>::min());
    }
};

ADD_TEST(AccountTest)
#endif // ACCOUNTTEST_H
