#ifndef PAYMENTSREQUESTBUILDERTEST_H
#define PAYMENTSREQUESTBUILDERTEST_H

#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"

class PaymentsRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testPayments() {
       Server *server = new Server("https://horizon-testnet.stellar.org");
       QUrl uri = server->payments()
               .limit(200)
               .order(RequestBuilder::Order::DESC)
               .buildUri();
       QCOMPARE(QString("https://horizon-testnet.stellar.org/payments?limit=200&order=desc"), uri.toString());
     }

     void testForAccount() {
       Server *server = new Server("https://horizon-testnet.stellar.org");
       QUrl uri = server->payments()
               .forAccount(KeyPair::fromAccountId("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"))
               .limit(200)
               .order(RequestBuilder::Order::DESC)
               .buildUri();
       QCOMPARE(QString("https://horizon-testnet.stellar.org/accounts/GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H/payments?limit=200&order=desc"), uri.toString());
     }

     void testForLedger() {
       Server *server = new Server("https://horizon-testnet.stellar.org");
       QUrl uri = server->payments()
               .forLedger(200000000000L)
               .limit(50)
               .order(RequestBuilder::Order::ASC)
               .buildUri();
       QCOMPARE(QString("https://horizon-testnet.stellar.org/ledgers/200000000000/payments?limit=50&order=asc"), uri.toString());
     }

     void testForTransaction() {
       Server *server = new Server("https://horizon-testnet.stellar.org");
       QUrl uri = server->payments()
               .forTransaction("991534d902063b7715cd74207bef4e7bd7aa2f108f62d3eba837ce6023b2d4f3")
               .buildUri();
       QCOMPARE(QString("https://horizon-testnet.stellar.org/transactions/991534d902063b7715cd74207bef4e7bd7aa2f108f62d3eba837ce6023b2d4f3/payments"), uri.toString());
   }

    /**
     * A parameter set twice keeps the last value, where it was first set,
     * as js's payments().limit(10).limit(20).cursor('a').cursor('b') and
     * py's params do. The request carried both values.
     */
    void testParameterSetTwiceIsReplaced() {
        Server server("https://horizon-testnet.stellar.org");
        QCOMPARE(server.payments().limit(10).limit(20).cursor("a").cursor("b").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/payments?limit=20&cursor=b"));
        QCOMPARE(server.payments().cursor("a").limit(10).order(RequestBuilder::Order::ASC).cursor("b").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/payments?cursor=b&limit=10&order=asc"));
        QCOMPARE(server.operations().includeFailed(true).includeFailed(false).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/operations?include_failed=false"));
        QScopedPointer<KeyPair> first(KeyPair::fromAccountId("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"));
        QScopedPointer<KeyPair> second(KeyPair::fromAccountId("GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN"));
        QCOMPARE(server.accounts().forSigner(first.data()).forSigner(second.data()).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts?signer=GDSBCQO34HWPGUGQSP3QBFEXVTSR2PW46UIGTHVWGWJGQKH3AFNHXHXN"));
    }

    /** include_failed on payments, as js's includeFailed(true) and py's include_failed(True) send it. */
    void testIncludeFailed() {
        Server server("https://horizon-testnet.stellar.org");
        QCOMPARE(server.payments().includeFailed(true).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/payments?include_failed=true"));
        QCOMPARE(server.payments()
                     .forAccount(KeyPair::fromAccountId("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"))
                     .includeFailed(false).limit(10).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts/GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H/payments?include_failed=false&limit=10"));
    }

    /**
     * /payments and the payments of an account, ledger and transaction, as
     * js-stellar-sdk 17.1.0 (the call builder's url after checkFilter(),
     * what call() requests) and py-stellar-sdk 14.1.0 (the builder's url and
     * params as requests encodes them) build these URLs, the same in both.
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        QScopedPointer<KeyPair> account(KeyPair::fromAccountId("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB"));
        QCOMPARE(server.payments().cursor("now").includeFailed(true).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/payments?cursor=now&include_failed=true"));
        QCOMPARE(server.payments().forAccount(account.data()).order(RequestBuilder::Order::DESC).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/accounts/GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB/payments?order=desc"));
        QCOMPARE(server.payments().forLedger(123456).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/ledgers/123456/payments"));
        QCOMPARE(server.payments().forTransaction("5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b").buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/transactions/5c2e4dad596941ef944d72741c8f8f1a4282f8f2f141e81d827f44bf365d626b/payments"));
    }
};

ADD_TEST(PaymentsRequestBuilderTest)
#endif // PAYMENTSREQUESTBUILDERTEST_H
