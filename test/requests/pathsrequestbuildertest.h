#ifndef PATHSREQUESTBUILDERTEST_H
#define PATHSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"

class PathsRequestBuilderTest: public QObject
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
      Server server("https://horizon-testnet.stellar.org");
      QUrl uri = server.paths()
              .destinationAccount(KeyPair::fromAccountId("GB24QI3BJNKBY4YNJZ2I37HFIYK56BL2OURFML76X46RQQKDLVT7WKJF"))
              .sourceAccount(KeyPair::fromAccountId("GD4KO3IOYYWIYVI236Y35K2DU6VNYRH3BPNFJSH57J5BLLCQHBIOK3IN"))
              .destinationAmount("20.50")
              .destinationAsset(Asset::createNonNativeAsset("USD", KeyPair::fromAccountId("GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH")))
              .cursor("13537736921089")
              .limit(200)
              .order(RequestBuilder::Order::ASC)
              .buildUri();

      QCOMPARE(uri.toString(),
              QString("https://horizon-testnet.stellar.org/paths?"
              "destination_account=GB24QI3BJNKBY4YNJZ2I37HFIYK56BL2OURFML76X46RQQKDLVT7WKJF&"
              "source_account=GD4KO3IOYYWIYVI236Y35K2DU6VNYRH3BPNFJSH57J5BLLCQHBIOK3IN&"
              "destination_amount=20.50&"
              "destination_asset_type=credit_alphanum4&"
              "destination_asset_code=USD&"
              "destination_asset_issuer=GAYSHLG75RPSMXWJ5KX7O7STE6RSZTD6NE4CTWAXFZYYVYIFRUVJIBJH&"
              "cursor=13537736921089&"
              "limit=200&"
              "order=asc"));
  }

};

ADD_TEST(PathsRequestBuilderTest)
#endif // PATHSREQUESTBUILDERTEST_H
