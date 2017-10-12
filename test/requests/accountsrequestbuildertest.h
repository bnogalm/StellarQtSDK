#ifndef ACCOUNTSREQUESTBUILDERTEST_H
#define ACCOUNTSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"

class AccountRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testAccounts() {
        Server* server = new Server("https://horizon-testnet.stellar.org");

        QUrl uri = server->accounts()
                .cursor("13537736921089")
                .limit(200)
                .order(RequestBuilder::Order::ASC)
                .buildUri();

        QString expected("https://horizon-testnet.stellar.org/accounts?cursor=13537736921089&limit=200&order=asc");
        QVERIFY(uri.toString() == expected);

    }

};

ADD_TEST(AccountRequestBuilderTest)
#endif // ACCOUNTSREQUESTBUILDERTEST_H
