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

    /**
     * /order_book for an alphanum4 and an alphanum12 pair, as
     * js-stellar-sdk 17.1.0 (the call builder's url after checkFilter(),
     * what call() requests) and py-stellar-sdk 14.1.0 (the builder's url and
     * params as requests encodes them) build these URLs, the same in both (selling first, as js's
     * orderbook(selling, buying) and py's orderbook(selling, buying) set it).
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        QScopedPointer<Asset> xlm(new AssetTypeNative());
        QScopedPointer<Asset> usd(Asset::createNonNativeAsset("USD", QString("GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG")));
        QScopedPointer<Asset> banana(Asset::createNonNativeAsset("BANANA", QString("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB")));
        QCOMPARE(server.orderBook().sellingAsset(usd.data()).buyingAsset(xlm.data()).limit(20).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/order_book?selling_asset_type=credit_alphanum4&selling_asset_code=USD&selling_asset_issuer=GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG&buying_asset_type=native&limit=20"));
        QCOMPARE(server.orderBook().sellingAsset(xlm.data()).buyingAsset(banana.data()).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/order_book?selling_asset_type=native&buying_asset_type=credit_alphanum12&buying_asset_code=BANANA&buying_asset_issuer=GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB"));
    }
};

ADD_TEST(OrderBookRequestBuilderTest)
#endif // ORDERBOOKREQUESTBUILDERTEST_H
