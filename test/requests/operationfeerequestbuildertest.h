#ifndef OPERATIONFEEREQUESTBUILDERTEST_H
#define OPERATIONFEEREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"

class OperationFeeRequestBuilderTest: public QObject
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
        QUrl uri = server->operationFeeStats()
                .buildUri();
        QString expected("https://horizon-testnet.stellar.org/fee_stats");
        QVERIFY(uri.toString() == expected);

    }

};

ADD_TEST(OperationFeeRequestBuilderTest)
#endif // OPERATIONFEEREQUESTBUILDERTEST_H
