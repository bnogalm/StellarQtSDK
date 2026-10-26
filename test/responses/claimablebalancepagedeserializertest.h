#ifndef CLAIMABLEBALANCEPAGEDESERIALIZERTEST_H
#define CLAIMABLEBALANCEPAGEDESERIALIZERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"

#include "../../src/responses/page.h"
#include "../../src/predicate.h"
#include "../../src/responses/claimablebalanceresponse.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>
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



    // ---- Exact reading (1.0.4) -------------------------------------------
private:
    /** `json` with the member at `path` ("a.b[1].c") set to `value`, or removed when undefined. */
    static QJsonValue edited(const QJsonValue& json, QStringList path, const QJsonValue& value)
    {
        const QString step = path.takeFirst();
        const int bracket = step.indexOf(QLatin1Char('['));
        const QString key = bracket < 0 ? step : step.left(bracket);
        QJsonObject object = json.toObject();
        if (bracket >= 0) {
            const int index = step.mid(bracket + 1, step.size() - bracket - 2).toInt();
            QJsonArray array = object.value(key).toArray();
            array[index] = path.isEmpty() ? value : edited(array.at(index), path, value);
            object[key] = array;
        } else if (path.isEmpty()) {
            if (value.isUndefined())
                object.remove(key);
            else
                object[key] = value;
        } else {
            object[key] = edited(object.value(key), path, value);
        }
        return object;
    }

    /** What a T loading `json` with `member` set to `value` throws; empty if it loads. */
    template<class T>
    static QString refusalOf(const QByteArray& json, const QString& member, const QJsonValue& value)
    {
        const QJsonValue changed = edited(QJsonDocument::fromJson(json).object(),
                                          member.split(QLatin1Char('.')), value);
        T response;
        try {
            response.loadFromJson(QJsonDocument(changed.toObject()).toJson());
        } catch (const std::exception& e) {
            return QString::fromUtf8(e.what());
        }
        return QString();
    }

