#ifndef ROOTREQUESTBUILDTEST_H
#define ROOTREQUESTBUILDTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"

class RootRequestBuildTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testRoot()
    {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.root()
        .buildUri();
        QCOMPARE(uri.toString(), QString("https://horizon-testnet.stellar.org/"));
    }
};

ADD_TEST(RootRequestBuildTest)
#endif // ROOTREQUESTBUILDTEST_H
