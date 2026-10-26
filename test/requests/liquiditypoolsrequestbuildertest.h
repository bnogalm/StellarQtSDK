#ifndef LIQUIDITYPOOLSREQUESTBUILDERTEST_H
#define LIQUIDITYPOOLSREQUESTBUILDERTEST_H
#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/server.h"
#include "../../src/network.h"
#include "../../src/requests/liquiditypoolsrequestbuilder.h"
#include "../../src/asset.h"
#include "../../src/assettypenative.h"
#include "../../src/assettypecreditalphanum4.h"

/**
 * URL-shape tests for `/liquidity_pools` builder. No network — we inspect
 * the URI built by `buildUri()`.
 */
class LiquidityPoolsRequestBuilderTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testListAllUri()
    {
        Server s(QStringLiteral("https://horizon.stellar.org"));
        LiquidityPoolsRequestBuilder b(&s);
        b.cursor("13537736921089");
        b.limit(15);
        b.order(RequestBuilder::Order::ASC);
        QUrl uri = b.buildUri();
        QString s2 = uri.toString();
        QVERIFY(s2.contains("/liquidity_pools"));
        QVERIFY(s2.contains("cursor=13537736921089"));
        QVERIFY(s2.contains("limit=15"));
        QVERIFY(s2.contains("order=asc"));
    }

    void testForReservesUri()
    {
        Server s(QStringLiteral("https://horizon.stellar.org"));
        LiquidityPoolsRequestBuilder b(&s);
        QList<Asset*> reserves;
        reserves.append(new AssetTypeNative());
        reserves.append(new AssetTypeCreditAlphaNum4(
            "USDC",
            QString("GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN")));
        b.forReserves(reserves);
        QString s2 = b.buildUri().toString();
        QVERIFY(s2.contains("/liquidity_pools"));
        QVERIFY(s2.contains("reserves="));
        // Built canonical form is "native,USDC:GA5..."
        QVERIFY(s2.contains("native"));
        QVERIFY(s2.contains("USDC"));
        qDeleteAll(reserves);
    }

    void testForAccountUri()
    {
        Server s(QStringLiteral("https://horizon.stellar.org"));
        LiquidityPoolsRequestBuilder b(&s);
        b.forAccount("GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN");
        QString s2 = b.buildUri().toString();
        QVERIFY(s2.contains("/liquidity_pools"));
        QVERIFY(s2.contains("account=GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN"));
    }

    /**
     * /liquidity_pools by reserves and by account, and
     * /liquidity_pools/{id}, as
     * js-stellar-sdk 17.1.0 (the call builder's url after checkFilter(),
     * what call() requests) and py-stellar-sdk 14.1.0 (the builder's url and
     * params as requests encodes them) build these URLs, the same in both.
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        QScopedPointer<Asset> xlm(new AssetTypeNative());
        QScopedPointer<Asset> usd(Asset::createNonNativeAsset("USD", QString("GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG")));
        QCOMPARE(server.liquidityPools().forReserves(QList<Asset*>() << xlm.data() << usd.data()).limit(10).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/liquidity_pools?reserves=native%2CUSD%3AGDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG&limit=10"));
        QCOMPARE(server.liquidityPools().forAccount(QString("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB")).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/liquidity_pools?account=GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB"));
        // The single pool: the request is dropped before the event loop sends it.
        LiquidityPoolsRequestBuilder one = server.liquidityPools();
        delete one.liquidityPool("dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7");
        QCOMPARE(one.requestUri().toString(), QString("https://horizon-testnet.stellar.org/liquidity_pools/dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7"));
    }
};

ADD_TEST(LiquidityPoolsRequestBuilderTest)
#endif // LIQUIDITYPOOLSREQUESTBUILDERTEST_H
