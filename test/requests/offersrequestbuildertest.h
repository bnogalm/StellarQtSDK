#ifndef OFFERSREQUESTBUILDERTEST_H
#define OFFERSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"
#include "../../src/asset.h"
class OffersRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testForAccount() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.offers()
                .forAccount(KeyPair::fromAccountId(QString("GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H")))
                .limit(200)
                .order(RequestBuilder::Order::DESC)
                .buildUri();
        QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/accounts/GBRPYHIL2CI3FNQ4BXLFMNDLFJUNPU2HY3ZMFSHONUCEOASW7QC7OX2H/offers?limit=200&order=desc"));
    }

};

ADD_TEST(OffersRequestBuilderTest)
#endif // OFFERSREQUESTBUILDERTEST_H