private slots:

    /**
     * A claimable balance's id, amount and claimants, exactly. A predicate
     * Qt could not read (a time that is not one, a rel_before that is not a
     * number, an "and" of three) used to become another predicate, read as
     * time 0 or 0 seconds, and an amount given as a number was left empty.
     */
    void testClaimableBalanceIsReadExactly()
    {
        typedef Page<ClaimableBalanceResponse> Balances;
        const QString r = QStringLiteral("_embedded.records[0].");
        const QString p = r + QStringLiteral("claimants[0].predicate");
        QVERIFY(refusalOf<Balances>(json, r + "amount", 1000).startsWith(r + "amount: "));
        QVERIFY(refusalOf<Balances>(json, r + "amount", QString("1000,0")).startsWith(r + "amount: "));
        QVERIFY(refusalOf<Balances>(json, r + "id", QJsonValue(QJsonValue::Undefined)).startsWith(r + "id: "));
        QVERIFY(refusalOf<Balances>(json, r + "paging_token", QJsonValue()).startsWith(r + "paging_token: "));
        QVERIFY(refusalOf<Balances>(json, r + "claimants", QJsonValue(QJsonValue::Undefined)).startsWith(r + "claimants: "));
        QVERIFY(refusalOf<Balances>(json, r + "claimants[0].destination", QString("GNOTANACCOUNT")).startsWith(r + "claimants[0].destination: "));
        QVERIFY(refusalOf<Balances>(json, p, QJsonObject{{"abs_before", "yesterday"}}).startsWith(p + ".abs_before: "));
        QVERIFY(refusalOf<Balances>(json, p, QJsonObject{{"rel_before", "12s"}}).startsWith(p + ".rel_before: "));
        QVERIFY(refusalOf<Balances>(json, p, QJsonObject{{"unconditional", false}}).startsWith(p + ".unconditional: "));
        QVERIFY(refusalOf<Balances>(json, p, QJsonObject{{"sometimes", true}}).startsWith(p + ": "));
        QVERIFY(refusalOf<Balances>(json, p, QJsonObject{{"and", QJsonArray{QJsonObject{{"unconditional", true}},
                                                                             QJsonObject{{"unconditional", true}},
                                                                             QJsonObject{{"unconditional", true}}}}})
                    .startsWith(p + ".and: "));
        QVERIFY(refusalOf<Balances>(json, p, QJsonObject{{"abs_before", "2020-09-28T17:57:04Z"},
                                                         {"abs_before_epoch", "1601315825"}})
                    .startsWith(p + ".abs_before: ")); // one second apart
        QVERIFY(refusalOf<Balances>(json, r + "last_modified_time", QString("last week")).startsWith(r + "last_modified_time: "));
    }

    /**
     * last_modified_time is ISO 8601 text in Horizon's answer, which this
     * integer property never took: it read 0 (or, unset, whatever the memory
     * held). It is the time in seconds now, and 0 for Horizon's null.
     */
    void testLastModifiedTimeIsRead()
    {
        Page<ClaimableBalanceResponse> balances;
        balances.loadFromJson(realPubnetClaimableBalances());
        QCOMPARE(balances.get(0).getLastModifiedTime(),
                 QDateTime::fromString("2026-09-28T15:16:06Z", Qt::ISODate).toSecsSinceEpoch());
        QJsonObject nulled = QJsonDocument::fromJson(realPubnetClaimableBalances()).object();
        nulled = edited(nulled, QString("_embedded.records[0].last_modified_time").split('.'), QJsonValue()).toObject();
        Page<ClaimableBalanceResponse> nulledPage;
        nulledPage.loadFromJson(QJsonDocument(nulled).toJson());
        QCOMPARE(nulledPage.get(0).getLastModifiedTime(), Q_INT64_C(0));
        ClaimableBalanceResponse fresh;
        QCOMPARE(fresh.getLastModifiedTime(), Q_INT64_C(0));
        QCOMPARE(fresh.getLastModifiedLedger(), Q_INT64_C(0));
    }

    /**
     * Real claimable balances, testnet and pubnet, load with their exact
     * predicates: an "and" of a "not" and an abs_before, each with Horizon's
     * abs_before_epoch.
     */
    void testRealHorizonClaimableBalances()
    {
        Page<ClaimableBalanceResponse> testnet;
        testnet.loadFromJson(realTestnetClaimableBalances());
        QCOMPARE(testnet.get(0).getAmount(), QString("10.0000000"));
        QCOMPARE(testnet.get(0).getFlags().isClawbackEnabled(), true);
        QCOMPARE(testnet.get(0).getClaimants().size(), 2);
        QVERIFY(dynamic_cast<const Predicate::Unconditional*>(&testnet.get(0).getClaimants().at(0).getPredicate()));
        const Predicate::And& both = dynamic_cast<const Predicate::And&>(testnet.get(0).getClaimants().at(1).getPredicate());
        Predicate::And copy(both);
        const Predicate::Not& notBefore = dynamic_cast<const Predicate::Not&>(copy.get(0));
        Predicate::Not notCopy(notBefore);
        QCOMPARE(dynamic_cast<Predicate::AbsBefore&>(notCopy.getInner()).getTimestampSeconds(), Q_INT64_C(1790609255));
        QCOMPARE(dynamic_cast<Predicate::AbsBefore&>(copy.get(1)).getTimestampSeconds(), Q_INT64_C(1790695667));

        Page<ClaimableBalanceResponse> pubnet;
        pubnet.loadFromJson(realPubnetClaimableBalances());
        QCOMPARE(pubnet.size(), 2);
        QCOMPARE(pubnet.get(0).getAmount(), QString("535454111111.0000000"));
        QCOMPARE(pubnet.get(0).getFlags().isClawbackEnabled(), false);
        QCOMPARE(pubnet.get(0).getAssetString(), QString("PALLADIUM:GAJALNL2LHOKXQYQ7GGGS22WE7TFNYUDCKDSRIY32AYMCW7QIUZGEFU4"));
    }

    /**
     * Every member Horizon always sends in a claimable balance is required:
     * missing or null, the ledger it was last changed in read 0 and its
     * links empty. The time may be null (Horizon leaves it so when it has
     * none), and the sponsor out.
     */
    void testMembersHorizonAlwaysSendsAreRequired()
    {
        typedef Page<ClaimableBalanceResponse> Balances;
        const QByteArray real = realPubnetClaimableBalances();
        const QString r = QStringLiteral("_embedded.records[0].");
        QVERIFY(refusalOf<Balances>(real, r + "amount", QString("535454111111.0000000")).isEmpty());
        for (const char* key : { "id", "asset", "amount", "claimants", "paging_token", "last_modified_ledger",
                                 "_links", "_links.self", "_links.self.href", "claimants[0].destination",
                                 "claimants[0].predicate" }) {
            QVERIFY2(refusalOf<Balances>(real, r + key, QJsonValue(QJsonValue::Undefined)).startsWith(r + key + ": "), key);
            QVERIFY2(refusalOf<Balances>(real, r + key, QJsonValue()).startsWith(r + key + ": "), key);
        }
        QVERIFY(refusalOf<Balances>(real, r + "last_modified_time", QJsonValue()).isEmpty());
        QVERIFY(refusalOf<Balances>(real, r + "sponsor", QJsonValue(QJsonValue::Undefined)).isEmpty());
    }
