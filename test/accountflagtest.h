#ifndef ACCOUNTFLAGTEST_H
#define ACCOUNTFLAGTEST_H
#include <QObject>
#include "src/strkey.h"
#include "src/util.h"

#include <QtTest>
#include "testcollector.h"
#include "../src/account.h"

class AccountFlagTest: public QObject
{
    Q_OBJECT


private slots:
    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testValues() {
        QCOMPARE((int)Account::AccountFlag::AUTH_REQUIRED,1);
        QCOMPARE((int)Account::AccountFlag::AUTH_REVOCABLE,2);
        QCOMPARE((int)Account::AccountFlag::AUTH_IMMUTABLE,4);
    }

};

ADD_TEST(AccountFlagTest)
#endif // ACCOUNTFLAGTEST_H
