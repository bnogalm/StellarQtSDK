#ifndef ROOTREQUESTBUILDTEST_H
#define ROOTREQUESTBUILDTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"

class RootRequestBuildTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testRoot()
    {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.root()
        .buildUri();
        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/"));
    }

    /**
     * A Horizon URL with a path ("https://example.com/horizon") keeps it in
     * front of every endpoint, as js-stellar-sdk 17.1.0 builds these URLs
     * (the call builder's url after checkFilter(), what call() requests).
     * py-stellar-sdk 14.1.0 keeps the path too and sends the same
     * parameters with the same values, but strict-send puts source_amount
     * first.
     */
    void testUrlWithPathMatchesJsAndPy()
    {
        Server server("https://example.com/horizon");
        AssetTypeNative xlm;
        QCOMPARE(server.accounts().limit(1).buildUri().toString(),
                 QString("https://example.com/horizon/accounts?limit=1"));
        QCOMPARE(server.effects().forTransaction("5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b").buildUri().toString(),
                 QString("https://example.com/horizon/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b/effects"));
        QCOMPARE(server.operationFeeStats().buildUri().toString(),
                 QString("https://example.com/horizon/fee_stats"));
        QCOMPARE(server.strictSendPaths().sourceAsset(&xlm).sourceAmount("10")
                     .destinationAccount(KeyPair("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN")).buildUri().toString(),
                 QString("https://example.com/horizon/paths/strict-send?source_asset_type=native&source_amount=10&destination_account=GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN"));
    }
};

ADD_TEST(RootRequestBuildTest)
#endif // ROOTREQUESTBUILDTEST_H
