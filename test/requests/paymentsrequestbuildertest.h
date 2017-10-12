#ifndef PAYMENTSREQUESTBUILDERTEST_H
#define PAYMENTSREQUESTBUILDERTEST_H

#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"

class PaymentsRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testPayments() {
       Server *server = new Server("https://horizon-testnet.stellar.org");
       QUrl uri = server->payments()
               .limit(200)
               .order(RequestBuilder::Order::DESC)
               .buildUri();
       QCOMPARE(QString("https://horizon-testnet.stellar.org/payments?limit=200&order=desc"), uri.toString());
     }

     void testForAccount() {
       Server *server = new Server("https://horizon-testnet.stellar.org");
       QUrl uri = server->payments()
               .forAccount(KeyPair::fromAccountId("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"))
               .limit(200)
               .order(RequestBuilder::Order::DESC)
               .buildUri();
       QCOMPARE(QString("https://horizon-testnet.stellar.org/accounts/GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H/payments?limit=200&order=desc"), uri.toString());
     }

     void testForLedger() {
       Server *server = new Server("https://horizon-testnet.stellar.org");
       QUrl uri = server->payments()
               .forLedger(200000000000L)
               .limit(50)
               .order(RequestBuilder::Order::ASC)
               .buildUri();
       QCOMPARE(QString("https://horizon-testnet.stellar.org/ledgers/200000000000/payments?limit=50&order=asc"), uri.toString());
     }

     void testForTransaction() {
       Server *server = new Server("https://horizon-testnet.stellar.org");
       QUrl uri = server->payments()
               .forTransaction("991534d902063b7715cd74207bef4e7bd7aa2f108f62d3eba837ce6023b2d4f3")
               .buildUri();
       QCOMPARE(QString("https://horizon-testnet.stellar.org/transactions/991534d902063b7715cd74207bef4e7bd7aa2f108f62d3eba837ce6023b2d4f3/payments"), uri.toString());
   }
};

ADD_TEST(PaymentsRequestBuilderTest)
#endif // PAYMENTSREQUESTBUILDERTEST_H
