#ifndef TRADEAGGREGATIONSREQUESTBUILDERTEST_H
#define TRADEAGGREGATIONSREQUESTBUILDERTEST_H
#include <QObject>

#include <QtTest>
#include "../testcollector.h"
#include "../../src/server.h"
#include "../../src/keypair.h"
class TradeAggregationsRequestBuilderTest: public QObject
{
    Q_OBJECT
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testTradeAggregations() {
        Server server("https://horizon-testnet.stellar.org");
        QUrl uri = server.tradeAggregations(
                    new AssetTypeNative(),
                    Asset::createNonNativeAsset("BTC", KeyPair::fromAccountId("GATEMHCCKCY67ZUCKTROYN24ZYT5GK4EQZ65JJLDHKHRUZI3EUEKMTCH")),
                    1512689100000L,
                    1512775500000L,
                    86400000L,
                    3600000L
                    ).limit(200).order(RequestBuilder::Order::ASC).buildUri();

        QCOMPARE(uri.toString(),QString("https://horizon-testnet.stellar.org/trade_aggregations?"
                                        "base_asset_type=native&"
                                        "counter_asset_type=credit_alphanum4&"
                                        "counter_asset_code=BTC&"
                                        "counter_asset_issuer=GATEMHCCKCY67ZUCKTROYN24ZYT5GK4EQZ65JJLDHKHRUZI3EUEKMTCH&"
                                        "start_time=1512689100000&"
                                        "end_time=1512775500000&"
                                        "resolution=86400000&"
                                        "offset=3600000&"
                                        "limit=200&"
                                        "order=asc"));

    }

    /**
     * Resolution and offset are checked before the request, as js and py
     * check them: the pairs js's tradeAggregation and py's
     * trade_aggregations refuse are refused, those they take are built.
     * This test used to build 300000 with an offset of 3600 (ms), which
     * both refuse and Horizon answers 400. A negative offset is refused
     * here only.
     */
    void testResolutionAndOffsetAreChecked() {
        Server server("https://horizon-testnet.stellar.org");
        AssetTypeNative native;
        auto builds = [&](qint64 resolution, qint64 offset) {
            try {
                server.tradeAggregations(&native, &native, 0, 1, resolution, offset);
                return true;
            } catch (const std::runtime_error&) {
                return false;
            }
        };
        QVERIFY(builds(60000, 0));
        QVERIFY(builds(86400000, 3600000));
        QVERIFY(builds(3600000, 3600000));
        QVERIFY(builds(604800000, 82800000));
        QVERIFY(!builds(300000, 3600));
        QVERIFY(!builds(123, 0));
        QVERIFY(!builds(300000, 3600000));
        QVERIFY(!builds(604800000, 86400000));
        QVERIFY(!builds(86400000, -3600000));
    }

    /**
     * /trade_aggregations for an alphanum4 and an alphanum12 pair, as
     * js-stellar-sdk 17.1.0 (the call builder's url after checkFilter(),
     * what call() requests) and py-stellar-sdk 14.1.0 (the builder's url and
     * params as requests encodes them) build these URLs, the same in both.
     */
    void testUrlsMatchJsAndPy() {
        Server server("https://horizon-testnet.stellar.org");
        QScopedPointer<Asset> xlm(new AssetTypeNative());
        QScopedPointer<Asset> usd(Asset::createNonNativeAsset("USD", QString("GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG")));
        QScopedPointer<Asset> banana(Asset::createNonNativeAsset("BANANA", QString("GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB")));
        QCOMPARE(server.tradeAggregations(usd.data(), xlm.data(), Q_INT64_C(1512689100000), Q_INT64_C(1512775500000), 300000, 0)
                     .limit(200).order(RequestBuilder::Order::DESC).buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/trade_aggregations?base_asset_type=credit_alphanum4&base_asset_code=USD&base_asset_issuer=GDVDKQFP665JAO7A2LSHNLQIUNYNAAIGJ6FYJVMG4DT3YJQQJSRBLQDG&counter_asset_type=native&start_time=1512689100000&end_time=1512775500000&resolution=300000&offset=0&limit=200&order=desc"));
        QCOMPARE(server.tradeAggregations(xlm.data(), banana.data(), 0, Q_INT64_C(1512775500000), 3600000, 3600000)
                     .buildUri().toString(),
                 QString("https://horizon-testnet.stellar.org/trade_aggregations?base_asset_type=native&counter_asset_type=credit_alphanum12&counter_asset_code=BANANA&counter_asset_issuer=GD42RQNXTRIW6YR3E2HXV5T2AI27LBRHOERV2JIYNFMXOBA234SWLQQB&start_time=0&end_time=1512775500000&resolution=3600000&offset=3600000"));
    }
};

ADD_TEST(TradeAggregationsRequestBuilderTest)
#endif // TRADEAGGREGATIONSREQUESTBUILDERTEST_H
