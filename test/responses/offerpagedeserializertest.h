#ifndef OFFERPAGEDESERIALIZERTEST_H
#define OFFERPAGEDESERIALIZERTEST_H

#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/offerresponse.h"
#include "../../src/responses/page.h"
#include "../../src/keypair.h"
#include "../../src/asset.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>

class OfferPageDeserializerTest: public QObject
{
    Q_OBJECT
    const QByteArray json ="{\n"
            "  \"_links\": {\n"
            "    \"self\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc\\u0026limit=10\\u0026cursor=\"\n"
            "    },\n"
            "    \"next\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc\\u0026limit=10\\u0026cursor=241\"\n"
            "    },\n"
            "    \"prev\": {\n"
            "      \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=desc\\u0026limit=10\\u0026cursor=241\"\n"
            "    }\n"
            "  },\n"
            "  \"_embedded\": {\n"
            "    \"records\": [\n"
            "      {\n"
            "        \"_links\": {\n"
            "          \"self\": {\n"
            "            \"href\": \"https://horizon-testnet.stellar.org/offers/241\"\n"
            "          },\n"
            "          \"offer_maker\": {\n"
            "            \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"\n"
            "          }\n"
            "        },\n"
            "        \"id\": 241,\n"
            "        \"paging_token\": \"241\",\n"
            "        \"seller\": \"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\",\n"
            "        \"selling\": {\n"
            "          \"asset_type\": \"credit_alphanum4\",\n"
            "          \"asset_code\": \"INR\",\n"
            "          \"asset_issuer\": \"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"\n"
            "        },\n"
            "        \"buying\": {\n"
            "          \"asset_type\": \"credit_alphanum4\",\n"
            "          \"asset_code\": \"USD\",\n"
            "          \"asset_issuer\": \"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"\n"
            "        },\n"
            "        \"amount\": \"10.0000000\",\n"
            "        \"price_r\": {\n"
            "          \"n\": 10,\n"
            "          \"d\": 1\n"
            "        },\n"
            "        \"price\": \"11.0000000\"\n,"
            "        \"last_modified_ledger\": 22200794,\n"
            "        \"last_modified_time\": \"2019-01-28T12:30:38Z\"\n"
            "      }\n"
            "    ]\n"
            "  }\n"
  "}";


