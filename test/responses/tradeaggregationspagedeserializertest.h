#ifndef TRADEAGGREGATIONSPAGEDESERIALIZERTEST_H
#define TRADEAGGREGATIONSPAGEDESERIALIZERTEST_H
#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/page.h"
#include "../../src/responses/tradeaggregationresponse.h".h"
#include "../../src/keypair.h"
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
                      "        \"low\": \"0.0000124\",\n"
                      "        \"open\": \"0.0000125\",\n"
                      "        \"close\": \"0.0000126\"\n"
                      "      },\n"
                      "      {\n"
                      "        \"timestamp\": 1512732300000,\n"
                      "        \"trade_count\": 1,\n"
                      "        \"base_volume\": \"233.6065573\",\n"
                      "        \"counter_volume\": \"0.0028500\",\n"
                      "        \"avg\": \"0.0000122\",\n"
                      "        \"high\": \"0.0000122\",\n"
                      "        \"low\": \"0.0000122\",\n"
                      "        \"open\": \"0.0000122\",\n"
                      "        \"close\": \"0.0000122\"\n"
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
                      "        \"close\": \"0.0000062\"\n"
                      "      }\n"
                      "    ]\n"
                      "  }\n"
                      "}";
};
ADD_TEST(TradeAggregationsPageDeserializerTest)
#endif // TRADEAGGREGATIONSPAGEDESERIALIZERTEST_H
