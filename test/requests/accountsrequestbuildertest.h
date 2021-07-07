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
      } catch (std::runtime_error e) {
        //assertEquals("cannot set both sponsor and asset", e.getMessage());
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
      } catch (std::runtime_error e) {
        //assertEquals("cannot set both signer and sponsor", e.getMessage());
      }
    }
};

ADD_TEST(AccountRequestBuilderTest)
#endif // ACCOUNTSREQUESTBUILDERTEST_H
