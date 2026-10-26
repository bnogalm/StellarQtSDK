#ifndef TRANSACTIONSREQUESTBUILDERTEST_H
#define TRANSACTIONSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"

class TransactionsRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }

    void testTransactions() {
        Server *server = new Server("https://horizon-testnet.stellar.org");
        QUrl uri = server->transactions()
                .limit(200)
                .order(RequestBuilder::Order::DESC)
                .buildUri();
        QCOMPARE(QString("https://horizon-testnet.stellar.org/transactions?limit=200&order=desc"), uri.toString());
    }

    void testForAccount() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        QUrl uri = server->transactions()
                .forAccount(KeyPair::fromAccountId(QString("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H")))
                .limit(200)
                .order(RequestBuilder::Order::DESC)
                .buildUri();
        QCOMPARE(QString("https://horizon-testnet.stellar.org/accounts/GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H/transactions?limit=200&order=desc"), uri.toString());
    }

    void testForClaimableBalance() {
      Server server("https://horizon-testnet.stellar.org");
      QUrl uri = server.transactions()
              .forClaimableBalance("00000000846c047755e4a46912336f56096b48ece78ddb5fbf6d90f0eb4ecae5324fbddb")
              .limit(200)
              .order(RequestBuilder::Order::DESC)
              .buildUri();
      QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/claimable_balances/00000000846c047755e4a46912336f56096b48ece78ddb5fbf6d90f0eb4ecae5324fbddb/transactions?limit=200&order=desc"));
    }
    void testForLedger() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        QUrl uri = server->transactions()
                .forLedger(200000000000L)
                .limit(50)
                .order(RequestBuilder::Order::ASC)
                .buildUri();
        QCOMPARE(QString("https://horizon-testnet.stellar.org/ledgers/200000000000/transactions?limit=50&order=asc"), uri.toString());
    }

    void testIncludeFailed() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.transactions()
                .forLedger(200000000000L)
                .includeFailed(true)
                .limit(50)
                .order(RequestBuilder::Order::ASC)
                .buildUri();
        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/ledgers/200000000000/transactions?include_failed=true&limit=50&order=asc"));
    }

    /** A pool's transactions, at py's for_liquidity_pool endpoint (js's forLiquidityPool). */
    void testForLiquidityPool() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.transactions()
                .forLiquidityPool("dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7")
                .includeFailed(true)
                .buildUri();
        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/liquidity_pools/dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7/transactions?include_failed=true"));
    }

    /** An order is "asc" or "desc": anything else (a typo, "ASC") read as "desc", the order reversed. */
    void testOrderFromStringIsExact()
    {
        Server server("https://horizon-testnet.stellar.org");
        RequestBuilder builder(&server);
        QVERIFY(builder.orderFromString("asc") == RequestBuilder::Order::ASC);
        QVERIFY(builder.orderFromString("desc") == RequestBuilder::Order::DESC);
        QVERIFY_EXCEPTION_THROWN(builder.orderFromString("ASC"), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(builder.orderFromString("ascending"), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(builder.orderFromString(QString()), std::runtime_error);
    }

    /**
     * The transactions of an account, ledger, claimable balance and pool,
     * and /transactions/{hash}, as
     * js-stellar-sdk 17.1.0 (the call builder's url after checkFilter(),
     * what call() requests) and py-stellar-sdk 14.1.0 (the builder's url and
     * params as requests encodes them) build these URLs, the same in both.
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        QScopedPointer<KeyPair> account(KeyPair::fromAccountId("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB"));
        QCOMPARE(server.transactions().forAccount(account.data()).includeFailed(true).limit(3).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts/GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB/transactions?include_failed=true&limit=3"));
        QCOMPARE(server.transactions().forLedger(123456).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/ledgers/123456/transactions"));
        QCOMPARE(server.transactions().forClaimableBalance("00000000929b20b72e5890ab51c24f1cc46fa01c4f318d8d33367d24dd614cfdf5491072").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/claimable_balances/00000000929b20b72e5890ab51c24f1cc46fa01c4f318d8d33367d24dd614cfdf5491072/transactions"));
        QCOMPARE(server.transactions().forLiquidityPool("dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/liquidity_pools/dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7/transactions"));
        // The single transaction: the request is dropped before the event loop sends it.
        TransactionsRequestBuilder one = server.transactions();
        delete one.transaction(QString("5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b"));
        QCOMPARE(one.requestUri().toString(), QString("https://horizon-testnet.stellar.org/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b"));
    }
};

ADD_TEST(TransactionsRequestBuilderTest)
#endif // TRANSACTIONSREQUESTBUILDERTEST_H
