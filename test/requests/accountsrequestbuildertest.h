#ifndef ACCOUNTSREQUESTBUILDERTEST_H
#define ACCOUNTSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"

#include "../../src/assettypecreditalphanum4.h"

class AccountRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testAccounts() {
        Server* server = new Server("https://horizon-testnet.stellar.org");

        QUrl uri = server->accounts()
                .cursor("13537736921089")
                .limit(200)
                .order(RequestBuilder::Order::ASC)
                .buildUri();

        QString expected("https://horizon-testnet.stellar.org/accounts?cursor=13537736921089&limit=200&order=asc");
        QVERIFY(uri.toString() == expected);

    }

    void testAccountsSigner() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        KeyPair* signer = new KeyPair("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB");
        QUrl uri = server->accounts()
                .limit(200)
                .forSigner(signer)
                .order(RequestBuilder::Order::ASC)
                .buildUri();

        QString expected("https://horizon-testnet.stellar.org/accounts?limit=200&signer=GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB&order=asc");
        QVERIFY(uri.toString() == expected);
    }

    void testForSponsor() {
      Server* server = new Server("https://horizon-testnet.stellar.org");
      KeyPair* sponsor = new KeyPair("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN");
      QUrl uri = server->accounts()
          .forSponsor(sponsor)
          .buildUri();
      QString expected("https://horizon-testnet.stellar.org/accounts?sponsor=GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN");      
      QVERIFY(uri.toString()==expected);
    }
    void testAccountsAsset() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        KeyPair* issuer = new KeyPair("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB");
        Asset* issuedAsset = Asset::createNonNativeAsset("USD",issuer);
        QUrl uri = server->accounts()
                .limit(200)
                .forAsset(issuedAsset)
                .order(RequestBuilder::Order::ASC)
                .buildUri();

        QString expected("https://horizon-testnet.stellar.org/accounts?limit=200&asset=USD%3AGD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB&order=asc");
        QVERIFY(uri.toString() == expected);
    }

    void testForAssetAndSponsorInvalid() {
      Server* server = new Server("https://horizon-testnet.stellar.org");
      KeyPair* sponsor = new KeyPair("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB");
      KeyPair* issuer = new KeyPair("GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG");
      AssetTypeCreditAlphaNum* asset = new AssetTypeCreditAlphaNum4("USD", issuer);
      try {
        server->accounts()
            .forSponsor(sponsor)
            .forAsset(asset)
            .buildUri();
        QFAIL("missing exception");
      } catch (const std::runtime_error& e) {
        //assertEquals("cannot set both sponsor and asset", e.getMessage());
        Q_UNUSED(e)
      }
    }

    void testForSignerAndSponsorInvalid() {
      Server* server = new Server("https://horizon-testnet.stellar.org");
      KeyPair* sponsor = new KeyPair("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB");
      KeyPair* signer = new KeyPair("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN");

      try {
        server->accounts()
            .forSigner(signer)
            .forSponsor(sponsor)
            .buildUri();
        QFAIL("missing exception");
      } catch (const std::runtime_error& e) {
        //assertEquals("cannot set both signer and sponsor", e.getMessage());
        Q_UNUSED(e)
      }
    }

    /**
     * signer, sponsor, asset and liquidity_pool exclude each other in
     * either order: forLiquidityPool refused the other three, but none of
     * them refused a liquidity_pool already set, so the request went out
     * and Horizon answered 400. The same filter set again replaces it.
     */
    void testOneFilterInEitherOrder() {
        Server server("https://horizon-testnet.stellar.org");
        const QString pool("dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7");
        QScopedPointer<KeyPair> account(KeyPair::fromAccountId("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN"));
        QScopedPointer<KeyPair> issuer(KeyPair::fromAccountId("GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG"));
        QScopedPointer<Asset> asset(new AssetTypeCreditAlphaNum4("USD", *issuer));

        QVERIFY_EXCEPTION_THROWN(server.accounts().forLiquidityPool(pool).forSigner(account.data()), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(server.accounts().forLiquidityPool(pool).forSponsor(account.data()), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(server.accounts().forLiquidityPool(pool).forAsset(asset.data()), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(server.accounts().forSigner(account.data()).forLiquidityPool(pool), std::runtime_error);
        QVERIFY_EXCEPTION_THROWN(server.accounts().forAsset(asset.data()).forSigner(account.data()), std::runtime_error);
        try {
            server.accounts().forLiquidityPool(pool).forSigner(account.data());
            QFAIL("missing exception");
        } catch (const std::runtime_error& e) {
            QVERIFY(QString::fromUtf8(e.what()).startsWith("cannot set both liquidity_pool and signer"));
        }
        QCOMPARE(server.accounts().forLiquidityPool("00").forLiquidityPool(pool).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts?liquidity_pool=") + pool);
    }

    /**
     * /accounts with each filter, and /accounts/{id}, as
     * js-stellar-sdk 17.1.0 (the call builder's url after checkFilter(),
     * what call() requests) and py-stellar-sdk 14.1.0 (the builder's url and
     * params as requests encodes them) build these URLs, the same in both.
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        const QString a("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB");
        const QString b("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN");
        const QString pool("dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7");
        QScopedPointer<Asset> banana(Asset::createNonNativeAsset("BANANA", a));
        // The QString overloads are private: an account id goes in as a KeyPair.
        QScopedPointer<KeyPair> ka(KeyPair::fromAccountId(a));
        QScopedPointer<KeyPair> kb(KeyPair::fromAccountId(b));
        QCOMPARE(server.accounts().cursor("now").limit(10).order(RequestBuilder::Order::DESC).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts?cursor=now&limit=10&order=desc"));
        QCOMPARE(server.accounts().forSigner(ka.data()).limit(20).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts?signer=GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB&limit=20"));
        QCOMPARE(server.accounts().forSponsor(kb.data()).order(RequestBuilder::Order::ASC).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts?sponsor=GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN&order=asc"));
        QCOMPARE(server.accounts().forAsset(banana.data()).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts?asset=BANANA%3AGD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB"));
        QCOMPARE(server.accounts().forLiquidityPool(pool).cursor("123").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts?liquidity_pool=dd7b1ab831c273310ddbec6f97870aa83c2fbd78ce22aded37ecbf4f3380fac7&cursor=123"));
        // The single account: the request is dropped before the event loop sends it.
        AccountsRequestBuilder one = server.accounts();
        delete one.account(ka.data());
        QCOMPARE(one.requestUri().toString(), QString("https://horizon-testnet.stellar.org/accounts/GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB"));
    }
};

ADD_TEST(AccountRequestBuilderTest)
#endif // ACCOUNTSREQUESTBUILDERTEST_H
