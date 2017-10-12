#ifndef EFFECTSREQUESTBUILDERTEST_H
#define EFFECTSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"

class EffectsRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }

    void testEffects() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.effects()
                .limit(200)
                .order(RequestBuilder::Order::DESC)
                .buildUri();
        QCOMPARE( uri.toString(), QString("https://horizon-testnet.stellar.org/effects?limit=200&order=desc"));
    }


    void testForAccount() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.effects()
                .forAccount(KeyPair::fromAccountId("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H"))
                .limit(200)
                .order(RequestBuilder::Order::DESC)
                .buildUri();
        QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/accounts/GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H/effects?limit=200&order=desc"));
    }


    void testForLedger() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.effects()
                .forLedger(200000000000L)
                .limit(50)
                .order(RequestBuilder::Order::ASC)
                .buildUri();
        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/ledgers/200000000000/effects?limit=50&order=asc"));
    }


    void testForTransaction() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.effects()
                .forTransaction("991534d902063b7715cd74207bef4e7bd7aa2f108f62d3eba837ce6023b2d4f3")
                .buildUri();
        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/transactions/991534d902063b7715cd74207bef4e7bd7aa2f108f62d3eba837ce6023b2d4f3/effects"));
    }


    void testForOperation() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.effects()
                .forOperation(28798257847L)
                .cursor("85794837")
                .buildUri();
        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/operations/28798257847/effects?cursor=85794837"));
    }

};

ADD_TEST(EffectsRequestBuilderTest)
#endif // EFFECTSREQUESTBUILDERTEST_H
