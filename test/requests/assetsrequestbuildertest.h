#ifndef ASSETSREQUESTBUILDERTEST_H
#define ASSETSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"

class AssetsRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testAccounts()
    {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.assets()
        .assetCode("USD")
        .assetIssuer("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN")
        .cursor("13537736921089")
        .limit(200)
        .order(RequestBuilder::Order::ASC)
        .buildUri();
        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/assets?asset_code=USD&asset_issuer=GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN&cursor=13537736921089&limit=200&order=asc"));
    }

    /**
     * /assets by code and issuer, as
     * js-stellar-sdk 17.1.0 (the call builder's url after checkFilter(),
     * what call() requests) and py-stellar-sdk 14.1.0 (the builder's url and
     * params as requests encodes them) build it, the same in both.
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        QCOMPARE(server.assets().assetCode("USD").assetIssuer("GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG")
                     .limit(5).order(RequestBuilder::Order::DESC).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/assets?asset_code=USD&asset_issuer=GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG&limit=5&order=desc"));
    }
};

ADD_TEST(AssetsRequestBuilderTest)
#endif // ASSETSREQUESTBUILDERTEST_H



