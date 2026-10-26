#ifndef TRADEAGGREGATIONSPAGEDESERIALIZERTEST_H
#define TRADEAGGREGATIONSPAGEDESERIALIZERTEST_H
#include <QObject>


#include <QtTest>
#include <functional>
#include <stdexcept>
#include "../testcollector.h"
#include "../../src/responses/page.h"
#include "../../src/responses/tradeaggregationresponse.h"
#include "../../src/keypair.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>
class TradeAggregationsPageDeserializerTest: public QObject
{
    Q_OBJECT

public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserialize() {
        Page<TradeAggregationResponse> page;
        page.loadFromJson(json);

        QCOMPARE(page.getLinks().getSelf().getHref(), "https://horizon.stellar.org/trade_aggregations?base_asset_type=native&start_time=1512689100000&counter_asset_issuer=GATEMHCCKCY67ZUCKTROYN24ZYT5GK4EQZ65JJLDHKHRUZI3EUEKMTCH&limit=200&end_time=1512775500000&counter_asset_type=credit_alphanum4&resolution=300000&order=asc&counter_asset_code=BTC");
        QCOMPARE(page.getLinks().getNext().getHref(), "https://horizon.stellar.org/trade_aggregations?base_asset_type=native&counter_asset_code=BTC&counter_asset_issuer=GATEMHCCKCY67ZUCKTROYN24ZYT5GK4EQZ65JJLDHKHRUZI3EUEKMTCH&counter_asset_type=credit_alphanum4&end_time=1512775500000&limit=200&order=asc&resolution=300000&start_time=1512765000000");

        QCOMPARE(page.get(0).getTimestamp(), (qint64)1512731100000L);
        QCOMPARE(page.get(0).getTradeCount(), (qint32)2);
        QCOMPARE(page.get(0).getBaseVolume(), QString("341.8032786"));
        QCOMPARE(page.get(0).getCounterVolume(), QString("0.0041700"));
        QCOMPARE(page.get(0).getAvg(), QString("0.0000122"));
        QCOMPARE(page.get(0).getHigh(), QString("0.0000123"));
        QCOMPARE(page.get(0).getLow(), QString("0.0000124"));
        QCOMPARE(page.get(0).getOpen(), QString("0.0000125"));
        QCOMPARE(page.get(0).getClose(), QString("0.0000126"));
        QCOMPARE(page.get(0).getHighR().getNumerator(), 123);
        QCOMPARE(page.get(0).getHighR().getDenominator(), 10000000);
        QCOMPARE(page.get(0).getLowR().getNumerator(), 124);
        QCOMPARE(page.get(0).getLowR().getDenominator(), 10000000);
        QCOMPARE(page.get(0).getOpenR().getNumerator(), 125);
        QCOMPARE(page.get(0).getOpenR().getDenominator(), 10000000);
        QCOMPARE(page.get(0).getCloseR().getNumerator(), 126);
        QCOMPARE(page.get(0).getCloseR().getDenominator(), 10000000);

        //support parse string timestamp and trade_count
        QCOMPARE(page.get(1).getTimestamp(), (qint64)1512732300000);
        QCOMPARE(page.get(1).getTradeCount(), (qint32)1);
    }

