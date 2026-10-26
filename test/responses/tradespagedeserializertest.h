#ifndef TRADESPAGEDESERIALIZERTEST_H
#define TRADESPAGEDESERIALIZERTEST_H
#include <QObject>


#include <QtTest>
#include <functional>
#include <stdexcept>
#include "../testcollector.h"
#include "../../src/responses/page.h"
#include "../../src/responses/traderesponse.h"
#include "../../src/keypair.h"
#include "../../src/assettypenative.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>

class TradesPageDeserializerTest: public QObject
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
        Page<TradeResponse> tradesPage;
        tradesPage.loadFromJson(json);
        QCOMPARE(tradesPage.getLinks().getNext().getHref(), QString("https://horizon.stellar.org/trades?cursor=3748308153536513-0&limit=10&order=asc"));
        QCOMPARE(tradesPage.getLinks().getPrev().getHref(), QString("https://horizon.stellar.org/trades?cursor=3697472920621057-0&limit=10&order=desc"));
        QCOMPARE(tradesPage.getLinks().getSelf().getHref(), QString("https://horizon.stellar.org/trades?cursor=&limit=10&order=asc"));

        QCOMPARE(tradesPage.get(0).getId(), QString("3697472920621057-0"));
        QCOMPARE(tradesPage.get(0).getPagingToken(), QString("3697472920621057-0"));
        QCOMPARE(tradesPage.get(0).getLedgerCloseTime(), QString("2015-11-18T03:47:47Z"));
        QCOMPARE(tradesPage.get(0).getOfferId(),QString("9"));
        QCOMPARE(tradesPage.get(0).getBaseOfferId(), QString("10"));
        QCOMPARE(tradesPage.get(0).getCounterOfferId(), QString("11"));

        QCOMPARE(tradesPage.get(0).isBaseSeller(),true);
        QVERIFY(tradesPage.get(0).getBaseAsset()->equals(new AssetTypeNative()));
        QVERIFY(tradesPage.get(0).getCounterAsset()->equals( Asset::createNonNativeAsset("JPY", KeyPair::fromAccountId(QString("GBVAOIACNSB7OVUXJYC5UE2D4YK2F7A24T7EE5YOMN4CE6GCHUTOUQXM")))));

        QCOMPARE(tradesPage.get(0).getPrice().getNumerator(), 267);
        QCOMPARE(tradesPage.get(0).getPrice().getDenominator(), 1000);
        QCOMPARE(tradesPage.get(0).getLiquidityPoolFeeBp(), 0);
        QCOMPARE(tradesPage.get(0).getTradeType(), QString("orderbook"));
        QCOMPARE(tradesPage.get(1).getBaseAccount().getAccountId(), QString("GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G"));
    }

    void testDeserializeLiquidityPoolTrade() {
        QByteArray lpJson = "{"
            "  \"_embedded\": { \"records\": [ {"
            "    \"_links\": { \"self\": { \"href\": \"\" }, \"base\": { \"href\": \"\" }, \"counter\": { \"href\": \"\" }, \"operation\": { \"href\": \"\" } },"
            "    \"id\": \"123456789-0\","
            "    \"paging_token\": \"123456789-0\","
            "    \"ledger_close_time\": \"2021-11-18T03:47:47Z\","
            "    \"trade_type\": \"liquidity_pool\","
            "    \"liquidity_pool_fee_bp\": 30,"
            "    \"base_liquidity_pool_id\": \"abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789\","
            "    \"base_amount\": \"10.0000000\","
            "    \"base_asset_type\": \"native\","
            "    \"counter_account\": \"GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\","
            "    \"counter_amount\": \"2.6700000\","
            "    \"counter_asset_type\": \"credit_alphanum4\","
            "    \"counter_asset_code\": \"JPY\","
            "    \"counter_asset_issuer\": \"GBVAOIACNSB7OVUXJYC5UE2D4YK2F7A24T7EE5YOMN4CE6GCHUTOUQXM\","
            "    \"base_is_seller\": true,"
            "    \"price\": { \"n\": 267, \"d\": 1000 }"
            "  } ] }"
            "}";

        Page<TradeResponse> tradesPage;
        tradesPage.loadFromJson(lpJson);

        QCOMPARE(tradesPage.get(0).getTradeType(), QString("liquidity_pool"));
        QCOMPARE(tradesPage.get(0).getLiquidityPoolFeeBp(), 30);
        QCOMPARE(tradesPage.get(0).getBaseLiquidityPoolId(), QString("abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789"));
        QVERIFY(tradesPage.get(0).getCounterLiquidityPoolId().isEmpty());
        QCOMPARE(tradesPage.get(0).getPrice().getNumerator(), 267);
    }

    /**
     * Horizon sends the trade price as int64 strings, and liquidity-pool
     * trades often need more than 32 bits. The first two records are real
     * pubnet trades (Horizon, 2026-09-25). getPrice() used to wrap each term
     * mod 2^32: 37286884178/822576135 read as -1367821486/822576135,
     * 477/964549061895 as 477/-1818579705, 4294967297/4294967296 as 1/0.
     * Now the int64 getters are exact and getPrice() refuses what does not
     * fit, even after reducing by the gcd.
     */
    void testInt64TradePriceIsExactAndNeverWrapped() {
        const QByteArray lpJson = "{ \"_embedded\": { \"records\": ["
            " {\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"counter\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/277506538598866945\"}},\"ledger_close_time\":\"2026-09-28T15:16:12Z\", \"id\": \"277506538598866945-1\", \"paging_token\": \"1-0\", \"base_asset_type\": \"native\", \"counter_asset_type\": \"native\", \"base_is_seller\": true, \"trade_type\": \"liquidity_pool\","
            "   \"base_amount\": \"82.2576135\", \"counter_amount\": \"3728.6884178\","
            "   \"price\": {\"n\": \"37286884178\", \"d\": \"822576135\"} },"
            " {\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"counter\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/277506538598608897\"}},\"ledger_close_time\":\"2026-09-28T15:16:12Z\", \"id\": \"277506538598608897-2\", \"paging_token\": \"1-0\", \"base_asset_type\": \"native\", \"counter_asset_type\": \"native\", \"base_is_seller\": true, \"trade_type\": \"liquidity_pool\","
            "   \"base_amount\": \"96454.9061895\", \"counter_amount\": \"0.0000477\","
            "   \"price\": {\"n\": \"477\", \"d\": \"964549061895\"} },"
            " {\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"counter\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/2\"}},\"ledger_close_time\":\"2026-09-28T15:16:12Z\",\"trade_type\":\"orderbook\", \"id\": \"2\", \"paging_token\": \"1-0\", \"base_asset_type\": \"native\", \"counter_asset_type\": \"native\", \"base_is_seller\": true, \"base_amount\": \"1.0000000\", \"counter_amount\": \"1.0000000\", \"price\": {\"n\": \"4294967297\", \"d\": \"4294967296\"} },"
            " {\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"counter\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/3\"}},\"ledger_close_time\":\"2026-09-28T15:16:12Z\",\"trade_type\":\"orderbook\", \"id\": \"3\", \"paging_token\": \"1-0\", \"base_asset_type\": \"native\", \"counter_asset_type\": \"native\", \"base_is_seller\": true, \"base_amount\": \"1.0000000\", \"counter_amount\": \"1.0000000\", \"price\": {\"n\": \"4294967296\", \"d\": \"2147483648\"} },"
            " {\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"counter\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/4\"}},\"ledger_close_time\":\"2026-09-28T15:16:12Z\",\"trade_type\":\"orderbook\", \"id\": \"4\", \"paging_token\": \"1-0\", \"base_asset_type\": \"native\", \"counter_asset_type\": \"native\", \"base_is_seller\": true, \"base_amount\": \"1.0000000\", \"counter_amount\": \"1.0000000\", \"price\": {\"n\": \"2\", \"d\": \"4\"} },"
            " {\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"counter\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/5\"}},\"ledger_close_time\":\"2026-09-28T15:16:12Z\",\"trade_type\":\"orderbook\", \"id\": \"5\", \"paging_token\": \"1-0\", \"base_asset_type\": \"native\", \"counter_asset_type\": \"native\", \"base_is_seller\": true, \"base_amount\": \"1.0000000\", \"counter_amount\": \"1.0000000\", \"price\": {\"n\": \"2147483647\", \"d\": \"1\"} },"
            " {\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"counter\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/6\"}},\"ledger_close_time\":\"2026-09-28T15:16:12Z\",\"trade_type\":\"orderbook\", \"id\": \"6\", \"paging_token\": \"1-0\", \"base_asset_type\": \"native\", \"counter_asset_type\": \"native\", \"base_is_seller\": true, \"base_amount\": \"1.0000000\", \"counter_amount\": \"1.0000000\", \"price\": {\"n\": 4294967297, \"d\": 4294967296} },"
            " {\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"counter\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/7\"}},\"ledger_close_time\":\"2026-09-28T15:16:12Z\",\"trade_type\":\"orderbook\", \"id\": \"7\", \"paging_token\": \"1-0\", \"base_asset_type\": \"native\", \"counter_asset_type\": \"native\", \"base_is_seller\": true, \"base_amount\": \"1.0000000\", \"counter_amount\": \"1.0000000\", \"price\": {\"n\": \"12abc\", \"d\": \"1\"} },"
            " {\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"counter\":{\"href\":\"https://horizon-testnet.stellar.org/accounts/\"},\"operation\":{\"href\":\"https://horizon-testnet.stellar.org/operations/8\"}},\"ledger_close_time\":\"2026-09-28T15:16:12Z\",\"trade_type\":\"orderbook\", \"id\": \"8\", \"paging_token\": \"1-0\", \"base_asset_type\": \"native\", \"counter_asset_type\": \"native\", \"base_is_seller\": true, \"base_amount\": \"1.0000000\", \"counter_amount\": \"1.0000000\" }"
            "] } }";
        // A term that is not a number is refused when the page is read (it
        // was kept, for getPriceN() to throw later), and so is a missing
        // price, which Horizon always sends (it used to read, its getters
        // throwing when asked).
        Page<TradeResponse> refused;
        try {
            refused.loadFromJson(lpJson);
            QFAIL("a price term that is not a number was read");
        } catch (const std::runtime_error& e) {
            QVERIFY(QString::fromUtf8(e.what()).startsWith("_embedded.records[7].price.n: "));
        }
        QJsonObject withoutBadTerm = QJsonDocument::fromJson(lpJson).object();
        QJsonObject embedded = withoutBadTerm.value("_embedded").toObject();
        QJsonArray records = embedded.value("records").toArray();
        QJsonObject seventh = records.at(7).toObject();
        seventh.remove("price");
        records[7] = seventh;
        embedded["records"] = records;
        withoutBadTerm["_embedded"] = embedded;
        Page<TradeResponse> noPrice;
        try {
            noPrice.loadFromJson(QJsonDocument(withoutBadTerm).toJson());
            QFAIL("a trade without its price was read");
        } catch (const std::runtime_error& e) {
            QVERIFY(QString::fromUtf8(e.what()).startsWith("_embedded.records[7].price: "));
        }
        records.removeLast();
        records.removeLast();
        embedded["records"] = records;
        withoutBadTerm["_embedded"] = embedded;
        Page<TradeResponse> page;
        page.loadFromJson(QJsonDocument(withoutBadTerm).toJson());
        QCOMPARE(page.size(), 7);

        QCOMPARE(page.get(0).getPriceN(), Q_INT64_C(37286884178));
        QCOMPARE(page.get(0).getPriceD(), Q_INT64_C(822576135));
        QVERIFY(refusedNaming("getPriceN()/getPriceD()", [&]() { page.get(0).getPrice(); }));
        QCOMPARE(page.get(1).getPriceN(), Q_INT64_C(477));
        QCOMPARE(page.get(1).getPriceD(), Q_INT64_C(964549061895));   // gcd 3: still too big
        QVERIFY(refusedNaming("getPriceN()/getPriceD()", [&]() { page.get(1).getPrice(); }));
        QCOMPARE(page.get(2).getPriceN(), Q_INT64_C(4294967297));
        QVERIFY(refusedNaming("getPriceN()/getPriceD()", [&]() { page.get(2).getPrice(); }));

        QCOMPARE(page.get(3).getPrice(), Price(2, 1));        // reduced to fit
        QCOMPARE(page.get(4).getPrice(), Price(2, 4));        // fits: as sent
        QCOMPARE(page.get(5).getPrice(), Price(2147483647, 1));
        // JSON numbers too, exact up to 2^53.
        QCOMPARE(page.get(6).getPriceN(), Q_INT64_C(4294967297));
        QCOMPARE(page.get(6).getPriceD(), Q_INT64_C(4294967296));
        // Missing terms throw instead of reading as 0 (a price set by hand).
        TradeResponse bare;
        bare.setProperty("price", QVariantMap());
        QVERIFY(refusedNaming("price.n", [&]() { bare.getPriceN(); }));
        QVERIFY(refusedNaming("price.n", [&]() { bare.getPrice(); }));
        QVERIFY(refusedNaming("price.d", [&]() { bare.getPriceD(); }));
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

      const QByteArray json = "{\n"
              "  \"_links\": {\n"
              "    \"self\": {\n"
              "      \"href\": \"https://horizon.stellar.org/trades?cursor=\\u0026limit=10\\u0026order=asc\"\n"
              "    },\n"
              "    \"next\": {\n"
              "      \"href\": \"https://horizon.stellar.org/trades?cursor=3748308153536513-0\\u0026limit=10\\u0026order=asc\"\n"
              "    },\n"
              "    \"prev\": {\n"
              "      \"href\": \"https://horizon.stellar.org/trades?cursor=3697472920621057-0\\u0026limit=10\\u0026order=desc\"\n"
              "    }\n"
              "  },\n"
              "  \"_embedded\": {\n"
              "    \"records\": [\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"self\": {\n"
              "            \"href\": \"\"\n"
              "          },\n"
              "          \"base\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\"\n"
              "          },\n"
              "          \"counter\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\"\n"
              "          },\n"
              "          \"operation\": {\n"
              "            \"href\": \"https://horizon.stellar.org/operations/3697472920621057\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"3697472920621057-0\",\n"
              "        \"paging_token\": \"3697472920621057-0\",\n"
              "        \"ledger_close_time\": \"2015-11-18T03:47:47Z\",\n"
              "        \"offer_id\": \"9\",\n"
              "        \"base_offer_id\": \"10\",\n"
              "        \"base_account\": \"GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\",\n"
              "        \"base_amount\": \"10.0000000\",\n"
              "        \"base_asset_type\": \"native\",\n"
              "        \"counter_offer_id\": \"11\",\n"
              "        \"counter_account\": \"GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\",\n"
              "        \"counter_amount\": \"2.6700000\",\n"
              "        \"counter_asset_type\": \"credit_alphanum4\",\n"
              "        \"counter_asset_code\": \"JPY\",\n"
              "        \"counter_asset_issuer\": \"GBVAOIACNSB7OVUXJYC5UE2D4YK2F7A24T7EE5YOMN4CE6GCHUTOUQXM\",\n"
              "        \"base_is_seller\": true,\n"
              "        \"price\": {\n"
              "          \"n\": 267,\n"
              "          \"d\": 1000\n"
              "        },\n"
              "        \"trade_type\": \"orderbook\"\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"self\": {\n"
              "            \"href\": \"\"\n"
              "          },\n"
              "          \"base\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\"\n"
              "          },\n"
              "          \"counter\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\"\n"
              "          },\n"
              "          \"operation\": {\n"
              "            \"href\": \"https://horizon.stellar.org/operations/3697472920621057\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"3697472920621057-1\",\n"
              "        \"paging_token\": \"3697472920621057-1\",\n"
              "        \"ledger_close_time\": \"2015-11-18T03:47:47Z\",\n"
              "        \"offer_id\": \"4\",\n"
              "        \"base_account\": \"GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\",\n"
              "        \"base_amount\": \"10.0000000\",\n"
              "        \"base_asset_type\": \"native\",\n"
              "        \"counter_account\": \"GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\",\n"
              "        \"counter_amount\": \"2.6800000\",\n"
              "        \"counter_asset_type\": \"credit_alphanum4\",\n"
              "        \"counter_asset_code\": \"JPY\",\n"
              "        \"counter_asset_issuer\": \"GBVAOIACNSB7OVUXJYC5UE2D4YK2F7A24T7EE5YOMN4CE6GCHUTOUQXM\",\n"
              "        \"base_is_seller\": true,\n"
              "        \"price\": {\n"
              "          \"n\": 67,\n"
              "          \"d\": 250\n"
              "        },\n"
              "        \"trade_type\": \"orderbook\"\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"self\": {\n"
              "            \"href\": \"\"\n"
              "          },\n"
              "          \"base\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\"\n"
              "          },\n"
              "          \"counter\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\"\n"
              "          },\n"
              "          \"operation\": {\n"
              "            \"href\": \"https://horizon.stellar.org/operations/3697472920621057\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"3697472920621057-2\",\n"
              "        \"paging_token\": \"3697472920621057-2\",\n"
              "        \"ledger_close_time\": \"2015-11-18T03:47:47Z\",\n"
              "        \"offer_id\": \"8\",\n"
              "        \"base_account\": \"GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\",\n"
              "        \"base_amount\": \"20.0000000\",\n"
              "        \"base_asset_type\": \"native\",\n"
              "        \"counter_account\": \"GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\",\n"
              "        \"counter_amount\": \"5.3600000\",\n"
              "        \"counter_asset_type\": \"credit_alphanum4\",\n"
              "        \"counter_asset_code\": \"JPY\",\n"
              "        \"counter_asset_issuer\": \"GBVAOIACNSB7OVUXJYC5UE2D4YK2F7A24T7EE5YOMN4CE6GCHUTOUQXM\",\n"
              "        \"base_is_seller\": true,\n"
              "        \"price\": {\n"
              "          \"n\": 67,\n"
              "          \"d\": 250\n"
              "        },\n"
              "        \"trade_type\": \"orderbook\"\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"self\": {\n"
              "            \"href\": \"\"\n"
              "          },\n"
              "          \"base\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\"\n"
              "          },\n"
              "          \"counter\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\"\n"
              "          },\n"
              "          \"operation\": {\n"
              "            \"href\": \"https://horizon.stellar.org/operations/3712329212497921\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"3712329212497921-0\",\n"
              "        \"paging_token\": \"3712329212497921-0\",\n"
              "        \"ledger_close_time\": \"2015-11-18T07:26:21Z\",\n"
              "        \"offer_id\": \"36\",\n"
              "        \"base_account\": \"GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\",\n"
              "        \"base_amount\": \"5.0000000\",\n"
              "        \"base_asset_type\": \"native\",\n"
              "        \"counter_account\": \"GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\",\n"
              "        \"counter_amount\": \"1.2000192\",\n"
              "        \"counter_asset_type\": \"credit_alphanum4\",\n"
              "        \"counter_asset_code\": \"JPY\",\n"
              "        \"counter_asset_issuer\": \"GBVAOIACNSB7OVUXJYC5UE2D4YK2F7A24T7EE5YOMN4CE6GCHUTOUQXM\",\n"
              "        \"base_is_seller\": false,\n"
              "        \"trade_type\": \"orderbook\",\n"
              "        \"price\": {\n"
              "          \"n\": \"120001920000000\",\n"
              "          \"d\": \"500000000000000\"\n"
              "        }\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"self\": {\n"
              "            \"href\": \"\"\n"
              "          },\n"
              "          \"base\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\"\n"
              "          },\n"
              "          \"counter\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\"\n"
              "          },\n"
              "          \"operation\": {\n"
              "            \"href\": \"https://horizon.stellar.org/operations/3716237632737281\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"3716237632737281-0\",\n"
              "        \"paging_token\": \"3716237632737281-0\",\n"
              "        \"ledger_close_time\": \"2015-11-18T08:27:26Z\",\n"
              "        \"offer_id\": \"37\",\n"
              "        \"base_account\": \"GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\",\n"
              "        \"base_amount\": \"10.0000000\",\n"
              "        \"base_asset_type\": \"native\",\n"
              "        \"counter_account\": \"GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\",\n"
              "        \"counter_amount\": \"2.4500000\",\n"
              "        \"counter_asset_type\": \"credit_alphanum4\",\n"
              "        \"counter_asset_code\": \"JPY\",\n"
              "        \"counter_asset_issuer\": \"GBVAOIACNSB7OVUXJYC5UE2D4YK2F7A24T7EE5YOMN4CE6GCHUTOUQXM\",\n"
              "        \"base_is_seller\": true,\n"
              "        \"price\": {\n"
              "          \"n\": 5000,\n"
              "          \"d\": 20833\n"
              "        },\n"
              "        \"trade_type\": \"orderbook\"\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"self\": {\n"
              "            \"href\": \"\"\n"
              "          },\n"
              "          \"base\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\"\n"
              "          },\n"
              "          \"counter\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\"\n"
              "          },\n"
              "          \"operation\": {\n"
              "            \"href\": \"https://horizon.stellar.org/operations/3716302057246721\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"3716302057246721-0\",\n"
              "        \"paging_token\": \"3716302057246721-0\",\n"
              "        \"ledger_close_time\": \"2015-11-18T08:28:40Z\",\n"
              "        \"offer_id\": \"35\",\n"
              "        \"base_account\": \"GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\",\n"
              "        \"base_amount\": \"10.0000000\",\n"
              "        \"base_asset_type\": \"native\",\n"
              "        \"counter_account\": \"GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\",\n"
              "        \"counter_amount\": \"2.5000000\",\n"
              "        \"counter_asset_type\": \"credit_alphanum4\",\n"
              "        \"counter_asset_code\": \"JPY\",\n"
              "        \"counter_asset_issuer\": \"GBVAOIACNSB7OVUXJYC5UE2D4YK2F7A24T7EE5YOMN4CE6GCHUTOUQXM\",\n"
              "        \"base_is_seller\": true,\n"
              "        \"price\": {\n"
              "          \"n\": 49,\n"
              "          \"d\": 200\n"
              "        },\n"
              "        \"trade_type\": \"orderbook\"\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"self\": {\n"
              "            \"href\": \"\"\n"
              "          },\n"
              "          \"base\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\"\n"
              "          },\n"
              "          \"counter\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\"\n"
              "          },\n"
              "          \"operation\": {\n"
              "            \"href\": \"https://horizon.stellar.org/operations/3716302057246721\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"3716302057246721-1\",\n"
              "        \"paging_token\": \"3716302057246721-1\",\n"
              "        \"ledger_close_time\": \"2015-11-18T08:28:40Z\",\n"
              "        \"offer_id\": \"34\",\n"
              "        \"base_account\": \"GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\",\n"
              "        \"base_amount\": \"10.0000000\",\n"
              "        \"base_asset_type\": \"native\",\n"
              "        \"counter_account\": \"GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\",\n"
              "        \"counter_amount\": \"3.0000000\",\n"
              "        \"counter_asset_type\": \"credit_alphanum4\",\n"
              "        \"counter_asset_code\": \"JPY\",\n"
              "        \"counter_asset_issuer\": \"GBVAOIACNSB7OVUXJYC5UE2D4YK2F7A24T7EE5YOMN4CE6GCHUTOUQXM\",\n"
              "        \"base_is_seller\": true,\n"
              "        \"price\": {\n"
              "          \"n\": 1,\n"
              "          \"d\": 4\n"
              "        },\n"
              "        \"trade_type\": \"orderbook\"\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"self\": {\n"
              "            \"href\": \"\"\n"
              "          },\n"
              "          \"base\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\"\n"
              "          },\n"
              "          \"counter\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\"\n"
              "          },\n"
              "          \"operation\": {\n"
              "            \"href\": \"https://horizon.stellar.org/operations/3725961438695425\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"3725961438695425-0\",\n"
              "        \"paging_token\": \"3725961438695425-0\",\n"
              "        \"ledger_close_time\": \"2015-11-18T11:09:30Z\",\n"
              "        \"offer_id\": \"47\",\n"
              "        \"base_account\": \"GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\",\n"
              "        \"base_amount\": \"5.0000000\",\n"
              "        \"base_asset_type\": \"native\",\n"
              "        \"counter_account\": \"GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\",\n"
              "        \"counter_amount\": \"1.0000000\",\n"
              "        \"counter_asset_type\": \"credit_alphanum4\",\n"
              "        \"counter_asset_code\": \"JPY\",\n"
              "        \"counter_asset_issuer\": \"GBVAOIACNSB7OVUXJYC5UE2D4YK2F7A24T7EE5YOMN4CE6GCHUTOUQXM\",\n"
              "        \"base_is_seller\": false,\n"
              "        \"price\": {\n"
              "          \"n\": 1,\n"
              "          \"d\": 5\n"
              "        },\n"
              "        \"trade_type\": \"orderbook\"\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"self\": {\n"
              "            \"href\": \"\"\n"
              "          },\n"
              "          \"base\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\"\n"
              "          },\n"
              "          \"counter\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\"\n"
              "          },\n"
              "          \"operation\": {\n"
              "            \"href\": \"https://horizon.stellar.org/operations/3748080520269825\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"3748080520269825-0\",\n"
              "        \"paging_token\": \"3748080520269825-0\",\n"
              "        \"ledger_close_time\": \"2015-11-18T16:56:09Z\",\n"
              "        \"offer_id\": \"53\",\n"
              "        \"base_account\": \"GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\",\n"
              "        \"base_amount\": \"10.0000000\",\n"
              "        \"base_asset_type\": \"native\",\n"
              "        \"counter_account\": \"GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\",\n"
              "        \"counter_amount\": \"3.0000000\",\n"
              "        \"counter_asset_type\": \"credit_alphanum4\",\n"
              "        \"counter_asset_code\": \"JPY\",\n"
              "        \"counter_asset_issuer\": \"GBVAOIACNSB7OVUXJYC5UE2D4YK2F7A24T7EE5YOMN4CE6GCHUTOUQXM\",\n"
              "        \"base_is_seller\": true,\n"
              "        \"price\": {\n"
              "          \"n\": 3,\n"
              "          \"d\": 10\n"
              "        },\n"
              "        \"trade_type\": \"orderbook\"\n"
              "      },\n"
              "      {\n"
              "        \"_links\": {\n"
              "          \"self\": {\n"
              "            \"href\": \"\"\n"
              "          },\n"
              "          \"base\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\"\n"
              "          },\n"
              "          \"counter\": {\n"
              "            \"href\": \"https://horizon.stellar.org/accounts/GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\"\n"
              "          },\n"
              "          \"operation\": {\n"
              "            \"href\": \"https://horizon.stellar.org/operations/3748308153536513\"\n"
              "          }\n"
              "        },\n"
              "        \"id\": \"3748308153536513-0\",\n"
              "        \"paging_token\": \"3748308153536513-0\",\n"
              "        \"ledger_close_time\": \"2015-11-18T16:59:37Z\",\n"
              "        \"offer_id\": \"59\",\n"
              "        \"base_account\": \"GAVH5JM5OKXGMQDS7YPRJ4MQCPXJUGH26LYQPQJ4SOMOJ4SXY472ZM7G\",\n"
              "        \"base_amount\": \"30.0000000\",\n"
              "        \"base_asset_type\": \"native\",\n"
              "        \"counter_account\": \"GBB4JST32UWKOLGYYSCEYBHBCOFL2TGBHDVOMZP462ET4ZRD4ULA7S2L\",\n"
              "        \"counter_amount\": \"9.0000000\",\n"
              "        \"counter_asset_type\": \"credit_alphanum4\",\n"
              "        \"counter_asset_code\": \"JPY\",\n"
              "        \"counter_asset_issuer\": \"GBVAOIACNSB7OVUXJYC5UE2D4YK2F7A24T7EE5YOMN4CE6GCHUTOUQXM\",\n"
              "        \"base_is_seller\": true,\n"
              "        \"price\": {\n"
              "          \"n\": 3,\n"
              "          \"d\": 10\n"
              "        },\n"
              "        \"trade_type\": \"orderbook\"\n"
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
     * A trade's price terms, amounts and paging token, exactly, when the page
     * is read. A price term that is not a whole number (text or a fraction)
     * or a zero denominator was kept, to fail or come out as 0 later.
     */
    void testTradeIsReadExactly()
    {
        typedef Page<TradeResponse> Trades;
        const QByteArray real = realPubnetTradesPage();
        const QString r = QStringLiteral("_embedded.records[0].");
        QVERIFY(refusalOf<Trades>(real, r + "price.n", QString("1.5")).startsWith(r + "price.n: "));
        QVERIFY(refusalOf<Trades>(real, r + "price.n", QString("abc")).startsWith(r + "price.n: "));
        QVERIFY(refusalOf<Trades>(real, r + "price.d", QString("0")).startsWith(r + "price.d: "));
        QVERIFY(refusalOf<Trades>(real, r + "price.d", QJsonValue(QJsonValue::Undefined)).startsWith(r + "price.d: "));
        QVERIFY(refusalOf<Trades>(real, r + "price", QString("4.6451682")).startsWith(r + "price"));
        QVERIFY(refusalOf<Trades>(real, r + "base_amount", 20).startsWith(r + "base_amount: "));
        QVERIFY(refusalOf<Trades>(real, r + "counter_amount", QString("92.90336330000")).startsWith(r + "counter_amount: "));
        QVERIFY(refusalOf<Trades>(real, r + "base_is_seller", QString("false")).startsWith(r + "base_is_seller: "));
        QVERIFY(refusalOf<Trades>(real, r + "paging_token", QJsonValue(QJsonValue::Undefined)).startsWith(r + "paging_token: "));
        QVERIFY(refusalOf<Trades>(real, "_embedded.records[1].liquidity_pool_fee_bp", QString("30bp"))
                    .startsWith("_embedded.records[1].liquidity_pool_fee_bp: "));
    }

    /**
     * Real pubnet trades, one against an offer and one against a liquidity
     * pool (whose price terms and synthetic offer id are past int32 and
     * 2^53), load with their exact values.
     */
    void testRealHorizonTrades()
    {
        Page<TradeResponse> trades;
        trades.loadFromJson(realPubnetTradesPage());
        QCOMPARE(trades.size(), 2);
        QCOMPARE(trades.get(0).getBaseOfferId(), QString("4889418201723064321"));
        QCOMPARE(trades.get(0).getPriceN(), Q_INT64_C(620696229));
        QCOMPARE(trades.get(0).getPriceD(), Q_INT64_C(133621907));
        QCOMPARE(trades.get(0).getCounterAmount(), QString("92.9033633"));
        QCOMPARE(trades.get(1).getTradeType(), QString("liquidity_pool"));
        QCOMPARE(trades.get(1).getLiquidityPoolFeeBp(), 30);
        QCOMPARE(trades.get(1).getPriceN(), Q_INT64_C(721021434299));
        QCOMPARE(trades.get(1).getPriceD(), Q_INT64_C(216356));
        QCOMPARE(trades.get(1).getBaseLiquidityPoolId(),
                 QString("e53308623b66655499125403bf0a91f434047d0fc029324b13a93cdc40438f27"));
    }

    /**
     * Every member Horizon always sends in a trade is required: missing or
     * null, its id, time and type read empty, its assets' types empty, its
     * price absent (getPriceN() 0) and its links empty. What only one kind
     * of trade has (an offer's ids and accounts, a pool's id and fee) stays
     * optional.
     */
    void testMembersHorizonAlwaysSendsAreRequired()
    {
        typedef Page<TradeResponse> Trades;
        const QByteArray real = realPubnetTradesPage();
        QVERIFY(refusalOf<Trades>(real, "_embedded.records[0].base_amount", QString("20.0000000")).isEmpty());
        for (const QString& r : { QStringLiteral("_embedded.records[0]."), QStringLiteral("_embedded.records[1].") }) {
            for (const char* key : { "id", "paging_token", "ledger_close_time", "trade_type", "base_amount",
                                     "base_asset_type", "counter_amount", "counter_asset_type", "base_is_seller",
                                     "price", "_links", "_links.base", "_links.counter", "_links.operation",
                                     "_links.operation.href" }) {
                QVERIFY2(refusalOf<Trades>(real, r + key, QJsonValue(QJsonValue::Undefined)).startsWith(r + key + ": "),
                         qPrintable(r + key));
                QVERIFY2(refusalOf<Trades>(real, r + key, QJsonValue()).startsWith(r + key + ": "), qPrintable(r + key));
            }
        }
        for (const char* key : { "_embedded.records[0].base_offer_id", "_embedded.records[0].base_account",
                                 "_embedded.records[1].base_liquidity_pool_id",
                                 "_embedded.records[1].liquidity_pool_fee_bp" })
            QVERIFY2(refusalOf<Trades>(real, key, QJsonValue(QJsonValue::Undefined)).isEmpty(), key);
    }
private:
    /** Two real pubnet trades, one against an offer and one against a liquidity pool. */
    static QByteArray realPubnetTradesPage()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon.stellar.org/accounts/GD754P2RK354PJITVUQX32TLXTV4H3LS34FDBRRTHWYUSCQ4F36GTADV\"},\"counter\":{\"href\":\"https://horizon.stellar.org/accounts/GBGP2AAFN5SMGJ2BZNWKHSICIDC33T4WNGBGR2TP5CPFO6O3B7RVWR45\"},\"operation\":{\"href\":\"https://horizon.stellar.org/operations/277732183295676417\"}},\"id\":\"277732183295676417-0\",\"paging_token\":\"277732183295676417-0\",\"ledger_close_time\":\"2026-09-28T15:16:36Z\",\"trade_type\":\"orderbook\",\"base_offer_id\":\"4889418201723064321\",\"base_account\":\"GD754P2RK354PJITVUQX32TLXTV4H3LS34FDBRRTHWYUSCQ4F36GTADV\",\"base_amount\":\"20.0000000\",\"base_asset_type\":\"credit_alphanum4\",\"base_asset_code\":\"USDC\",\"base_asset_issuer\":\"GA5ZSEJYB37JRC5AVCIA5MOP4RHTM335X2KGX3IHOJAPP5RE34K4KZVN\",\"counter_offer_id\":\"1859390056\",\"counter_account\":\"GBGP2AAFN5SMGJ2BZNWKHSICIDC33T4WNGBGR2TP5CPFO6O3B7RVWR45\",\"counter_amount\":\"92.9033633\",\"counter_asset_type\":\"native\",\"base_is_seller\":false,\"price\":{\"n\":\"620696229\",\"d\":\"133621907\"}},{\"_links\":{\"self\":{\"href\":\"\"},\"base\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/e53308623b66655499125403bf0a91f434047d0fc029324b13a93cdc40438f27\"},\"counter\":{\"href\":\"https://horizon.stellar.org/accounts/GB3LKXDV7MKIJ7YTCFBVGPMGH3CTXMR4DESXS2XUM6FH32HR6NCYOXCC\"},\"operation\":{\"href\":\"https://horizon.stellar.org/operations/277732183295737857\"}},\"id\":\"277732183295737857-3\",\"paging_token\":\"277732183295737857-3\",\"ledger_close_time\":\"2026-09-28T15:16:36Z\",\"trade_type\":\"liquidity_pool\",\"liquidity_pool_fee_bp\":30,\"base_liquidity_pool_id\":\"e53308623b66655499125403bf0a91f434047d0fc029324b13a93cdc40438f27\",\"base_amount\":\"0.0216356\",\"base_asset_type\":\"native\",\"counter_offer_id\":\"4889418201723125761\",\"counter_account\":\"G"
            "B3LKXDV7MKIJ7YTCFBVGPMGH3CTXMR4DESXS2XUM6FH32HR6NCYOXCC\",\"counter_amount\":\"72102.1434299\",\"counter_asset_type\":\"credit_alphanum12\",\"counter_asset_code\":\"Cleanshave\",\"counter_asset_issuer\":\"GCTZJCIPAJQ5OTNL5MJGA6ETMG7A3GOKYYZUO6JJPWUZCJSSUYNVAISG\",\"base_is_seller\":true,\"price\":{\"n\":\"721021434299\",\"d\":\"216356\"}}]},\"_links\":{\"self\":{\"href\":\"https://horizon.example/x?cursor=&limit=2&order=asc\"},\"next\":{\"href\":\"https://horizon.example/x?cursor=2&limit=2&order=asc\"},\"prev\":{\"href\":\"https://horizon.example/x?cursor=1&limit=2&order=desc\"}}}");
    }

private slots:
};





ADD_TEST(TradesPageDeserializerTest)
#endif // TRADESPAGEDESERIALIZERTEST_H
