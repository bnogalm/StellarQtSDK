#ifndef LEDGERSREQUESTBUILDERTEST_H
#define LEDGERSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"

class LedgersRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testLedgers() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.ledgers()
                .limit(200)
                .order(RequestBuilder::Order::ASC)
                .buildUri();
        QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/ledgers?limit=200&order=asc"));

    }

    /**
     * /ledgers and /ledgers/{sequence}, as
     * js-stellar-sdk 17.1.0 (the call builder's url after checkFilter(),
     * what call() requests) and py-stellar-sdk 14.1.0 (the builder's url and
     * params as requests encodes them) build these URLs, the same in both.
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        QCOMPARE(server.ledgers().limit(200).order(RequestBuilder::Order::DESC).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/ledgers?limit=200&order=desc"));
        QCOMPARE(server.ledgers().limit(10).limit(20).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/ledgers?limit=20"));
        // The single ledger: the request is dropped before the event loop sends it.
        LedgersRequestBuilder one = server.ledgers();
        delete one.ledger(898826);
        QCOMPARE(one.requestUri().toString(), QString("https://horizon-testnet.stellar.org/ledgers/898826"));
    }
};

ADD_TEST(LedgersRequestBuilderTest)
#endif // LEDGERSREQUESTBUILDERTEST_H
