#ifndef ACCOUNTSREQUESTBUILDERTEST_H
#define ACCOUNTSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"
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
                .signer(signer)
                .order(RequestBuilder::Order::ASC)
                .buildUri();

        QString expected("https://horizon-testnet.stellar.org/accounts?limit=200&signer=GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB&order=asc");
        QVERIFY(uri.toString() == expected);
    }

    void testAccountsAsset() {
        Server* server = new Server("https://horizon-testnet.stellar.org");
        KeyPair* issuer = new KeyPair("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB");
        Asset* issuedAsset = Asset::createNonNativeAsset("USD",issuer);
        QUrl uri = server->accounts()
                .limit(200)
                .asset(issuedAsset)
                .order(RequestBuilder::Order::ASC)
                .buildUri();

        QString expected("https://horizon-testnet.stellar.org/accounts?limit=200&asset=USD%3AGD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB&order=asc");
        QVERIFY(uri.toString() == expected);
    }
};

ADD_TEST(AccountRequestBuilderTest)
#endif // ACCOUNTSREQUESTBUILDERTEST_H
