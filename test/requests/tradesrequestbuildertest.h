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
    void testOrderBook() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        QUrl uri = server->trades()
                .buyingAsset(Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId("GAUPA4HERNBDPVO4IUA3MJXBCRRK5W54EVXTDK6IIUTGDQRB6D5W242W")))
                .sellingAsset(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId("GDRRHSJMHXDTQBT4JTCILNGF5AS54FEMTXL7KOLMF6TFTHRK6SSUSUZZ")))
                .cursor("13537736921089")
                .limit(200)
                .order(RequestBuilder::Order::ASC)
                .buildUri();

        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/order_book/trades?"
                     "buying_asset_type=credit_alphanum4&"
                     "buying_asset_code=EUR&"
                     "buying_asset_issuer=GAUPA4HERNBDPVO4IUA3MJXBCRRK5W54EVXTDK6IIUTGDQRB6D5W242W&"
                     "selling_asset_type=credit_alphanum4&"
                     "selling_asset_code=USD&"
                     "selling_asset_issuer=GDRRHSJMHXDTQBT4JTCILNGF5AS54FEMTXL7KOLMF6TFTHRK6SSUSUZZ&"
                     "cursor=13537736921089&"
                     "limit=200&"
                     "order=asc"));

    }

};

ADD_TEST(TradesRequestBuilderTest)
#endif // TRADESREQUESTBUILDERTEST_H
