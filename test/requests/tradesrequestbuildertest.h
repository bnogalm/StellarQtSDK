#ifndef TRADESREQUESTBUILDERTEST_H
#define TRADESREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"

class TradesRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testTrades() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        QUrl uri = server->trades()
                .baseAsset(Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId("GAUPA4HERNBDPVO4IUA3MJXBCRRK5W54EVXTDK6IIUTGDQRB6D5W242W")))
                .counterAsset(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId("GDRRHSJMHXDTQBT4JTCILNGF5AS54FEMTXL7KOLMF6TFTHRK6SSUSUZZ")))
                .cursor("13537736921089")
                .limit(200)
                .order(RequestBuilder::Order::ASC)
                .buildUri();

        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/trades?"
                     "base_asset_type=credit_alphanum4&"
                     "base_asset_code=EUR&"
                     "base_asset_issuer=GAUPA4HERNBDPVO4IUA3MJXBCRRK5W54EVXTDK6IIUTGDQRB6D5W242W&"
                     "counter_asset_type=credit_alphanum4&"
                     "counter_asset_code=USD&"
                     "counter_asset_issuer=GDRRHSJMHXDTQBT4JTCILNGF5AS54FEMTXL7KOLMF6TFTHRK6SSUSUZZ&"
                     "cursor=13537736921089&"
                     "limit=200&"
                     "order=asc"));

    }



    void testTradesForAccount() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        QUrl uri = server->trades()
                .forAccount(KeyPair::fromAccountId("GDRRHSJMHXDTQBT4JTCILNGF5AS54FEMTXL7KOLMF6TFTHRK6SSUSUZZ"))
                .cursor("13537736921089")
                .limit(200)
                .order(RequestBuilder::Order::ASC)
                .buildUri();

        QCOMPARE( uri.toString(),QString("https://horizon-testnet.stellar.org/accounts/GDRRHSJMHXDTQBT4JTCILNGF5AS54FEMTXL7KOLMF6TFTHRK6SSUSUZZ/trades?"
                                         "cursor=13537736921089&"
                                         "limit=200&"
                                         "order=asc"));

    }

    /**
     * /trades by asset pair, offer and type, and the trades of an account
     * and a pool, as js-stellar-sdk 17.1.0 builds them (the call builder's
     * url after checkFilter(), what call() requests). py-stellar-sdk 14.1.0
     * builds the same URLs, but for one offer it asks /offers/241/trades
     * where js and this SDK ask /trades?offer_id=241 (Horizon serves both).
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        QScopedPointer<Asset> xlm(new AssetTypeNative());
        QScopedPointer<Asset> usd(Asset::createNonNativeAsset("USD", QString("GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG")));
        QCOMPARE(server.trades().baseAsset(xlm.data()).counterAsset(usd.data()).limit(5).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/trades?base_asset_type=native&counter_asset_type=credit_alphanum4&counter_asset_code=USD&counter_asset_issuer=GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG&limit=5"));
        QCOMPARE(server.trades().offerId("241").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/trades?offer_id=241"));
        QCOMPARE(server.trades().tradeType("liquidity_pool").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/trades?trade_type=liquidity_pool"));
        QCOMPARE(server.trades().forAccount(QString("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB")).cursor("12").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts/GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB/trades?cursor=12"));
        QCOMPARE(server.trades().forLiquidityPool("dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/liquidity_pools/dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7/trades"));
    }
};

ADD_TEST(TradesRequestBuilderTest)
#endif // TRADESREQUESTBUILDERTEST_H
