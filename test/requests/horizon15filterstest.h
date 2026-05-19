#ifndef HORIZON15FILTERSTEST_H
#define HORIZON15FILTERSTEST_H
#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/server.h"
#include "../../src/network.h"
#include "../../src/requests/accountsrequestbuilder.h"
#include "../../src/requests/assetsrequestbuilder.h"
#include "../../src/requests/offersrequestbuilder.h"
#include "../../src/requests/operationsrequestbuilder.h"
#include "../../src/requests/tradesrequestbuilder.h"
#include "../../src/requests/orderbookrequestbuilder.h"
#include "../../src/assettypecreditalphanum4.h"

/**
 * URL-shape tests for the Horizon-15 filters added on existing builders.
 * No network — only the generated URI is inspected.
 */
class Horizon15FiltersTest: public QObject
{
    Q_OBJECT

    static QString uri(RequestBuilder* b) { return b->buildUri().toString(); }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testAccountsForLiquidityPool()
    {
        Server s(QStringLiteral("https://horizon.stellar.org"));
        AccountsRequestBuilder b(&s);
        b.forLiquidityPool("abc1234");
        QVERIFY(uri(&b).contains("liquidity_pool=abc1234"));
    }

    void testAssetsForAsset()
    {
        Server s(QStringLiteral("https://horizon.stellar.org"));
        AssetsRequestBuilder b(&s);
        AssetTypeCreditAlphaNum4 a("USDC",
            QString("GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN"));
        b.forAsset(&a);
        QString u = uri(&b);
        QVERIFY(u.contains("asset_code=USDC"));
        QVERIFY(u.contains("asset_issuer=GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN"));
    }

    void testOffersForSeller()
    {
        Server s(QStringLiteral("https://horizon.stellar.org"));
        OffersRequestBuilder b(&s);
        b.forSeller("GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN");
        QVERIFY(uri(&b).contains("seller=GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN"));
    }

    void testOperationsForLiquidityPool()
    {
        Server s(QStringLiteral("https://horizon.stellar.org"));
        OperationsRequestBuilder b(&s);
        b.forLiquidityPool("abc1234");
        QString u = uri(&b);
        QVERIFY(u.contains("/liquidity_pools/abc1234/operations"));
    }

    void testOperationsIncludeFailed()
    {
        Server s(QStringLiteral("https://horizon.stellar.org"));
        OperationsRequestBuilder b(&s);
        b.includeFailed(true);
        QVERIFY(uri(&b).contains("include_failed=true"));
    }

    void testTradesForLiquidityPool()
    {
        Server s(QStringLiteral("https://horizon.stellar.org"));
        TradesRequestBuilder b(&s);
        b.forLiquidityPool("abc1234");
        QVERIFY(uri(&b).contains("/liquidity_pools/abc1234/trades"));
    }

    void testTradesForAccountString()
    {
        Server s(QStringLiteral("https://horizon.stellar.org"));
        TradesRequestBuilder b(&s);
        b.forAccount(QStringLiteral("GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN"));
        QVERIFY(uri(&b).contains("/accounts/GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN/trades"));
    }

    void testTradesTradeType()
    {
        Server s(QStringLiteral("https://horizon.stellar.org"));
        TradesRequestBuilder b(&s);
        b.tradeType("liquidity_pool");
        QVERIFY(uri(&b).contains("trade_type=liquidity_pool"));
    }

    void testOrderBookPaginationFlows()
    {
        // Just verify the cursor/limit/order chain still compiles and the
        // params survive in the URL. (Previously documented as "no tested".)
        Server s(QStringLiteral("https://horizon.stellar.org"));
        OrderBookRequestBuilder b(&s);
        b.cursor("c1");
        b.limit(50);
        b.order(RequestBuilder::Order::DESC);
        QString u = uri(&b);
        QVERIFY(u.contains("cursor=c1"));
        QVERIFY(u.contains("limit=50"));
        QVERIFY(u.contains("order=desc"));
    }
};

ADD_TEST(Horizon15FiltersTest)
#endif // HORIZON15FILTERSTEST_H
