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
};

ADD_TEST(AssetsRequestBuilderTest)
#endif // ASSETSREQUESTBUILDERTEST_H



