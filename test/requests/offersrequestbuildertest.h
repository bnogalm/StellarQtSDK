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

    /**
     * /offers with seller, selling, buying and sponsor, an account's
     * offers, and /offers/{id}, as
     * js-stellar-sdk 17.1.0 (the call builder's url after checkFilter(),
     * what call() requests) and py-stellar-sdk 14.1.0 (the builder's url and
     * params as requests encodes them) build these URLs, the same in both.
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        QScopedPointer<KeyPair> account(KeyPair::fromAccountId("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB"));
        // forSponsor(QString) is private: the sponsor goes in as a KeyPair.
        QScopedPointer<KeyPair> sponsor(KeyPair::fromAccountId("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN"));
        QScopedPointer<Asset> xlm(new AssetTypeNative());
        QScopedPointer<Asset> usd(Asset::createNonNativeAsset("USD", QString("GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG")));
        QCOMPARE(server.offers().forAccount(account.data()).limit(4).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts/GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB/offers?limit=4"));
        QCOMPARE(server.offers().forSeller("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB")
                     .selling(usd.data()).buying(xlm.data())
                     .forSponsor(sponsor.data()).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/offers?seller=GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB&selling_asset_type=credit_alphanum4&selling_asset_code=USD&selling_asset_issuer=GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG&buying_asset_type=native&sponsor=GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN"));
        // The single offer: the request is dropped before the event loop sends it.
        OffersRequestBuilder one = server.offers();
        delete one.offer(Q_INT64_C(241));
        QCOMPARE(one.requestUri().toString(), QString("https://horizon-testnet.stellar.org/offers/241"));
    }
};

ADD_TEST(OffersRequestBuilderTest)
#endif // OFFERSREQUESTBUILDERTEST_H
