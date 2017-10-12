#ifndef FEDERATIONTEST_H
#define FEDERATIONTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/federation/federation.h"

class FederationTest: public QObject
{
    Q_OBJECT

    //TODO use offline test...
//    const QByteArray successResponse =
//   "{\"stellar_address\":\"bob*stellar.org\",\"account_id\":\"GCW667JUHCOP5Y7KY6KGDHNPHFM4CS3FCBQ7QWDUALXTX3PGXLSOEALY\"}";
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }


    void testResolveSuccess()  {
//        FederationResponse response(0);
//        response.loadFromJson(successResponse);

        FederationResponse* response = Federation::resolve("bank*funtracker.site");

        WAIT_FOR(response->getStellarAddress().isNull())

        QCOMPARE(response->getStellarAddress(), QString("bank*funtracker.site"));
        QCOMPARE(response->getAccountId(), QString("GBUYUAI75XXWDZEKLY66CFYKQPET5JR4EENXZBUZ3YXZ7DS56Z4OKOFU"));
        QVERIFY(response->getMemoType().isNull());
        QVERIFY(response->getMemo().isNull());
    }


    void testMalformedAddress() {
        try {
            FederationResponse* response = Federation::resolve("bob*stellar.org*test");
            Q_UNUSED(response)
            QFAIL("Expected exception");
        } catch (...) {

       }
    }

};
//we disable this test to dont spam funtracker.site
#ifndef STELLAR_SKIP_LIVE_TESTS
ADD_TEST(FederationTest)
#endif
#endif // FEDERATIONTEST_H



