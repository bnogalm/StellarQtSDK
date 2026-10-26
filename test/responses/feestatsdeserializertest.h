#ifndef OPERATIONFEESTATSDESERIALIZERTEST_H
#define OPERATIONFEESTATSDESERIALIZERTEST_H
#include <QObject>
#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/feestatsresponse.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>

class FeeStatsDeserializerTest: public QObject
{
    Q_OBJECT
    const QByteArray json = "{\n"
                            "  \"last_ledger\": \"22606298\",\n"
                            "  \"last_ledger_base_fee\": \"100\",\n"
                            "  \"ledger_capacity_usage\": \"0.97\",\n"
                            "  \"max_fee\": {\n"
                            "    \"min\": \"130\",\n"
                            "    \"max\": \"8000\",\n"
                            "    \"mode\": \"250\",\n"
                            "    \"p10\": \"150\",\n"
                            "    \"p20\": \"200\",\n"
                            "    \"p30\": \"300\",\n"
                            "    \"p40\": \"400\",\n"
                            "    \"p50\": \"500\",\n"
                            "    \"p60\": \"1000\",\n"
                            "    \"p70\": \"2000\",\n"
                            "    \"p80\": \"3000\",\n"
                            "    \"p90\": \"4000\",\n"
                            "    \"p95\": \"5000\",\n"
                            "    \"p99\": \"8000\"\n"
                            "  },\n"
                            "  \"fee_charged\": {\n"
                            "    \"min\": \"100\",\n"
                            "    \"max\": \"102\",\n"
                            "    \"mode\": \"101\",\n"
                            "    \"p10\": \"103\",\n"
                            "    \"p20\": \"104\",\n"
                            "    \"p30\": \"105\",\n"
                            "    \"p40\": \"106\",\n"
                            "    \"p50\": \"107\",\n"
                            "    \"p60\": \"108\",\n"
                            "    \"p70\": \"109\",\n"
                            "    \"p80\": \"110\",\n"
                            "    \"p90\": \"111\",\n"
                            "    \"p95\": \"112\",\n"
                            "    \"p99\": \"113\"\n"
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
        FeeStatsResponse stats;
        stats.loadFromJson(json);
        QCOMPARE(stats.getLastLedger(),22606298);
        QCOMPARE(stats.getLastLedgerBaseFee(),100);
        QCOMPARE(stats.getLedgerCapacityUsage(),0.97f);
        FeeDistribution maxFee = stats.getMaxFee();
        QCOMPARE(maxFee.getMin(),130);
        QCOMPARE(maxFee.getMax(),8000);
        QCOMPARE(maxFee.getMode(),250);
        QCOMPARE(maxFee.getP10(),150);
        QCOMPARE(maxFee.getP20(),200);
        QCOMPARE(maxFee.getP30(),300);
        QCOMPARE(maxFee.getP40(),400);
        QCOMPARE(maxFee.getP50(),500);
        QCOMPARE(maxFee.getP60(),1000);
        QCOMPARE(maxFee.getP70(),2000);
        QCOMPARE(maxFee.getP80(),3000);
        QCOMPARE(maxFee.getP90(),4000);
        QCOMPARE(maxFee.getP95(),5000);
        QCOMPARE(maxFee.getP99(),8000);

        FeeDistribution feeCharged = stats.getFeeCharged();
        QCOMPARE(feeCharged.getMin(),100);
        QCOMPARE(feeCharged.getMax(),102);
        QCOMPARE(feeCharged.getMode(),101);
        QCOMPARE(feeCharged.getP10(),103);
        QCOMPARE(feeCharged.getP20(),104);
        QCOMPARE(feeCharged.getP30(),105);
        QCOMPARE(feeCharged.getP40(),106);
        QCOMPARE(feeCharged.getP50(),107);
        QCOMPARE(feeCharged.getP60(),108);
        QCOMPARE(feeCharged.getP70(),109);
        QCOMPARE(feeCharged.getP80(),110);
        QCOMPARE(feeCharged.getP90(),111);
        QCOMPARE(feeCharged.getP95(),112);
        QCOMPARE(feeCharged.getP99(),113);
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
     * The fees a transaction is priced with, exactly: a percentile that is
     * not a number, a fraction, or a number past int64 used to read as 0 (a
     * fee of 0 is refused by the network), and a missing one as 0 too.
     */
    void testFeeStatsAreReadExactly()
    {
        QVERIFY(refusalOf<FeeStatsResponse>(json, "fee_charged.p50", QString("abc")).startsWith("fee_charged.p50: "));
        QVERIFY(refusalOf<FeeStatsResponse>(json, "fee_charged.p99", 113.5).startsWith("fee_charged.p99: "));
        QVERIFY(refusalOf<FeeStatsResponse>(json, "max_fee.max", QString("9223372036854775808")).startsWith("max_fee.max: "));
        QVERIFY(refusalOf<FeeStatsResponse>(json, "max_fee.p95", QJsonValue(QJsonValue::Undefined)).startsWith("max_fee.p95: "));
        QVERIFY(refusalOf<FeeStatsResponse>(json, "max_fee.mode", QJsonValue()).startsWith("max_fee.mode: "));
        QVERIFY(refusalOf<FeeStatsResponse>(json, "last_ledger_base_fee", QString("100 stroops")).startsWith("last_ledger_base_fee: "));
        QVERIFY(refusalOf<FeeStatsResponse>(json, "last_ledger_base_fee", QJsonValue(QJsonValue::Undefined)).startsWith("last_ledger_base_fee: "));
        QVERIFY(refusalOf<FeeStatsResponse>(json, "ledger_capacity_usage", QString("high")).startsWith("ledger_capacity_usage: "));
        QVERIFY(refusalOf<FeeStatsResponse>(json, "fee_charged", QString("100")).startsWith("fee_charged: "));
    }

