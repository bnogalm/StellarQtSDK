#ifndef PATHSREQUESTBUILDERTEST_H
#define PATHSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"

class PathsRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testStrictReceiveWithSourceAccount() {
      Server server("https://horizon-testnet.stellar.org");
      QUrl uri = server.strictReceivePaths()
              .destinationAccount(KeyPair("GB24QI3BJNKBY4YNJZ2I37HFIYK56BL2OURFML76X46RQQKDLVT7WKJF"))
              .sourceAccount(KeyPair("GD4KO3IOYYWIYVI236Y35K2DU6VNYRH3BPNFJSH57J5BLLCQHBIOK3IN"))
              .destinationAmount("20.50")
              .destinationAsset(Asset::createNonNativeAsset("USD", KeyPair("GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH")))
              .cursor("13537736921089")
              .limit(200)
              .order(RequestBuilder::Order::ASC)
              .buildUri();


      QCOMPARE(uri.toString(),
              QString("https://horizon-testnet.stellar.org/paths/strict-receive?"
              "destination_account=GB24QI3BJNKBY4YNJZ2I37HFIYK56BL2OURFML76X46RQQKDLVT7WKJF&"
              "source_account=GD4KO3IOYYWIYVI236Y35K2DU6VNYRH3BPNFJSH57J5BLLCQHBIOK3IN&"
              "destination_amount=20.50&"
              "destination_asset_type=credit_alphanum4&"
              "destination_asset_code=USD&"
              "destination_asset_issuer=GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH&"
              "cursor=13537736921089&"
              "limit=200&"
              "order=asc"));
  }


    void testStrictReceiveWithSourceAssets() {
        QList<Asset*> assets = QList<Asset*>({
                                                 Asset::create("native", "", ""),
                                                 Asset::create("credit_alphanum4", "USD", "GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH"),
                                                 Asset::create("credit_alphanum4", "EUR", "GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH")
                                             }
                                             );

        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.strictReceivePaths()
                .destinationAccount(KeyPair("GB24QI3BJNKBY4YNJZ2I37HFIYK56BL2OURFML76X46RQQKDLVT7WKJF"))
                .sourceAssets(assets)
                .destinationAmount("20.50")
                .destinationAsset(Asset::createNonNativeAsset("USD", KeyPair("GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH")))
                .cursor("13537736921089")
                .limit(3)
                .order(RequestBuilder::Order::ASC)
                .buildUri();

        QCOMPARE( uri.toString(),"https://horizon-testnet.stellar.org/paths/strict-receive?"
                                 "destination_account=GB24QI3BJNKBY4YNJZ2I37HFIYK56BL2OURFML76X46RQQKDLVT7WKJF&"
                                 "source_assets=native%2CUSD%3AGAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH%2CEUR%3AGAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH&"
                                 "destination_amount=20.50&"
                                 "destination_asset_type=credit_alphanum4&"
                                 "destination_asset_code=USD&"
                                 "destination_asset_issuer=GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH&"
                                 "cursor=13537736921089&"
                                 "limit=3&"
                                 "order=asc");
    }

    void testStrictReceiveWithSourceAccountAndSourceAssets() {
        QList<Asset*> assets = QList<Asset*>({
                                                 Asset::create("native", "", ""),
                                                 Asset::create("credit_alphanum4", "USD", "GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH"),
                                                 Asset::create("credit_alphanum4", "EUR", "GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH")
                                             }
                    );
        Server server("https://horizon-testnet.stellar.org");

        try {
            server.strictReceivePaths().sourceAssets(assets).sourceAccount(KeyPair("GD4KO3IOYYWIYVI236Y35K2DU6VNYRH3BPNFJSH57J5BLLCQHBIOK3IN"));
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QString errorMsg(e.what());
            QVERIFY(errorMsg.contains("cannot set both source_assets and source_account"));
        }

        try {
            server.strictReceivePaths().sourceAccount(KeyPair("GD4KO3IOYYWIYVI236Y35K2DU6VNYRH3BPNFJSH57J5BLLCQHBIOK3IN")).sourceAssets(assets);
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QString errorMsg(e.what());
            QVERIFY(errorMsg.contains("cannot set both source_assets and source_account"));
        }
    }


    void testStrictSendWithDestinationAccount() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.strictSendPaths()
                .destinationAccount(KeyPair("GB24QI3BJNKBY4YNJZ2I37HFIYK56BL2OURFML76X46RQQKDLVT7WKJF"))
                .sourceAmount("20.50")
                .sourceAsset(Asset::createNonNativeAsset("USD", KeyPair("GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH")))
                .cursor("13537736921089")
                .limit(200)
                .order(RequestBuilder::Order::ASC)
                .buildUri();

        QCOMPARE(uri.toString(), "https://horizon-testnet.stellar.org/paths/strict-send?"
                     "destination_account=GB24QI3BJNKBY4YNJZ2I37HFIYK56BL2OURFML76X46RQQKDLVT7WKJF&"
                     "source_amount=20.50&"
                     "source_asset_type=credit_alphanum4&"
                     "source_asset_code=USD&"
                     "source_asset_issuer=GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH&"
                     "cursor=13537736921089&"
                     "limit=200&"
                     "order=asc");
    }


    void testStrictSendWithDestinationAssets() {
        QList<Asset*> assets = QList<Asset*>({
                    Asset::create("native", "", ""),
                    Asset::create("credit_alphanum4", "USD", "GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH"),
                    Asset::create("credit_alphanum4", "EUR", "GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH")
                    });
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.strictSendPaths()
                .destinationAssets(assets)
                .sourceAmount("20.50")
                .sourceAsset(Asset::createNonNativeAsset("USD", KeyPair("GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH")))
                .cursor("13537736921089")
                .limit(3)
                .order(RequestBuilder::Order::ASC)
                .buildUri();

        QCOMPARE(uri.toString(), "https://horizon-testnet.stellar.org/paths/strict-send?"
                     "destination_assets=native%2CUSD%3AGAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH%2CEUR%3AGAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH&"
                     "source_amount=20.50&"
                     "source_asset_type=credit_alphanum4&"
                     "source_asset_code=USD&"
                     "source_asset_issuer=GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH&"
                     "cursor=13537736921089&"
                     "limit=3&"
                     "order=asc");
    }


    void testStrictSendWithDestinationAccountAndDestinationAssets() {
        QList<Asset*> assets = QList<Asset*>({
                    Asset::create("native", "", ""),
                    Asset::create("credit_alphanum4", "USD", "GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH"),
                    Asset::create("credit_alphanum4", "EUR", "GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH")
                    });
        Server server("https://horizon-testnet.stellar.org");

        try {
            server.strictSendPaths().destinationAssets(assets).destinationAccount(KeyPair("GD4KO3IOYYWIYVI236Y35K2DU6VNYRH3BPNFJSH57J5BLLCQHBIOK3IN"));
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QString errorMsg(e.what());
            QVERIFY(errorMsg.contains("cannot set both destination_assets and destination_account"));
        }

        try {
            server.strictSendPaths().destinationAccount(KeyPair("GD4KO3IOYYWIYVI236Y35K2DU6VNYRH3BPNFJSH57J5BLLCQHBIOK3IN")).destinationAssets(assets);
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QString errorMsg(e.what());
            QVERIFY(errorMsg.contains("cannot set both destination_assets and destination_account"));
        }
    }

    /**
     * /paths/strict-receive from an account and from source assets, and
     * /paths/strict-send to an account and to destination assets, as
     * js-stellar-sdk 17.1.0 builds them (the call builder's url after
     * checkFilter(), what call() requests). py-stellar-sdk 14.1.0 sends the
     * same parameters with the same values, amounts first.
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        QScopedPointer<Asset> xlm(new AssetTypeNative());
        QScopedPointer<Asset> usd(Asset::createNonNativeAsset("USD", QString("GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG")));
        QScopedPointer<Asset> banana(Asset::createNonNativeAsset("BANANA", QString("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB")));
        QCOMPARE(server.strictReceivePaths()
                     .sourceAccount(KeyPair("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB"))
                     .destinationAmount("20.5").destinationAsset(usd.data()).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/paths/strict-receive?source_account=GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB&destination_amount=20.5&destination_asset_type=credit_alphanum4&destination_asset_code=USD&destination_asset_issuer=GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG"));
        QCOMPARE(server.strictReceivePaths()
                     .sourceAssets(QList<Asset*>() << xlm.data() << banana.data())
                     .destinationAmount("20.5").destinationAsset(usd.data()).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/paths/strict-receive?source_assets=native%2CBANANA%3AGD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB&destination_amount=20.5&destination_asset_type=credit_alphanum4&destination_asset_code=USD&destination_asset_issuer=GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG"));
        QCOMPARE(server.strictSendPaths().sourceAsset(usd.data()).sourceAmount("10")
                     .destinationAccount(KeyPair("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN")).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/paths/strict-send?source_asset_type=credit_alphanum4&source_asset_code=USD&source_asset_issuer=GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG&source_amount=10&destination_account=GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN"));
        QCOMPARE(server.strictSendPaths().sourceAsset(xlm.data()).sourceAmount("10")
                     .destinationAssets(QList<Asset*>() << usd.data() << banana.data()).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/paths/strict-send?source_asset_type=native&source_amount=10&destination_assets=USD%3AGDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG%2CBANANA%3AGD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB"));
    }
};

ADD_TEST(PathsRequestBuilderTest)
#endif // PATHSREQUESTBUILDERTEST_H