    /**
     * Real pubnet XLM/USDC hourly buckets (Horizon, 2026-09-25): Horizon sends
     * high_r/low_r/open_r/close_r as int64 strings and busy buckets exceed
     * int32. get*R() used to wrap each term mod 2^32: low_r
     * 1353159973/6180000000 (0.219) came back as 1353159973/1885032704, a
     * plausible-looking 0.718, and 2151171214/10000000000 as
     * -2143796082/1410065408. Now the int64 getters are exact and the Price
     * getters refuse what does not fit, even after reducing by the gcd.
     */
    void testInt64TermsAreExactAndNeverWrapped() {
        const QByteArray aggJson = "{ \"_embedded\": { \"records\": ["
            " {\"timestamp\": \"1790330400000\", \"trade_count\": \"1558\","
            "  \"base_volume\": \"1.0000000\", \"counter_volume\": \"0.2200000\", \"avg\": \"0.2200000\","
            "  \"high\": \"0.2219476\", \"high_r\": {\"n\": \"22194759\", \"d\": \"100000000\"},"
            "  \"low\": \"0.2189579\", \"low_r\": {\"n\": \"1353159973\", \"d\": \"6180000000\"},"
            "  \"open\": \"0.2201839\", \"open_r\": {\"n\": \"2201839\", \"d\": \"10000000\"},"
            "  \"close\": \"0.2199108\", \"close_r\": {\"n\": \"2199108\", \"d\": \"10000000\"}},"
            " {\"timestamp\": \"1790341200000\", \"trade_count\": \"3995\","
            "  \"base_volume\": \"1.0000000\", \"counter_volume\": \"0.2200000\", \"avg\": \"0.2200000\","
            "  \"high\": \"0.2240287\", \"high_r\": {\"n\": \"3926525294\", \"d\": \"17526884944\"},"
            "  \"low\": \"0.2190609\", \"low_r\": {\"n\": \"4943064899\", \"d\": \"22564802740\"},"
            "  \"open\": \"0.2217350\", \"open_r\": {\"n\": \"380995634\", \"d\": \"1718247583\"},"
            "  \"close\": \"0.2230139\", \"close_r\": {\"n\": \"2230139\", \"d\": \"10000000\"}},"
            " {\"timestamp\": \"1790344800000\", \"trade_count\": \"2947\","
            "  \"base_volume\": \"1.0000000\", \"counter_volume\": \"0.2200000\", \"avg\": \"0.2200000\","
            "  \"high\": \"0.2202244\", \"high_r\": {\"n\": \"22022439\", \"d\": \"100000000\"},"
            "  \"low\": \"0.2151171\", \"low_r\": {\"n\": \"2151171214\", \"d\": \"10000000000\"},"
            "  \"open\": \"0.2198000\", \"open_r\": {\"n\": \"2000000\", \"d\": \"9099181\"},"
            "  \"close\": \"0.2156450\", \"close_r\": {\"n\": \"10782249\", \"d\": \"50000000\"}},"
            " {\"timestamp\": 1, \"trade_count\": \"1\", \"high_r\": {\"n\": \"4294967296\", \"d\": \"2147483648\"},"
            "  \"base_volume\": \"1.0000000\", \"counter_volume\": \"1.0000000\", \"avg\": \"1.0000000\","
            "  \"high\": \"2.0000000\", \"low\": \"1.0000000\", \"open\": \"1.0000000\", \"close\": \"0.5000000\","
            "  \"low_r\": {\"n\": \"4294967297\", \"d\": \"4294967296\"},"
            "  \"open_r\": {\"n\": 4294967297, \"d\": 4294967296},"
            "  \"close_r\": {\"n\": \"1\", \"d\": \"-2\"}}"
            "] } }";
        // A negative term is refused when the page is read (it was kept, for
        // getCloseR() to throw later and getCloseRN() to hand it out), and so
        // is a missing close price, which Horizon always sends (it used to
        // read, getCloseR() throwing when asked).
        Page<TradeAggregationResponse> refused;
        try {
            refused.loadFromJson(aggJson);
            QFAIL("a negative price term was read");
        } catch (const std::runtime_error& e) {
            QVERIFY(QString::fromUtf8(e.what()).startsWith("_embedded.records[3].close_r.d: "));
        }
        QJsonObject valid = QJsonDocument::fromJson(aggJson).object();
        QJsonObject embedded = valid.value("_embedded").toObject();
        QJsonArray records = embedded.value("records").toArray();
        QJsonObject fourth = records.at(3).toObject();
        fourth.remove("close_r");
        records[3] = fourth;
        embedded["records"] = records;
        valid["_embedded"] = embedded;
        Page<TradeAggregationResponse> noClose;
        try {
            noClose.loadFromJson(QJsonDocument(valid).toJson());
            QFAIL("an aggregation without its close price was read");
        } catch (const std::runtime_error& e) {
            QVERIFY(QString::fromUtf8(e.what()).startsWith("_embedded.records[3].close_r: "));
        }
        fourth.insert("close_r", QJsonObject{{"n", "1"}, {"d", "2"}});
        records[3] = fourth;
        embedded["records"] = records;
        valid["_embedded"] = embedded;
        Page<TradeAggregationResponse> page;
        page.loadFromJson(QJsonDocument(valid).toJson());
        QCOMPARE(page.size(), 4);

        const TradeAggregationResponse& a = page.get(0);
        QCOMPARE(a.getLowRN(), Q_INT64_C(1353159973));
        QCOMPARE(a.getLowRD(), Q_INT64_C(6180000000));
        QVERIFY(refusedNaming("getLowRN()/getLowRD()", [&]() { a.getLowR(); }));
        QCOMPARE(a.getHighR(), Price(22194759, 100000000));
        QCOMPARE(a.getOpenR(), Price(2201839, 10000000));
        QCOMPARE(a.getCloseR(), Price(2199108, 10000000));

        const TradeAggregationResponse& b = page.get(1);
        QCOMPARE(b.getHighRN(), Q_INT64_C(3926525294));
        QCOMPARE(b.getHighRD(), Q_INT64_C(17526884944));   // gcd 2: still too big
        QVERIFY(refusedNaming("getHighRN()/getHighRD()", [&]() { b.getHighR(); }));
        QCOMPARE(b.getLowRN(), Q_INT64_C(4943064899));
        QCOMPARE(b.getLowRD(), Q_INT64_C(22564802740));
        QVERIFY(refusedNaming("getLowRN()/getLowRD()", [&]() { b.getLowR(); }));
        QCOMPARE(b.getOpenR(), Price(380995634, 1718247583));
        QCOMPARE(b.getCloseRN(), Q_INT64_C(2230139));
        QCOMPARE(b.getCloseRD(), Q_INT64_C(10000000));

        const TradeAggregationResponse& c = page.get(2);
        QCOMPARE(c.getLowRN(), Q_INT64_C(2151171214));
        QCOMPARE(c.getLowRD(), Q_INT64_C(10000000000));
        QVERIFY(refusedNaming("getLowRN()/getLowRD()", [&]() { c.getLowR(); }));
        QCOMPARE(c.getOpenR(), Price(2000000, 9099181));
        QCOMPARE(c.getCloseR(), Price(10782249, 50000000));

        const TradeAggregationResponse& d = page.get(3);
        QCOMPARE(d.getHighR(), Price(2, 1));                           // reduced to fit
        QVERIFY(refusedNaming("getLowRN()/getLowRD()", [&]() { d.getLowR(); }));   // was 1/0
        QCOMPARE(d.getOpenRN(), Q_INT64_C(4294967297));                // JSON numbers too
        QCOMPARE(d.getOpenRD(), Q_INT64_C(4294967296));
        QCOMPARE(d.getCloseR(), Price(1, 2));
        // Missing terms throw instead of reading as 0 (a price set by hand).
        TradeAggregationResponse bare;
        bare.setProperty("close_r", QVariantMap());
        QVERIFY(refusedNaming("price.n", [&]() { bare.getCloseR(); }));
    }

private:
    static bool refusedNaming(const char* text, const std::function<void()>& read)
    {
        try {
            read();
        } catch (const std::runtime_error& e) {
            return QString::fromUtf8(e.what()).contains(QLatin1String(text));
        }
        return false;
    }

