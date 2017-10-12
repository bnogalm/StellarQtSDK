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

};

ADD_TEST(LedgersRequestBuilderTest)
#endif // LEDGERSREQUESTBUILDERTEST_H
