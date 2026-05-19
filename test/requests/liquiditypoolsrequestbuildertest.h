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
};

ADD_TEST(LiquidityPoolsRequestBuilderTest)
#endif // LIQUIDITYPOOLSREQUESTBUILDERTEST_H