    QByteArray withSponsor = "{\n"
        "  \"_links\": {\n"
        "    \"self\": {\n"
        "      \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc\\u0026limit=10\\u0026cursor=\"\n"
        "    },\n"
        "    \"next\": {\n"
        "      \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc\\u0026limit=10\\u0026cursor=241\"\n"
        "    },\n"
        "    \"prev\": {\n"
        "      \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=desc\\u0026limit=10\\u0026cursor=241\"\n"
        "    }\n"
        "  },\n"
        "  \"_embedded\": {\n"
        "    \"records\": [\n"
        "      {\n"
        "        \"_links\": {\n"
        "          \"self\": {\n"
        "            \"href\": \"https://horizon-testnet.stellar.org/offers/241\"\n"
        "          },\n"
        "          \"offer_maker\": {\n"
        "            \"href\": \"https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"\n"
        "          }\n"
        "        },\n"
        "        \"id\": 241,\n"
        "        \"paging_token\": \"241\",\n"
        "        \"seller\": \"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\",\n"
        "        \"selling\": {\n"
        "          \"asset_type\": \"credit_alphanum4\",\n"
        "          \"asset_code\": \"INR\",\n"
        "          \"asset_issuer\": \"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"\n"
        "        },\n"
        "        \"buying\": {\n"
        "          \"asset_type\": \"credit_alphanum4\",\n"
        "          \"asset_code\": \"USD\",\n"
        "          \"asset_issuer\": \"GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD\"\n"
        "        },\n"
        "        \"amount\": \"10.0000000\",\n"
        "        \"price_r\": {\n"
        "          \"n\": 10,\n"
        "          \"d\": 1\n"
        "        },\n"
        "        \"price\": \"11.0000000\"\n,"
        "        \"last_modified_ledger\": 22200794,\n"
        "        \"sponsor\": \"GCA7RXNKN7FGBLJVETJCUUXGXTCR6L2SJQFXDGMQCDET5YUE6KFNHQHO\",\n"
        "        \"last_modified_time\": \"2019-01-28T12:30:38Z\"\n"
        "      }\n"
        "    ]\n"
        "  }\n"
        "}";
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserialize() {
        Page<OfferResponse> offerPage;
        offerPage.loadFromJson(json);

        QCOMPARE(offerPage.get(0).getId(), 241);
        QCOMPARE(offerPage.get(0).getSeller()->getAccountId(), QString("GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD"));
        QCOMPARE(offerPage.get(0).getPagingToken(), QString("241"));
        QVERIFY(offerPage.get(0).getSelling()->equals(Asset::createNonNativeAsset("INR", KeyPair::fromAccountId(QString("GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD")))));
        QVERIFY(offerPage.get(0).getBuying()->equals( Asset::createNonNativeAsset("USD", KeyPair::fromAccountId(QString("GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD")))));
        QCOMPARE(offerPage.get(0).getAmount(), QString("10.0000000"));
        QCOMPARE(offerPage.get(0).getPrice(), QString("11.0000000"));
        QCOMPARE(offerPage.get(0).getPriceR().getNumerator(), 10);
        QCOMPARE(offerPage.get(0).getPriceR().getDenominator(), 1);
        QCOMPARE(offerPage.get(0).getLastModifiedLedger(), 22200794);
        QCOMPARE(offerPage.get(0).getLastModifiedTime(), "2019-01-28T12:30:38Z");
        QCOMPARE(offerPage.getLinks().getNext().getHref(), QString("https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc&limit=10&cursor=241"));
        QCOMPARE(offerPage.getLinks().getPrev().getHref(), QString("https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=desc&limit=10&cursor=241"));
        QCOMPARE(offerPage.getLinks().getSelf().getHref(), QString("https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc&limit=10&cursor="));
        QVERIFY(offerPage.get(0).getSponsor().isNull());

    }
    void testDeserializeWithSponsor() {
       Page<OfferResponse> offerPage;
       offerPage.loadFromJson(withSponsor);

       QCOMPARE(offerPage.get(0).getId(),  241);
       QCOMPARE(offerPage.get(0).getSeller()->getAccountId(), QString("GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD"));
       QCOMPARE(offerPage.get(0).getPagingToken(), "241");
       QVERIFY(offerPage.get(0).getSelling()->equals(Asset::createNonNativeAsset("INR", "GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD")));
       QVERIFY(offerPage.get(0).getBuying()->equals(Asset::createNonNativeAsset("USD", "GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD")));
       QCOMPARE(offerPage.get(0).getAmount(), "10.0000000");
       QCOMPARE(offerPage.get(0).getPrice(), "11.0000000");
       QCOMPARE(offerPage.get(0).getPriceR().getNumerator(), 10);
       QCOMPARE(offerPage.get(0).getPriceR().getDenominator(), 1);
       QCOMPARE(offerPage.get(0).getLastModifiedLedger(), 22200794);
       QCOMPARE(offerPage.get(0).getLastModifiedTime(), "2019-01-28T12:30:38Z");
       QCOMPARE(offerPage.get(0).getSponsor(), "GCA7RXNKN7FGBLJVETJCUUXGXTCR6L2SJQFXDGMQCDET5YUE6KFNHQHO");

       QCOMPARE(offerPage.getLinks().getNext().getHref(), "https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc&limit=10&cursor=241");
       QCOMPARE(offerPage.getLinks().getPrev().getHref(), "https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=desc&limit=10&cursor=241");
       QCOMPARE(offerPage.getLinks().getSelf().getHref(), "https://horizon-testnet.stellar.org/accounts/GA2IYMIZSAMDD6QQTTSIEL73H2BKDJQTA7ENDEEAHJ3LMVF7OYIZPXQD/offers?order=asc&limit=10&cursor=");
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
     * An offer's id, amount and price, exactly. A price term given as text,
     * as a fraction or past int32, or a zero denominator, used to read as 0
     * through getPriceR(); an id that is not a number as 0; an amount given
     * as a number as empty.
     */
    void testOfferIsReadExactly()
    {
        typedef Page<OfferResponse> Offers;
        const QString r = QStringLiteral("_embedded.records[0].");
        QVERIFY(refusalOf<Offers>(json, r + "price_r.n", QString("abc")).startsWith(r + "price_r.n: "));
        QVERIFY(refusalOf<Offers>(json, r + "price_r.n", 2.5).startsWith(r + "price_r.n: "));
        QVERIFY(refusalOf<Offers>(json, r + "price_r.n", 2147483648.0).startsWith(r + "price_r.n: "));
        QVERIFY(refusalOf<Offers>(json, r + "price_r.d", 0).startsWith(r + "price_r.d: "));
        QVERIFY(refusalOf<Offers>(json, r + "price_r.d", QJsonValue(QJsonValue::Undefined)).startsWith(r + "price_r.d: "));
        QVERIFY(refusalOf<Offers>(json, r + "price_r", QJsonValue(QJsonValue::Undefined)).startsWith(r + "price_r: "));
        QVERIFY(refusalOf<Offers>(json, r + "id", QString("241x")).startsWith(r + "id: "));
        QVERIFY(refusalOf<Offers>(json, r + "id", QString("9223372036854775808")).startsWith(r + "id: "));
        QVERIFY(refusalOf<Offers>(json, r + "amount", 10).startsWith(r + "amount: "));
        QVERIFY(refusalOf<Offers>(json, r + "price", QString("eleven")).startsWith(r + "price: "));
        QVERIFY(refusalOf<Offers>(json, r + "paging_token", QJsonValue(QJsonValue::Undefined)).startsWith(r + "paging_token: "));
        QVERIFY(refusalOf<Offers>(json, r + "paging_token", 241).startsWith(r + "paging_token: "));
        QVERIFY(refusalOf<Offers>(json, r + "selling.asset_code", QString("INRUPEE")).startsWith(r + "selling.asset_code: "));
        QVERIFY(refusalOf<Offers>(json, r + "buying.asset_type", QString("credit")).startsWith(r + "buying.asset_type: "));
        QVERIFY(refusalOf<Offers>(json, r + "seller", QJsonValue()).startsWith(r + "seller: "));
        // A term given as decimal text is the same number, exactly.
        QVERIFY(refusalOf<Offers>(json, r + "price_r.n", QString("10")).isEmpty());
    }

    /**
     * getPriceR() is exact whatever price_r holds: it read each term with
     * toInt(), so {"n": "abc", "d": 1} made the price 0 and a term past int32
     * another price.
     */
    void testGetPriceRIsExact()
    {
        OfferResponse offer;
        offer.setProperty("price_r", QVariantMap{{"n", QString("abc")}, {"d", 1}});
        QVERIFY_EXCEPTION_THROWN(offer.getPriceR(), std::runtime_error);
        offer.setProperty("price_r", QVariantMap{{"n", 4294967297.0}, {"d", 1}});
        QVERIFY_EXCEPTION_THROWN(offer.getPriceR(), std::runtime_error);
        offer.setProperty("price_r", QVariantMap{{"n", 7}, {"d", 3}});
        QCOMPARE(offer.getPriceR().getNumerator(), 7);
        QCOMPARE(offer.getPriceR().getDenominator(), 3);
    }

    /** Real pubnet offers load, with their exact ids and prices. */
    void testRealHorizonOffers()
    {
        Page<OfferResponse> offers;
        offers.loadFromJson(realPubnetOffersPage());
        QCOMPARE(offers.size(), 2);
        const QJsonObject first = QJsonDocument::fromJson(realPubnetOffersPage()).object()
                .value("_embedded").toObject().value("records").toArray().at(0).toObject();
        QCOMPARE(QString::number(offers.get(0).getId()), first.value("id").toString());
        QCOMPARE(offers.get(0).getPriceR().getNumerator(), first.value("price_r").toObject().value("n").toInt());
        QCOMPARE(offers.get(0).getPriceR().getDenominator(), first.value("price_r").toObject().value("d").toInt());
        QCOMPARE(offers.get(0).getAmount(), first.value("amount").toString());
    }

    /**
     * Every member Horizon always sends in an offer is required: missing or
     * null, the ledger it was last changed in read 0 and its links empty.
     * The time may be null, and the sponsor out.
     */
    void testMembersHorizonAlwaysSendsAreRequired()
    {
        typedef Page<OfferResponse> Offers;
        const QByteArray real = realPubnetOffersPage();
        const QString r = QStringLiteral("_embedded.records[0].");
        QVERIFY(refusalOf<Offers>(real, r + "paging_token",
                                  QJsonDocument::fromJson(real).object().value("_embedded").toObject()
                                      .value("records").toArray().at(0).toObject().value("paging_token")).isEmpty());
        for (const char* key : { "id", "paging_token", "seller", "selling", "buying", "amount", "price", "price_r",
                                 "last_modified_ledger", "_links", "_links.self", "_links.offer_maker",
                                 "_links.self.href" }) {
            QVERIFY2(refusalOf<Offers>(real, r + key, QJsonValue(QJsonValue::Undefined)).startsWith(r + key + ": "), key);
            QVERIFY2(refusalOf<Offers>(real, r + key, QJsonValue()).startsWith(r + key + ": "), key);
        }
        QVERIFY(refusalOf<Offers>(real, r + "last_modified_time", QJsonValue()).isEmpty());
        QVERIFY(refusalOf<Offers>(real, r + "sponsor", QJsonValue(QJsonValue::Undefined)).isEmpty());
    }
private:
    /** Two real pubnet offers. */
    static QByteArray realPubnetOffersPage()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/offers/1859390057\"},\"offer_maker\":{\"href\":\"https://horizon.stellar.org/accounts/GDXEGWXU52BSPF2IB4DZPPPMVWREVTOPHY5MAPFOMF6NKPDZESFARU5K\"}},\"id\":\"1859390057\",\"paging_token\":\"1859390057\",\"seller\":\"GDXEGWXU52BSPF2IB4DZPPPMVWREVTOPHY5MAPFOMF6NKPDZESFARU5K\",\"selling\":{\"asset_type\":\"native\"},\"buying\":{\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"USDC\",\"asset_issuer\":\"GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\"},\"amount\":\"1480.0000000\",\"price_r\":{\"n\":215399,\"d\":1000000},\"price\":\"0.2153990\",\"last_modified_ledger\":64664563,\"last_modified_time\":\"2026-09-28T15:16:36Z\"},{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/offers/1859390056\"},\"offer_maker\":{\"href\":\"https://horizon.stellar.org/accounts/GBGP2AAFN5SMGJ2BZNWKHSICIDC33T4WNGBGR2TP5CPFO6O3B7RVWR45\"}},\"id\":\"1859390056\",\"paging_token\":\"1859390056\",\"seller\":\"GBGP2AAFN5SMGJ2BZNWKHSICIDC33T4WNGBGR2TP5CPFO6O3B7RVWR45\",\"selling\":{\"asset_type\":\"native\"},\"buying\":{\"asset_type\":\"credit_alphanum4\",\"asset_code\":\"USDC\",\"asset_issuer\":\"GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\"},\"amount\":\"2233.4037005\",\"price_r\":{\"n\":133621907,\"d\":620696229},\"price\":\"0.2152775\",\"last_modified_ledger\":64664563,\"last_modified_time\":\"2026-09-28T15:16:36Z\"}]},\"_links\":{\"self\":{\"href\":\"https://horizon.example/x?cursor=&limit=2&order=asc\"},\"next\":{\"href\":\"https://horizon.example/x?cursor=2&limit=2&order=asc\"},\"prev\":{\"href\":\"https://horizon.example/x?cursor=1&limit=2&order=desc\"}}}");
    }

private slots:
};

ADD_TEST(OfferPageDeserializerTest)



#endif // OFFERPAGEDESERIALIZERTEST_H
