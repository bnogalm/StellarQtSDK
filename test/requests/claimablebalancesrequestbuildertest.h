#ifndef CLAIMABLEBALANCESREQUESTBUILDERTEST_H
#define CLAIMABLEBALANCESREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../fakeserver.h"
#include "../testcollector.h"
#include "../../src/server.h"

#include "responses/claimablebalanceresponse.h"

class ClaimableBalancesRequestBuilderTest: public QObject
{
    Q_OBJECT

    QByteArray json = "{\n"
        "  \"_links\": {\n"
        "    \"self\": {\n"
        "      \"href\": \"https://horizon-protocol14.stellar.org/claimable_balances/00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd2\"\n"
        "    }\n"
        "  },\n"
        "  \"id\": \"00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd2\",\n"
        "  \"asset\": \"COP:GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\",\n"
        "  \"amount\": \"1000.0000000\",\n"
        "  \"sponsor\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\",\n"
        "  \"last_modified_ledger\": 117663,\n"
        "  \"claimants\": [\n"
        "    {\n"
        "      \"destination\": \"GD3W2FFQQ2WTJTBHXYBLMQS664XCT3LYSRKUBWNU7K4KXBZDBLPY3RAU\",\n"
        "      \"predicate\": {\n"
        "        \"abs_before\": \"2020-09-29T23:56:04Z\"\n"
        "      }\n"
        "    },\n"
        "    {\n"
        "      \"destination\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\",\n"
        "      \"predicate\": {\n"
        "        \"or\": [\n"
        "          {\n"
        "            \"abs_before\": \"2020-09-28T17:57:04Z\"\n"
        "          },\n"
        "          {\n"
        "            \"not\": {\n"
        "              \"abs_before\": \"2020-09-29T23:56:04Z\"\n"
        "            }\n"
        "          }\n"
        "        ]\n"
        "      }\n"
        "    }\n"
        "  ],\n"
        "  \"paging_token\": \"117663-00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd2\"\n"
        "}";
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {
        qApp->processEvents();
    }
    void testForSponsor() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        QUrl uri = server->claimableBalances()
            .forSponsor("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H")
            .limit(200)
            .order(RequestBuilder::Order::DESC)
            .buildUri();
        QCOMPARE(uri.toString(),"https://horizon-testnet.stellar.org/claimable_balances?sponsor=GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H&limit=200&order=desc");
      }

      void testWithoutParams() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        QUrl uri = server->claimableBalances().buildUri();
        QCOMPARE(uri.toString(),"https://horizon-testnet.stellar.org/claimable_balances");
      }

      void testForClaimant() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        QUrl uri = server->claimableBalances().forClaimant("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H").buildUri();
        QCOMPARE(uri.toString(),"https://horizon-testnet.stellar.org/claimable_balances?claimant=GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H");
      }

       void testForNativeAsset() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        QUrl uri = server->claimableBalances().forAsset(Asset::create("native")).buildUri();
        QCOMPARE(uri.toString(),"https://horizon-testnet.stellar.org/claimable_balances?asset=native");
      }

      void testForAsset() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        QUrl uri = server->claimableBalances().forAsset(Asset::create("USD:GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H")).buildUri();
        QCOMPARE(uri.toString(),"https://horizon-testnet.stellar.org/claimable_balances?asset=USD%3AGBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H");
      }

      void testForAssetAndClaimant() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        QUrl uri = server->claimableBalances().forClaimant("GCNY5OXYSY4FKHOPT2SPOQZAOEIGXB5LBYW3HVU3OWSTQITS65M5RCNY").forAsset(Asset::create("USD:GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H")).buildUri();
        QCOMPARE(uri.toString(),"https://horizon-testnet.stellar.org/claimable_balances?claimant=GCNY5OXYSY4FKHOPT2SPOQZAOEIGXB5LBYW3HVU3OWSTQITS65M5RCNY&asset=USD%3AGBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H");
      }

      void testSingleClaimableBalance() {

          FakeServer* fakeServer= new FakeServer();
          fakeServer->addGet("/claimable_balances/00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd2",json);

          Server* server = new Server(fakeServer->baseUrl());
          QString id = "00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd2";
          ClaimableBalanceResponse* response = server->claimableBalances().claimableBalance(id);
          bool responseIsReady=false;
          QObject::connect(response,&ClaimableBalanceResponse::ready,[&responseIsReady](){
                responseIsReady=true;
          });
          WAIT_FOR(!responseIsReady)

          QCOMPARE(response->getID(), id);
          QCOMPARE(response->getAmount(), "1000.0000000");
          fakeServer->deleteLater();
      }

    /**
     * /claimable_balances with sponsor, claimant and asset, and
     * /claimable_balances/{id}, as
     * js-stellar-sdk 17.1.0 (the call builder's url after checkFilter(),
     * what call() requests) and py-stellar-sdk 14.1.0 (the builder's url and
     * params as requests encodes them) build these URLs, the same in both.
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        const QString id("00000000929b20b72e5890ab51c24f1cc46fa01c4f318d8d33367d24dd614cfdf5491072");
        QScopedPointer<Asset> usd(Asset::createNonNativeAsset("USD", QString("GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG")));
        QScopedPointer<Asset> xlm(new AssetTypeNative());
        QCOMPARE(server.claimableBalances().forSponsor("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB")
                     .forClaimant("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN")
                     .forAsset(usd.data()).limit(3).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/claimable_balances?sponsor=GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB&claimant=GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN&asset=USD%3AGDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG&limit=3"));
        QCOMPARE(server.claimableBalances().forAsset(xlm.data()).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/claimable_balances?asset=native"));
        // The single balance: the request is dropped before the event loop sends it.
        ClaimableBalancesRequestBuilder one = server.claimableBalances();
        delete one.claimableBalance(id);
        QCOMPARE(one.requestUri().toString(), QString("https://horizon-testnet.stellar.org/claimable_balances/00000000929b20b72e5890ab51c24f1cc46fa01c4f318d8d33367d24dd614cfdf5491072"));
    }
};

ADD_TEST(ClaimableBalancesRequestBuilderTest)
#endif // CLAIMABLEBALANCESREQUESTBUILDERTEST_H