    QByteArray json = "{\n"
                      "  \"_links\": {\n"
                      "    \"self\": {\n"
                      "      \"href\": \"https://horizon.stellar.org/trade_aggregations?base_asset_type=native\\u0026start_time=1512689100000\\u0026counter_asset_issuer=GATEMHCCKCY67ZUCKTROYN24ZYT5GK4EQZ65JJLDHKHRUZI3EUEKMTCH\\u0026limit=200\\u0026end_time=1512775500000\\u0026counter_asset_type=credit_alphanum4\\u0026resolution=300000\\u0026order=asc\\u0026counter_asset_code=BTC\"\n"
                      "    },\n"
                      "    \"next\": {\n"
                      "      \"href\": \"https://horizon.stellar.org/trade_aggregations?base_asset_type=native\\u0026counter_asset_code=BTC\\u0026counter_asset_issuer=GATEMHCCKCY67ZUCKTROYN24ZYT5GK4EQZ65JJLDHKHRUZI3EUEKMTCH\\u0026counter_asset_type=credit_alphanum4\\u0026end_time=1512775500000\\u0026limit=200\\u0026order=asc\\u0026resolution=300000\\u0026start_time=1512765000000\"\n"
                      "    }\n"
                      "  },\n"
                      "  \"_embedded\": {\n"
                      "    \"records\": [\n"
                      "      {\n"
                      "        \"timestamp\": 1512731100000,\n"
                      "        \"trade_count\": 2,\n"
                      "        \"base_volume\": \"341.8032786\",\n"
                      "        \"counter_volume\": \"0.0041700\",\n"
                      "        \"avg\": \"0.0000122\",\n"
                      "        \"high\": \"0.0000123\",\n"
                      "        \"high_r\": {\n"
                      "          \"n\": \"123\",\n"
                      "          \"d\": \"10000000\"\n"
                      "        },\n"
                      "        \"low\": \"0.0000124\",\n"
                      "        \"low_r\": {\n"
                      "          \"n\": \"124\",\n"
                      "          \"d\": \"10000000\"\n"
                      "        },\n"
                      "        \"open\": \"0.0000125\",\n"
                      "        \"open_r\": {\n"
                      "          \"n\": \"125\",\n"
                      "          \"d\": \"10000000\"\n"
                      "        },\n"
                      "        \"close\": \"0.0000126\",\n"
                      "        \"close_r\": {\n"
                      "          \"n\": \"126\",\n"
                      "          \"d\": \"10000000\"\n"
                      "        }\n"
                      "      },\n"
                      "      {\n"
                      "        \"timestamp\": \"1512732300000\",\n"
                      "        \"trade_count\": \"1\",\n"
                      "        \"base_volume\": \"233.6065573\",\n"
                      "        \"counter_volume\": \"0.0028500\",\n"
                      "        \"avg\": \"0.0000122\",\n"
                      "        \"high\": \"0.0000122\",\n"
                      "        \"low\": \"0.0000122\",\n"
                      "        \"open\": \"0.0000122\",\n"
                      "        \"close\": \"0.0000122\",\n"
                      "        \"high_r\": {\n"
                      "          \"n\": \"122\",\n"
                      "          \"d\": \"10000000\"\n"
                      "        },\n"
                      "        \"low_r\": {\n"
                      "          \"n\": \"122\",\n"
                      "          \"d\": \"10000000\"\n"
                      "        },\n"
                      "        \"open_r\": {\n"
                      "          \"n\": \"122\",\n"
                      "          \"d\": \"10000000\"\n"
                      "        },\n"
                      "        \"close_r\": {\n"
                      "          \"n\": \"122\",\n"
                      "          \"d\": \"10000000\"\n"
                      "        }\n"
                      "      },\n"
                      "      {\n"
                      "        \"timestamp\": 1512764700000,\n"
                      "        \"trade_count\": 1,\n"
                      "        \"base_volume\": \"451.0000000\",\n"
                      "        \"counter_volume\": \"0.0027962\",\n"
                      "        \"avg\": \"0.0000062\",\n"
                      "        \"high\": \"0.0000062\",\n"
                      "        \"low\": \"0.0000062\",\n"
                      "        \"open\": \"0.0000062\",\n"
                      "        \"close\": \"0.0000062\",\n"
                      "        \"high_r\": {\n"
                      "          \"n\": \"62\",\n"
                      "          \"d\": \"10000000\"\n"
                      "        },\n"
                      "        \"low_r\": {\n"
                      "          \"n\": \"62\",\n"
                      "          \"d\": \"10000000\"\n"
                      "        },\n"
                      "        \"open_r\": {\n"
                      "          \"n\": \"62\",\n"
                      "          \"d\": \"10000000\"\n"
                      "        },\n"
                      "        \"close_r\": {\n"
                      "          \"n\": \"62\",\n"
                      "          \"d\": \"10000000\"\n"
                      "        }\n"
                      "      }\n"
                      "    ]\n"
                      "  }\n"
                      "}";

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
     * An aggregation's counts, volumes and prices, exactly: a trade count
     * or timestamp that is not a whole number read as 0, and a price term
     * that is not one, or a zero denominator, was kept to fail later.
     */
    void testTradeAggregationIsReadExactly()
    {
        typedef Page<TradeAggregationResponse> Aggregations;
        const QByteArray real = realPubnetTradeAggregations();
        const QString r = QStringLiteral("_embedded.records[0].");
        QVERIFY(refusalOf<Aggregations>(real, r + "trade_count", QString("2322.0")).startsWith(r + "trade_count: "));
        QVERIFY(refusalOf<Aggregations>(real, r + "timestamp", QString("yesterday")).startsWith(r + "timestamp: "));
        QVERIFY(refusalOf<Aggregations>(real, r + "high_r.d", QString("0")).startsWith(r + "high_r.d: "));
        QVERIFY(refusalOf<Aggregations>(real, r + "close_r.n", QString("2283.5")).startsWith(r + "close_r.n: "));
        QVERIFY(refusalOf<Aggregations>(real, r + "trade_count", QJsonValue(QJsonValue::Undefined)).startsWith(r + "trade_count: "));
        QVERIFY(refusalOf<Aggregations>(real, r + "timestamp", QJsonValue()).startsWith(r + "timestamp: "));
        QVERIFY(refusalOf<Aggregations>(real, r + "base_volume", 842963.1140368).startsWith(r + "base_volume: "));
        QVERIFY(refusalOf<Aggregations>(real, r + "avg", QString("0.23 USDC")).startsWith(r + "avg: "));
    }

