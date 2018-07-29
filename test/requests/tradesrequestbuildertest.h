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

};

ADD_TEST(TradesRequestBuilderTest)
#endif // TRADESREQUESTBUILDERTEST_H
