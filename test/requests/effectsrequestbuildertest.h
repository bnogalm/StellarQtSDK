#ifndef EFFECTSREQUESTBUILDERTEST_H
#define EFFECTSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"

class EffectsRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }

    void testEffects() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.effects()
                .limit(200)
                .order(RequestBuilder::Order::DESC)
                .buildUri();
        QCOMPARE( uri.toString(), QString("https://horizon-testnet.stellar.org/effects?limit=200&order=desc"));
    }


    void testForAccount() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.effects()
                .forAccount(KeyPair::fromAccountId("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"))
                .limit(200)
                .order(RequestBuilder::Order::DESC)
                .buildUri();
        QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/accounts/GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H/effects?limit=200&order=desc"));
    }


    void testForLedger() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.effects()
                .forLedger(200000000000L)
                .limit(50)
                .order(RequestBuilder::Order::ASC)
                .buildUri();
        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/ledgers/200000000000/effects?limit=50&order=asc"));
    }


    void testForTransaction() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.effects()
                .forTransaction("991534d902063b7715cd74207bef4e7bd7aa2f108f62d3eba837ce6023b2d4f3")
                .buildUri();
        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/transactions/991534d902063b7715cd74207bef4e7bd7aa2f108f62d3eba837ce6023b2d4f3/effects"));
    }


    void testForOperation() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.effects()
                .forOperation(28798257847L)
                .cursor("85794837")
                .buildUri();
        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/operations/28798257847/effects?cursor=85794837"));
    }

    /** A pool's effects, at py's for_liquidity_pool endpoint (js's forLiquidityPool). */
    void testForLiquidityPool() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.effects()
                .forLiquidityPool("dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7")
                .limit(20)
                .buildUri();
        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/liquidity_pools/dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7/effects?limit=20"));
    }

    /**
     * /effects and the effects of an account, ledger, transaction,
     * operation and pool, as
     * js-stellar-sdk 17.1.0 (the call builder's url after checkFilter(),
     * what call() requests) and py-stellar-sdk 14.1.0 (the builder's url and
     * params as requests encodes them) build these URLs, the same in both.
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        QScopedPointer<KeyPair> account(KeyPair::fromAccountId("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB"));
        QCOMPARE(server.effects().cursor("now").order(RequestBuilder::Order::ASC).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/effects?cursor=now&order=asc"));
        QCOMPARE(server.effects().forAccount(account.data()).limit(2).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts/GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB/effects?limit=2"));
        QCOMPARE(server.effects().forLedger(123456).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/ledgers/123456/effects"));
        QCOMPARE(server.effects().forTransaction("5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b/effects"));
        QCOMPARE(server.effects().forOperation(Q_INT64_C(77309415424)).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/operations/77309415424/effects"));
        QCOMPARE(server.effects().forLiquidityPool("dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/liquidity_pools/dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7/effects"));
    }
};

ADD_TEST(EffectsRequestBuilderTest)
#endif // EFFECTSREQUESTBUILDERTEST_H
