#ifndef LIQUIDITYPOOLRESPONSETEST_H
#define LIQUIDITYPOOLRESPONSETEST_H
#include <QObject>
#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/liquiditypoolresponse.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "../../src/responses/page.h"

class LiquidityPoolResponseTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    /** Deserialize a `/liquidity_pools/{id}` Horizon response. */
    void testDeserialize()
    {
        QByteArray json = "{"
            "\"id\":\"abc1234\","
            "\"paging_token\":\"abc1234\","
            "\"fee_bp\":30,"
            "\"type\":\"constant_product\","
            "\"total_trustlines\":\"42\","
            "\"total_shares\":\"100.0000000\","
            "\"reserves\":["
                "{\"asset\":\"native\",\"amount\":\"50.0000000\"},"
                "{\"asset\":\"USDC:GA5\",\"amount\":\"50.0000000\"}"
            "],"
            "\"last_modified_ledger\":12345,"
            "\"last_modified_time\":\"2021-11-08T01:46:38Z\","
            "\"_links\":{"
                "\"self\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/abc1234\"},"
                "\"operations\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/abc1234/operations\"},"
                "\"transactions\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/abc1234/transactions\"}"
            "}}";
        LiquidityPoolResponse r;
        r.loadFromJson(json);

        QCOMPARE(r.getId(), QString("abc1234"));
        QCOMPARE(r.getPagingToken(), QString("abc1234"));
        QCOMPARE(r.getFeeBp(), (qint32)30);
        QCOMPARE(r.getType(), QString("constant_product"));
        QCOMPARE(r.getTotalTrustlines(), (qint64)42);
        QCOMPARE(r.getTotalShares(), QString("100.0000000"));
        QCOMPARE(r.getReserves().size(), 2);
        QCOMPARE(r.getReserves()[0].getAsset(), QString("native"));
        QCOMPARE(r.getReserves()[0].getAmount(), QString("50.0000000"));
        QCOMPARE(r.getReserves()[1].getAsset(), QString("USDC:GA5"));
        QCOMPARE(r.getLastModifiedLedger(), (qint64)12345);
        QCOMPARE(r.getLastModifiedTime(), QString("2021-11-08T01:46:38Z"));
        QCOMPARE(r.getLinks().getSelf().getHref(),
                 QString("https://horizon.stellar.org/liquidity_pools/abc1234"));
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
     * A pool's fee, shares and reserves, exactly: a fee that is not a whole
     * number, a trustline count that is not one, or an amount given as a
     * number used to read as 0 or empty.
     */
    void testLiquidityPoolIsReadExactly()
    {
        typedef Page<LiquidityPoolResponse> Pools;
        const QByteArray real = realPubnetLiquidityPools();
        const QString r = QStringLiteral("_embedded.records[0].");
        QVERIFY(refusalOf<Pools>(real, r + "fee_bp", QString("30bp")).startsWith(r + "fee_bp: "));
        QVERIFY(refusalOf<Pools>(real, r + "fee_bp", 30.5).startsWith(r + "fee_bp: "));
        QVERIFY(refusalOf<Pools>(real, r + "fee_bp", QJsonValue(QJsonValue::Undefined)).startsWith(r + "fee_bp: "));
        QVERIFY(refusalOf<Pools>(real, r + "total_trustlines", QString("1x")).startsWith(r + "total_trustlines: "));
        QVERIFY(refusalOf<Pools>(real, r + "total_shares", 100).startsWith(r + "total_shares: "));
        QVERIFY(refusalOf<Pools>(real, r + "reserves[1].amount", 50).startsWith(r + "reserves[1].amount: "));
        QVERIFY(refusalOf<Pools>(real, r + "reserves[1].amount", QJsonValue(QJsonValue::Undefined)).startsWith(r + "reserves[1].amount: "));
        QVERIFY(refusalOf<Pools>(real, r + "reserves[0]", QString("native")).startsWith(r + "reserves[0]: "));
        QVERIFY(refusalOf<Pools>(real, r + "reserves", QJsonObject()).startsWith(r + "reserves: "));
        QVERIFY(refusalOf<Pools>(real, r + "paging_token", QJsonValue(QJsonValue::Undefined)).startsWith(r + "paging_token: "));
    }

    /** Real pubnet pools load with their exact fee, trustlines, shares and reserves. */
    void testRealHorizonLiquidityPools()
    {
        const QJsonArray records = QJsonDocument::fromJson(realPubnetLiquidityPools()).object()
                .value("_embedded").toObject().value("records").toArray();
        Page<LiquidityPoolResponse> pools;
        pools.loadFromJson(realPubnetLiquidityPools());
        QCOMPARE(pools.size(), 2);
        for (int i = 0; i < 2; ++i) {
            const QJsonObject pool = records.at(i).toObject();
            QCOMPARE(pools.get(i).getId(), pool.value("id").toString());
            QCOMPARE(pools.get(i).getFeeBp(), pool.value("fee_bp").toInt());
            QCOMPARE(QString::number(pools.get(i).getTotalTrustlines()), pool.value("total_trustlines").toString());
            QCOMPARE(pools.get(i).getTotalShares(), pool.value("total_shares").toString());
            QCOMPARE(pools.get(i).getReserves()[1].getAmount(),
                     pool.value("reserves").toArray().at(1).toObject().value("amount").toString());
        }
    }

    /**
     * Every member Horizon always sends in a pool is required: missing or
     * null, its type read empty, the ledger it was last changed in 0 and
     * its links empty. The time may be null.
     */
    void testMembersHorizonAlwaysSendsAreRequired()
    {
        typedef Page<LiquidityPoolResponse> Pools;
        const QByteArray real = realPubnetLiquidityPools();
        const QString r = QStringLiteral("_embedded.records[0].");
        QVERIFY(refusalOf<Pools>(real, r + "fee_bp", 30).isEmpty());
        for (const char* key : { "id", "paging_token", "fee_bp", "type", "total_trustlines", "total_shares",
                                 "reserves", "last_modified_ledger", "_links", "_links.self", "_links.operations",
                                 "_links.transactions", "_links.self.href", "reserves[0].asset",
                                 "reserves[0].amount" }) {
            QVERIFY2(refusalOf<Pools>(real, r + key, QJsonValue(QJsonValue::Undefined)).startsWith(r + key + ": "), key);
            QVERIFY2(refusalOf<Pools>(real, r + key, QJsonValue()).startsWith(r + key + ": "), key);
        }
        QVERIFY(refusalOf<Pools>(real, r + "last_modified_time", QJsonValue()).isEmpty());
    }
private:
    /** Two real pubnet liquidity pools. */
    static QByteArray realPubnetLiquidityPools()
    {
        return QByteArray(
            "{\"_embedded\":{\"records\":[{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/ffffe478b19c0d27c19fecdf36b6b60fefc563e1806109a58b86b4138abd085a\"},\"transactions\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/ffffe478b19c0d27c19fecdf36b6b60fefc563e1806109a58b86b4138abd085a/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/ffffe478b19c0d27c19fecdf36b6b60fefc563e1806109a58b86b4138abd085a/operations{?cursor,limit,order}\",\"templated\":true}},\"id\":\"ffffe478b19c0d27c19fecdf36b6b60fefc563e1806109a58b86b4138abd085a\",\"paging_token\":\"ffffe478b19c0d27c19fecdf36b6b60fefc563e1806109a58b86b4138abd085a\",\"fee_bp\":30,\"type\":\"constant_product\",\"total_trustlines\":\"1\",\"total_shares\":\"90988.3130213\",\"reserves\":[{\"asset\":\"LIBRE:GAYCCWKECNGDRHYU3UTREBD2XLC3CUQN6FV22TKM4WCQER3IWR7TF5CY\",\"amount\":\"3547.4369224\"},{\"asset\":\"NFIUM:GD76B36XOOH432PICDIVEOD4RU7TEFJ5DZVA2C5MW5LKTSCARK45V3DB\",\"amount\":\"2350232.9798320\"}],\"last_modified_ledger\":64662747,\"last_modified_time\":\"2026-09-28T12:45:16Z\"},{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/ffff7fe3743b3c63b12e80f22aed5e8799aa986e25fe155a19277b660d547e71\"},\"transactions\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/ffff7fe3743b3c63b12e80f22aed5e8799aa986e25fe155a19277b660d547e71/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon.stellar.org/liquidity_pools/ffff7fe3743b3c63b12e80f22aed5e8799aa986e25fe155a19277b660d547e71/operations{?cursor,limit,order}\",\"templated\":true}},\"id\":\"ffff7fe3743b3c63b12e80f22aed5e8799aa986e25fe155a19277b660d547e71\",\"paging_token\":\"ffff7fe3743b3c63b12e80f22aed5e8799aa986e25fe155a19277b660d547e71\",\"fee_bp\":30,\"type\":\"constant_product\",\"total_trustlines\""
            ":\"1\",\"total_shares\":\"15653.2744178\",\"reserves\":[{\"asset\":\"native\",\"amount\":\"1.6374287\"},{\"asset\":\"AAVE:GDTGN34MKUEO4BTJET4SX6TCRQ72FS2TPDNFO6JDXXVL4PNBSREHD2D5\",\"amount\":\"157892432.1726389\"}],\"last_modified_ledger\":60027687,\"last_modified_time\":\"2025-11-26T15:09:09Z\"}]},\"_links\":{\"self\":{\"href\":\"https://horizon.example/x?cursor=&limit=2&order=asc\"},\"next\":{\"href\":\"https://horizon.example/x?cursor=2&limit=2&order=asc\"},\"prev\":{\"href\":\"https://horizon.example/x?cursor=1&limit=2&order=desc\"}}}");
    }

private slots:
};

ADD_TEST(LiquidityPoolResponseTest)
#endif // LIQUIDITYPOOLRESPONSETEST_H
