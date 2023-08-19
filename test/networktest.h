#ifndef NETWORKTEST_H
#define NETWORKTEST_H

#include <QObject>

#include <QtTest>
#include "testcollector.h"
#include "../src/network.h"

class NetworkTest: public QObject
{
    Q_OBJECT


private slots:

    void initTestCase()
    {
    }
    void cleanupTestCase()
    {
    }

    void testNoDefaultNetwork() {
        Network::use(0);
        try{
            QVERIFY(Network::current()==0);
        }
        catch(const std::runtime_error& e)
        {
            QCOMPARE(e.what(),"Not selected network");
        }
    }

    void testSwitchToTestNetwork() {
        Network::useTestNetwork();
        QVERIFY(QString("Test SDF Network ; September 2015")== Network::current()->getNetworkPassphrase());
    }

    void testSwitchToPublicNetwork() {
        Network::usePublicNetwork();
        QVERIFY(QString("Public Global Stellar Network ; September 2015")== Network::current()->getNetworkPassphrase());
    }

};

ADD_TEST(NetworkTest)
#endif // NETWORKTEST_H