private:
    /** Real testnet claimable balances with or / not / abs_before predicates, one past the year 9999. */
    static QByteArray realTestnetClaimableBalances()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"self\":{\"href\":\"https://horizon-testnet.stellar.org/claimable_balances/0000000070729688ba0b0ece696eff1b254f88ab8c4baa0ff9218532122fd7c3329ed61e\"},\"transactions\":{\"href\":\"https://horizon-testnet.stellar.org/claimable_balances/0000000070729688ba0b0ece696eff1b254f88ab8c4baa0ff9218532122fd7c3329ed61e/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon-testnet.stellar.org/claimable_balances/0000000070729688ba0b0ece696eff1b254f88ab8c4baa0ff9218532122fd7c3329ed61e/operations{?cursor,limit,order}\",\"templated\":true}},\"id\":\"0000000070729688ba0b0ece696eff1b254f88ab8c4baa0ff9218532122fd7c3329ed61e\",\"asset\":\"TEST:GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"amount\":\"10.0000000\",\"sponsor\":\"GCTG3HQQJVPLUB4S2U6ND7X6A7ZDCUB6PILDNO66AOWTX54CSBPDQCXD\",\"last_modified_ledger\":4917136,\"last_modified_time\":\"2026-09-28T15:27:47Z\",\"claimants\":[{\"destination\":\"GBTTX7QVGGB6VTNNAKMXIFR6YL3RYA6UU3YR7FVBLKNOB2SWSA7XMQ7H\",\"predicate\":{\"unconditional\":true}},{\"destination\":\"GCALKXRTML6DHW2CT332CVAOYEWZQ2OBXGHW3YNY4ZLOE2QE3OFH7LLA\",\"predicate\":{\"and\":[{\"not\":{\"abs_before\":\"2026-09-28T15:27:35Z\",\"abs_before_epoch\":\"1790609255\"}},{\"abs_before\":\"2026-09-29T15:27:47Z\",\"abs_before_epoch\":\"1790695667\"}]}}],\"flags\":{\"clawback_enabled\":true},\"paging_token\":\"4917136-0000000070729688ba0b0ece696eff1b254f88ab8c4baa0ff9218532122fd7c3329ed61e\"}]},\"_links\":{\"self\":{\"href\":\"https://horizon.example/x?cursor=&limit=2&order=asc\"},\"next\":{\"href\":\"https://horizon.example/x?cursor=2&limit=2&order=asc\"},\"prev\":{\"href\":\"https://horizon.example/x?cursor=1&limit=2&order=desc\"}}}");
    }

    /** Two real pubnet claimable balances. */
    static QByteArray realPubnetClaimableBalances()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/claimable_balances/000000006b3d53f282d357d017ac5ff48278766401f9e5668c5a5b7c9b44d06882b1b8ca\"},\"transactions\":{\"href\":\"https://horizon.stellar.org/claimable_balances/000000006b3d53f282d357d017ac5ff48278766401f9e5668c5a5b7c9b44d06882b1b8ca/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon.stellar.org/claimable_balances/000000006b3d53f282d357d017ac5ff48278766401f9e5668c5a5b7c9b44d06882b1b8ca/operations{?cursor,limit,order}\",\"templated\":true}},\"id\":\"000000006b3d53f282d357d017ac5ff48278766401f9e5668c5a5b7c9b44d06882b1b8ca\",\"asset\":\"PALLADIUM:GAJALNL2LHOKXQYQ7GGGS22WE7TFNYUDCKDSRIY32AYMCW7QIUZGEFU4\",\"amount\":\"535454111111.0000000\",\"sponsor\":\"GAMIZAC6BZRUXGXUFWRVT4IAZQMB2E52MP6FOHTTTQ24USWZPVUF5WS6\",\"last_modified_ledger\":64664557,\"last_modified_time\":\"2026-09-28T15:16:06Z\",\"claimants\":[{\"destination\":\"GAMIZAC6BZRUXGXUFWRVT4IAZQMB2E52MP6FOHTTTQ24USWZPVUF5WS6\",\"predicate\":{\"not\":{\"abs_before\":\"2026-09-29T15:15:52Z\",\"abs_before_epoch\":\"1790694952\"}}},{\"destination\":\"GDXHZZDVKFJRQMAHMHV37GCCGRGQBPBX5BVM7OW2DSBJJMRNBTR5W2CQ\",\"predicate\":{\"unconditional\":true}}],\"flags\":{\"clawback_enabled\":false},\"paging_token\":\"64664557-000000006b3d53f282d357d017ac5ff48278766401f9e5668c5a5b7c9b44d06882b1b8ca\"},{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/claimable_balances/0000000036b8da456b9b665ddb5df0ae949e051c91bfbcbe04f680cf4bc636a087c852ce\"},\"transactions\":{\"href\":\"https://horizon.stellar.org/claimable_balances/0000000036b8da456b9b665ddb5df0ae949e051c91bfbcbe04f680cf4bc636a087c852ce/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon.stellar.org/claimable_balances/0000000036b8da456b9b665ddb5df0ae949e051c91bfbcbe04f680cf4"
            "bc636a087c852ce/operations{?cursor,limit,order}\",\"templated\":true}},\"id\":\"0000000036b8da456b9b665ddb5df0ae949e051c91bfbcbe04f680cf4bc636a087c852ce\",\"asset\":\"SSLX:GBHFGY3ZNEJWLNO4LBUKLYOCEK4V7ENEBJGPRHHX7JU47GWHBREH37UR\",\"amount\":\"169.0000000\",\"sponsor\":\"GDB3RSSWTUXO7MBTNMHUP3DRBIUR3QRV2CVFRAKMN4GM2B4QNGEUT6CU\",\"last_modified_ledger\":64664554,\"last_modified_time\":\"2026-09-28T15:15:51Z\",\"claimants\":[{\"destination\":\"GCJM4OLEFM2CW6W6NH6K73E37MTCKFR6FME65OWQU7ZPGHDC52GIAJ6U\",\"predicate\":{\"not\":{\"abs_before\":\"2027-09-23T15:04:01Z\",\"abs_before_epoch\":\"1821711841\"}}},{\"destination\":\"GAFI2AOGBCKV7DWK2N77O34RD52C6O6QGGVEV5CEVFQTCHOOI3VOKP7G\",\"predicate\":{\"unconditional\":true}}],\"flags\":{\"clawback_enabled\":false},\"paging_token\":\"64664554-0000000036b8da456b9b665ddb5df0ae949e051c91bfbcbe04f680cf4bc636a087c852ce\"}]},\"_links\":{\"self\":{\"href\":\"https://horizon.example/x?cursor=&limit=2&order=asc\"},\"next\":{\"href\":\"https://horizon.example/x?cursor=2&limit=2&order=asc\"},\"prev\":{\"href\":\"https://horizon.example/x?cursor=1&limit=2&order=desc\"}}}");
    }

private slots:
};
ADD_TEST(ClaimableBalancePageDeserializerTest)
#endif // CLAIMABLEBALANCEPAGEDESERIALIZERTEST_H