    /** Real fee stats, pubnet and testnet, load with their exact values. */
    void testRealHorizonFeeStats()
    {
        FeeStatsResponse pubnet;
        pubnet.loadFromJson(realPubnetFeeStats());
        QCOMPARE(pubnet.getLastLedger(), Q_INT64_C(64664562));
        QCOMPARE(pubnet.getLastLedgerBaseFee(), Q_INT64_C(100));
        QCOMPARE(pubnet.getLedgerCapacityUsage(), 0.82f);
        QCOMPARE(pubnet.getMaxFee().getMax(), Q_INT64_C(301106380));
        QCOMPARE(pubnet.getFeeCharged().getP99(), Q_INT64_C(317989));
        FeeStatsResponse testnet;
        testnet.loadFromJson(realTestnetFeeStats());
        QCOMPARE(testnet.getFeeCharged().getMax(), Q_INT64_C(8140878));
        QCOMPARE(testnet.getMaxFee().getP90(), Q_INT64_C(10000000));
    }

    /**
     * How full the last ledgers were is always sent too: missing or null it
     * read 0, as if there were room for any fee.
     */
    void testLedgerCapacityUsageIsRequired()
    {
        const QByteArray real = realPubnetFeeStats();
        QVERIFY(refusalOf<FeeStatsResponse>(real, "ledger_capacity_usage", QString("0.82")).isEmpty());
        QVERIFY(refusalOf<FeeStatsResponse>(real, "ledger_capacity_usage", QJsonValue(QJsonValue::Undefined))
                    .startsWith("ledger_capacity_usage: "));
        QVERIFY(refusalOf<FeeStatsResponse>(real, "ledger_capacity_usage", QJsonValue())
                    .startsWith("ledger_capacity_usage: "));
    }
private:
    /** Real pubnet fee stats. */
    static QByteArray realPubnetFeeStats()
    {
        return QByteArray(
            "{\"last_ledger\":\"64664562\",\"last_ledger_base_fee\":\"100\",\"ledger_capacity_usage\":\"0.82\",\"fee_charged\":{\"max\":\"398442\",\"min\":\"100\",\"mode\":\"100\",\"p10\":\"100\",\"p20\":\"100\",\"p30\":\"100\",\"p40\":\"100\",\"p50\":\"100\",\"p60\":\"9236\",\"p70\":\"9236\",\"p80\":\"9236\",\"p90\":\"9236\",\"p95\":\"17443\",\"p99\":\"317989\"},\"max_fee\":{\"max\":\"301106380\",\"min\":\"100\",\"mode\":\"141895\",\"p10\":\"125\",\"p20\":\"1000\",\"p30\":\"20001\",\"p40\":\"22688\",\"p50\":\"141895\",\"p60\":\"141895\",\"p70\":\"141895\",\"p80\":\"141895\",\"p90\":\"1000000\",\"p95\":\"3000000\",\"p99\":\"16824685\"}}");
    }

    /** Real testnet fee stats. */
    static QByteArray realTestnetFeeStats()
    {
        return QByteArray(
            "{\"last_ledger\":\"4917001\",\"last_ledger_base_fee\":\"100\",\"ledger_capacity_usage\":\"0.12\",\"fee_charged\":{\"max\":\"8140878\",\"min\":\"100\",\"mode\":\"100\",\"p10\":\"100\",\"p20\":\"100\",\"p30\":\"100\",\"p40\":\"100\",\"p50\":\"100\",\"p60\":\"9694\",\"p70\":\"17190\",\"p80\":\"85564\",\"p90\":\"152588\",\"p95\":\"1690276\",\"p99\":\"8140878\"},\"max_fee\":{\"max\":\"20000000\",\"min\":\"100\",\"mode\":\"1000000\",\"p10\":\"21442\",\"p20\":\"36576\",\"p30\":\"203994\",\"p40\":\"1000000\",\"p50\":\"1000000\",\"p60\":\"1000000\",\"p70\":\"1000000\",\"p80\":\"1103554\",\"p90\":\"10000000\",\"p95\":\"10026826\",\"p99\":\"20000000\"}}");
    }

private slots:
};

ADD_TEST(FeeStatsDeserializerTest)
#endif // OPERATIONFEESTATSDESERIALIZERTEST_H
