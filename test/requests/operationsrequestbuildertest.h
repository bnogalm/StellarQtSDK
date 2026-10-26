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

    /**
     * The operations of an account, ledger, transaction, claimable balance
     * and pool, with include_failed, and /operations/{id}, as
     * js-stellar-sdk 17.1.0 (the call builder's url after checkFilter(),
     * what call() requests) and py-stellar-sdk 14.1.0 (the builder's url and
     * params as requests encodes them) build these URLs, the same in both.
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        QScopedPointer<KeyPair> account(KeyPair::fromAccountId("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB"));
        QCOMPARE(server.operations().forAccount(account.data()).includeFailed(true).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts/GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB/operations?include_failed=true"));
        QCOMPARE(server.operations().forLedger(123456).includeFailed(false).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/ledgers/123456/operations?include_failed=false"));
        QCOMPARE(server.operations().forTransaction("5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b/operations"));
        QCOMPARE(server.operations().forClaimableBalance("00000000929b20b72e5890ab51c24f1cc46fa01c4f318d8d33367d24dd614cfdf5491072").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/claimable_balances/00000000929b20b72e5890ab51c24f1cc46fa01c4f318d8d33367d24dd614cfdf5491072/operations"));
        QCOMPARE(server.operations().forLiquidityPool("dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/liquidity_pools/dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7/operations"));
        // The single operation: the request is dropped before the event loop
        // sends it. It asked /operation/{id}, a route Horizon does not have.
        OperationsRequestBuilder one = server.operations();
        delete one.operation(Q_INT64_C(77309415424));
        QCOMPARE(one.requestUri().toString(), QString("https://horizon-testnet.stellar.org/operations/77309415424"));
    }
};

ADD_TEST(OperationsRequestBuilderTest)
#endif // OPERATIONSREQUESTBUILDERTEST_H
