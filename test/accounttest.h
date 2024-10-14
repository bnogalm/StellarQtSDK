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
#include "../src/responses/submittransactionresponse.h"
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
    void testGetters() {
        KeyPair* keypair = KeyPair::random();
        Account* account = new Account(keypair, 100L);
        QVERIFY(account->getKeypair()->getAccountId()== keypair->getAccountId());
        QVERIFY(account->getSequenceNumber()== 100L);
    }
};

ADD_TEST(AccountTest)
#endif // ACCOUNTTEST_H
