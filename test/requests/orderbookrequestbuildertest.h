#ifndef ORDERBOOKREQUESTBUILDERTEST_H
#define ORDERBOOKREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"
#include "../../src/asset.h"
class OrderBookRequestBuilderTest: public QObject
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
      Server server("https://horizon-testnet.stellar.org");
      QUrl uri = server.orderBook()
              .buyingAsset(Asset::createNonNativeAsset("EUR", KeyPair::fromAccountId(QString("GAUPA4HERNBDPVO4IUA3MJXBCRRK5W54EVXTDK6IIUTGDQRB6D5W242W"))))
              .sellingAsset(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(QString("GDRRHSJMHXDTQBT4JTCILNGF5AS54FEMTXL7KOLMF6TFTHRK6SSUSUZZ"))))
              .buildUri();

      QCOMPARE(uri.toString(),
        QString("https://horizon-testnet.stellar.org/order_book?"
        "buying_asset_type=credit_alphanum4&"
        "buying_asset_code=EUR&"
        "buying_asset_issuer=GAUPA4HERNBDPVO4IUA3MJXBCRRK5W54EVXTDK6IIUTGDQRB6D5W242W&"
        "selling_asset_type=credit_alphanum4&"
        "selling_asset_code=USD&"
        "selling_asset_issuer=GDRRHSJMHXDTQBT4JTCILNGF5AS54FEMTXL7KOLMF6TFTHRK6SSUSUZZ"));
  }

};

ADD_TEST(OrderBookRequestBuilderTest)
#endif // ORDERBOOKREQUESTBUILDERTEST_H
