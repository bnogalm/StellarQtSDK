#ifndef OPERATIONSREQUESTBUILDERTEST_H
#define OPERATIONSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"

class OperationsRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
     void testOperations() {
       Server server("https://horizon-testnet.stellar.org");
       QUrl uri = server.operations()
               .limit(200)
               .order(RequestBuilder::Order::DESC)
               .buildUri();
       QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/operations?limit=200&order=desc"));
     }
     void testForAccount() {
       Server server("https://horizon-testnet.stellar.org");
       QUrl uri = server.operations()
               .forAccount(KeyPair::fromAccountId("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"))
               .limit(200)
               .order(RequestBuilder::Order::DESC)
               .buildUri();
       QCOMPARE( uri.toString(),QString("https://horizon-testnet.stellar.org/accounts/GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H/operations?limit=200&order=desc"));
     }     
     void testForClaimableBalance() {
       Server server("https://horizon-testnet.stellar.org");
       QUrl uri = server.operations()
               .forClaimableBalance("00000000846c047755e4a46912336f56096b48ece78ddb5fbf6d90f0eb4ecae5324fbddb")
               .limit(200)
               .order(RequestBuilder::Order::DESC)
               .buildUri();
       QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/claimable_balances/00000000846c047755e4a46912336f56096b48ece78ddb5fbf6d90f0eb4ecae5324fbddb/operations?limit=200&order=desc"));
     }
     void testForLedger() {
       Server server("https://horizon-testnet.stellar.org");
       QUrl uri = server.operations()
               .forLedger(200000000000L)
               .limit(50)
               .order(RequestBuilder::Order::ASC)
               .buildUri();
       QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/ledgers/200000000000/operations?limit=50&order=asc"));
     }
     void testForTransaction() {
       Server server("https://horizon-testnet.stellar.org");
       QUrl uri = server.operations()
               .forTransaction("991534d902063b7715cd74207bef4e7bd7aa2f108f62d3eba837ce6023b2d4f3")
               .buildUri();
       QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/transactions/991534d902063b7715cd74207bef4e7bd7aa2f108f62d3eba837ce6023b2d4f3/operations"));
   }

     void testIncludeFailed() {
         Server server("https://horizon-testnet.stellar.org");
         QUrl uri = server.operations()
                 .forLedger(200000000000L)
                 .includeFailed(true)
                 .limit(50)
                 .order(RequestBuilder::Order::ASC)
                 .buildUri();
         QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/ledgers/200000000000/operations?include_failed=true&limit=50&order=asc"));
     }

};

ADD_TEST(OperationsRequestBuilderTest)
#endif // OPERATIONSREQUESTBUILDERTEST_H