    /** Real pubnet XLM/USDC hourly aggregations load with their exact values. */
    void testRealHorizonTradeAggregations()
    {
        Page<TradeAggregationResponse> aggregations;
        aggregations.loadFromJson(realPubnetTradeAggregations());
        QCOMPARE(aggregations.size(), 2);
        QCOMPARE(aggregations.get(0).getTimestamp(), Q_INT64_C(1790622000000));
        QCOMPARE(aggregations.get(0).getTradeCount(), 2322);
        QCOMPARE(aggregations.get(0).getBaseVolume(), QString("842963.1140368"));
        QCOMPARE(aggregations.get(0).getHighRN(), Q_INT64_C(5000000));
        QCOMPARE(aggregations.get(0).getHighRD(), Q_INT64_C(21477663));
        QCOMPARE(aggregations.get(0).getCloseR().getNumerator(), 2283);
    }

    /**
     * Every member Horizon always sends in an aggregation is required.
     * Missing or null, the volumes and prices read "", and the rationals as
     * no price at all, as if the market had not traded in the interval.
     */
    void testMembersHorizonAlwaysSendsAreRequired()
    {
        typedef Page<TradeAggregationResponse> Aggregations;
        const QByteArray real = realPubnetTradeAggregations();
        const QString r = QStringLiteral("_embedded.records[0].");
        QVERIFY(refusalOf<Aggregations>(real, r + "avg", QString("0.2302331")).isEmpty());
        for (const char* key : { "timestamp", "trade_count", "base_volume", "counter_volume", "avg", "high",
                                 "high_r", "low", "low_r", "open", "open_r", "close", "close_r" }) {
            QVERIFY2(refusalOf<Aggregations>(real, r + key, QJsonValue(QJsonValue::Undefined))
                         .startsWith(r + key + ": "), key);
            QVERIFY2(refusalOf<Aggregations>(real, r + key, QJsonValue()).startsWith(r + key + ": "), key);
        }
    }
private:
    /** Two real pubnet XLM/USDC hourly trade aggregations. */
    static QByteArray realPubnetTradeAggregations()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"timestamp\":\"1790622000000\",\"trade_count\":\"2322\",\"base_volume\":\"842963.1140368\",\"counter_volume\":\"194078.0159635\",\"avg\":\"0.2302331\",\"high\":\"0.2328000\",\"high_r\":{\"n\":\"5000000\",\"d\":\"21477663\"},\"low\":\"0.2281012\",\"low_r\":{\"n\":\"22810121\",\"d\":\"100000000\"},\"open\":\"0.2282047\",\"open_r\":{\"n\":\"2282047\",\"d\":\"10000000\"},\"close\":\"0.2282087\",\"close_r\":{\"n\":\"2283\",\"d\":\"10004\"}},{\"timestamp\":\"1790618400000\",\"trade_count\":\"7006\",\"base_volume\":\"1853011.7441331\",\"counter_volume\":\"428408.1612221\",\"avg\":\"0.2311956\",\"high\":\"0.2346164\",\"high_r\":{\"n\":\"23461639\",\"d\":\"100000000\"},\"low\":\"0.2254100\",\"low_r\":{\"n\":\"10000000\",\"d\":\"44363611\"},\"open\":\"0.2304286\",\"open_r\":{\"n\":\"46099539\",\"d\":\"200060000\"},\"close\":\"0.2254100\",\"close_r\":{\"n\":\"10000000\",\"d\":\"44363611\"}}]},\"_links\":{\"self\":{\"href\":\"https://horizon.example/x?cursor=&limit=2&order=asc\"},\"next\":{\"href\":\"https://horizon.example/x?cursor=2&limit=2&order=asc\"},\"prev\":{\"href\":\"https://horizon.example/x?cursor=1&limit=2&order=desc\"}}}");
    }

private slots:
};
ADD_TEST(TradeAggregationsPageDeserializerTest)
#endif // TRADEAGGREGATIONSPAGEDESERIALIZERTEST_H
