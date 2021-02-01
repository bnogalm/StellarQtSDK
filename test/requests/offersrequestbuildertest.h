#ifndef OFFERSREQUESTBUILDERTEST_H
#define OFFERSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"
#include "../../src/asset.h"
class OffersRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testForAccount() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.offers()
                .forAccount(KeyPair::fromAccountId(QString("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H")))
                .limit(200)
                .order(RequestBuilder::Order::DESC)
                .buildUri();
        QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/accounts/GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H/offers?limit=200&order=desc"));
    }
    void testSeller() {
        Server server("https://horizon-testnet.stellar.org");
        auto seller = KeyPair::fromAccountId("GA2HGBJIJKI6O4XEM7CZWY5PS6GKSXL6D34ERAJYQSPYA6X6AI7HYW36");
        QUrl uri = server.offers()
                .seller(seller)
                .limit(200)
                .order(RequestBuilder::Order::DESC)
                .buildUri();
        QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/offers?seller=GA2HGBJIJKI6O4XEM7CZWY5PS6GKSXL6D34ERAJYQSPYA6X6AI7HYW36&limit=200&order=desc"));
    }

    void testForSponsor() {
      Server server("https://horizon-testnet.stellar.org");
      auto sponsor = KeyPair::fromAccountId("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H");
      QUrl uri = server.offers().forSponsor(sponsor).buildUri();
      QCOMPARE(uri.toString(),"https://horizon-testnet.stellar.org/offers?sponsor=GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H");
    }
    void testSellingNative(){
        Server server("https://horizon-testnet.stellar.org");
        auto native = new AssetTypeNative();
        QUrl uri = server.offers()
                .selling(native)
                .buildUri();
        QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/offers?selling_asset_type=native"));
    }
    void testSellingCredit(){
        Server server("https://horizon-testnet.stellar.org");
        auto issuer = KeyPair::fromAccountId("GA2HGBJIJKI6O4XEM7CZWY5PS6GKSXL6D34ERAJYQSPYA6X6AI7HYW36");
        auto asset = Asset::createNonNativeAsset("USD",issuer);
        QUrl uri = server.offers()
                .selling(asset)
                .buildUri();
        QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/offers?selling_asset_type=credit_alphanum4&selling_asset_code=USD&selling_asset_issuer=GA2HGBJIJKI6O4XEM7CZWY5PS6GKSXL6D34ERAJYQSPYA6X6AI7HYW36"));
    }
    void testBuyingNative(){
        Server server("https://horizon-testnet.stellar.org");
        auto native = new AssetTypeNative();
        QUrl uri = server.offers()
                .buying(native)
                .buildUri();
        QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/offers?buying_asset_type=native"));
    }
    void testBuyingCredit(){
        Server server("https://horizon-testnet.stellar.org");
        auto issuer = KeyPair::fromAccountId("GA2HGBJIJKI6O4XEM7CZWY5PS6GKSXL6D34ERAJYQSPYA6X6AI7HYW36");
        auto asset = Asset::createNonNativeAsset("USD",issuer);
        QUrl uri = server.offers()
                .buying(asset)
                .buildUri();
        QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/offers?buying_asset_type=credit_alphanum4&buying_asset_code=USD&buying_asset_issuer=GA2HGBJIJKI6O4XEM7CZWY5PS6GKSXL6D34ERAJYQSPYA6X6AI7HYW36"));
    }

};

ADD_TEST(OffersRequestBuilderTest)
#endif // OFFERSREQUESTBUILDERTEST_H
