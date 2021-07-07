#ifndef CLAIMABLEBALANCEPAGEDESERIALIZERTEST_H
#define CLAIMABLEBALANCEPAGEDESERIALIZERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"

#include "../../src/responses/page.h"
#include "../../src/predicate.h"
#include "../../src/responses/claimablebalanceresponse.h"
class ClaimableBalancePageDeserializerTest : public QObject
{
    Q_OBJECT
    QByteArray json = "{\n"
        "  \"_links\": {\n"
        "    \"self\": {\n"
        "      \"href\": \"https://horizon-protocol14.stellar.org/claimable_balances?cursor=&limit=10&order=asc\"\n"
        "    },\n"
        "    \"next\": {\n"
        "      \"href\": \"https://horizon-protocol14.stellar.org/claimable_balances?cursor=117663-00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd2&limit=10&order=asc\"\n"
        "    },\n"
        "    \"prev\": {\n"
        "      \"href\": \"https://horizon-protocol14.stellar.org/claimable_balances?cursor=38888-00000000929b20b72e5890ab51c24f1cc46fa01c4f318d8d33367d24dd614cfdf5491072&limit=10&order=desc\"\n"
        "    }\n"
        "  },\n"
        "  \"_embedded\": {\n"
        "    \"records\": [\n"
        "      {\n"
        "        \"_links\": {\n"
        "          \"self\": {\n"
        "            \"href\": \"https://horizon-protocol14.stellar.org/claimable_balances/00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd2\"\n"
        "          }\n"
        "        },\n"
        "        \"id\": \"00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd2\",\n"
        "        \"asset\": \"COP:GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\",\n"
        "        \"amount\": \"1000.0000000\",\n"
        "        \"sponsor\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\",\n"
        "        \"last_modified_ledger\": 117663,\n"
        "        \"claimants\": [\n"
        "          {\n"
        "            \"destination\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\",\n"
        "            \"predicate\": {\n"
        "              \"or\": [\n"
        "                {\n"
        "                  \"abs_before\": \"2020-09-28T17:57:04Z\"\n"
        "                },\n"
        "                {\n"
        "                   \"rel_before\": \"12\"\n"
        "                }\n"
        "              ]\n"
        "            }\n"
        "          }\n"
        "        ],\n"
        "        \"paging_token\": \"117663-00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd2\"\n"
        "      },\n"
        "            {\n"
        "        \"_links\": {\n"
        "          \"self\": {\n"
        "            \"href\": \"https://horizon-protocol14.stellar.org/claimable_balances/00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd3\"\n"
        "          }\n"
        "        },\n"
        "        \"id\": \"00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd3\",\n"
        "        \"asset\": \"native\",\n"
        "        \"amount\": \"2000.0000000\",\n"
        "        \"last_modified_ledger\": 117663,\n"
        "        \"claimants\": [\n"
        "          {\n"
        "            \"destination\": \"GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO\",\n"
        "            \"predicate\": {\n"
        "              \"and\": [\n"
        "                {\n"
        "                  \"abs_before\": \"2020-09-28T17:57:04Z\"\n"
        "                },\n"
        "                {\n"
        "                  \"not\": {\n"
        "                    \"unconditional\": true\n"
        "                  }\n"
        "                }\n"
        "              ]\n"
        "            }\n"
        "          }\n"
        "        ],\n"
        "        \"paging_token\": \"117663-00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd3\"\n"
        "      }\n"
        "    ]\n"
        "  }\n"
        "}";
public:


private slots:

  void roundTripPredicate(const Predicate& p) {
    stellar::ClaimPredicate xdrPredicate = p.toXdr();
    Predicate* pr = Predicate::fromXdr(xdrPredicate);
    QVERIFY(p.equals(pr));
  }


  void testDeserialize() {
    Page<ClaimableBalanceResponse> claimableBalancePage;
    claimableBalancePage.loadFromJson(json);
    QCOMPARE(claimableBalancePage.getRecords().size(), 2);

    QCOMPARE(claimableBalancePage.get(0).getID(), "00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd2");
    QVERIFY(claimableBalancePage.get(0).getAsset()->equals( Asset::create("COP:GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO")));
    QCOMPARE(claimableBalancePage.get(0).getAssetString(), "COP:GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO");
    QCOMPARE(claimableBalancePage.get(0).getAmount(), "1000.0000000");
    QCOMPARE(claimableBalancePage.get(0).getSponsor(), "GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO");
    QCOMPARE(claimableBalancePage.get(0).getLastModifiedLedger(), 117663);
    QCOMPARE(claimableBalancePage.get(0).getPagingToken(), "117663-00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd2");
    QCOMPARE(claimableBalancePage.get(0).getClaimants().at(0).getDestination(), "GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO");
    Predicate::Or& por = (Predicate::Or&)claimableBalancePage.get(0).getClaimants().at(0).getPredicate();
    QCOMPARE(por.getInner().size(), 2);
    Predicate::AbsBefore& absBefore = (Predicate::AbsBefore&)por.get(0);
    Predicate::RelBefore& relBefore = (Predicate::RelBefore&)por.get(1);
    QCOMPARE(absBefore.getDate().toString(Qt::ISODate), "2020-09-28T17:57:04Z");
    QCOMPARE(relBefore.getSecondsSinceClose(), 12);

    QCOMPARE(claimableBalancePage.get(1).getID(), "00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd3");
    QVERIFY(claimableBalancePage.get(1).getAsset()->equals(Asset::create("native")));
    QCOMPARE(claimableBalancePage.get(1).getAssetString(), "native");
    QCOMPARE(claimableBalancePage.get(1).getAmount(), "2000.0000000");
    QVERIFY(claimableBalancePage.get(1).getSponsor().isEmpty());
    QCOMPARE(claimableBalancePage.get(1).getLastModifiedLedger(), 117663);
    QCOMPARE(claimableBalancePage.get(1).getPagingToken(), "117663-00000000ae76f49e8513d0922b6bcbc8a3f5c4c0a5161871f27924e08724646acab56cd3");
    QCOMPARE(claimableBalancePage.get(1).getClaimants().at(0).getDestination(), "GB56OJGSA6VHEUFZDX6AL2YDVG2TS5JDZYQJHDYHBDH7PCD5NIQKLSDO");

    Predicate::And& pand = (Predicate::And&)claimableBalancePage.get(1).getClaimants().at(0).getPredicate();
    QCOMPARE(pand.getInner().size(), 2);
    absBefore = (Predicate::AbsBefore&)pand.get(0);
    QCOMPARE(absBefore.getDate().toString(Qt::ISODate), "2020-09-28T17:57:04Z");
    Predicate::Not pnot = (Predicate::Not&)pand.get(1);
    QVERIFY(Predicate::Unconditional().equals(&pnot.getInner()));

    for (Response* record : claimableBalancePage.getRecords()) {
      for (Claimant claimant : ((ClaimableBalanceResponse*)record)->getClaimants()) {
        roundTripPredicate(claimant.getPredicate());
      }
    }
  }


};
ADD_TEST(ClaimableBalancePageDeserializerTest)
#endif // CLAIMABLEBALANCEPAGEDESERIALIZERTEST_H
