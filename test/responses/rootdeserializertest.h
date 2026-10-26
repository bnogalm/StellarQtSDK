#ifndef ROOTDESERIALIZERTEST_H
#define ROOTDESERIALIZERTEST_H

#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/rootresponse.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>
class RootDeserializerTest: public QObject
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
        QByteArray json = "{\n"
                "  \"_links\": {\n"
                "    \"account\": {\n"
                "      \"href\": \"https://horizon.stellar.org/accounts/{account_id}\",\n"
                "      \"templated\": true\n"
                "    },\n"
                "    \"account_transactions\": {\n"
                "      \"href\": \"https://horizon.stellar.org/accounts/{account_id}/transactions{?cursor,limit,order}\",\n"
                "      \"templated\": true\n"
                "    },\n"
                "    \"assets\": {\n"
                "      \"href\": \"https://horizon.stellar.org/assets{?asset_code,asset_issuer,cursor,limit,order}\",\n"
                "      \"templated\": true\n"
                "    },\n"
                "    \"friendbot\": {\n"
                "      \"href\": \"https://horizon.stellar.org/friendbot{?addr}\",\n"
                "      \"templated\": true\n"
                "    },\n"
                "    \"metrics\": {\n"
                "      \"href\": \"https://horizon.stellar.org/metrics\"\n"
                "    },\n"
                "    \"order_book\": {\n"
                "      \"href\": \"https://horizon.stellar.org/order_book{?selling_asset_type,selling_asset_code,selling_issuer,buying_asset_type,buying_asset_code,buying_issuer,limit}\",\n"
                "      \"templated\": true\n"
                "    },\n"
                "    \"self\": {\n"
                "      \"href\": \"https://horizon.stellar.org/\"\n"
                "    },\n"
                "    \"transaction\": {\n"
                "      \"href\": \"https://horizon.stellar.org/transactions/{hash}\",\n"
                "      \"templated\": true\n"
                "    },\n"
                "    \"transactions\": {\n"
                "      \"href\": \"https://horizon.stellar.org/transactions{?cursor,limit,order}\",\n"
                "      \"templated\": true\n"
                "    }\n"
                "  },\n"
                "  \"horizon_version\": \"snapshot-c5fe0ff\",\n"
                "  \"core_version\": \"stellar-core 9.2.0 (7561c1d53366ec79b908de533726269e08474f77)\",\n"
                "  \"history_latest_ledger\": 18369116,\n"
                "  \"history_elder_ledger\": 1,\n"
                "  \"core_latest_ledger\": 18369117,\n"
                "  \"network_passphrase\": \"Public Global Stellar Network ; September 2015\",\n"
                "  \"protocol_version\": 9,\n"
                "  \"current_protocol_version\": 10\n,"
                "  \"core_supported_protocol_version\": 11,\n"
                "  \"ingest_latest_ledger\": 64701295,\n"
                "  \"history_latest_ledger_closed_at\": \"2026-09-30T18:17:37Z\",\n"
                "  \"supported_protocol_version\": 29\n"
                "}";
            RootResponse root;
            root.loadFromJson(json);

            QCOMPARE(root.getHorizonVersion(), QString("snapshot-c5fe0ff"));
            QCOMPARE(root.getStellarCoreVersion(), QString("stellar-core 9.2.0 (7561c1d53366ec79b908de533726269e08474f77)"));
            QCOMPARE(root.getHistoryLatestLedger(), 18369116);
            QCOMPARE(root.getHistoryElderLedger(), 1);
            QCOMPARE(root.getCoreLatestLedger(), 18369117);
            QCOMPARE(root.getNetworkPassphrase(), QString("Public Global Stellar Network ; September 2015"));
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
            QCOMPARE(root.getProtocolVersion(), 9);
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
            QCOMPARE(root.getCurrentProtocolVersion(), 10);
            QCOMPARE(root.getCoreSupportedProtocolVersion(), 11);
            QCOMPARE(root.getIngestLatestLedger(), Q_INT64_C(64701295));
            QCOMPARE(root.getHistoryLatestLedgerClosedAt(), QString("2026-09-30T18:17:37Z"));
            QCOMPARE(root.getSupportedProtocolVersion(), 29);
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
     * The network passphrase transactions are signed for: missing or null
     * it read as empty, and given as anything but text it was converted.
     * The protocol versions are whole numbers.
     */
    void testRootIsReadExactly()
    {
        const QByteArray real = realTestnetRoot();
        QVERIFY(refusalOf<RootResponse>(real, "network_passphrase", QJsonValue(QJsonValue::Undefined)).startsWith("network_passphrase: "));
        QVERIFY(refusalOf<RootResponse>(real, "network_passphrase", QJsonValue()).startsWith("network_passphrase: "));
        QVERIFY(refusalOf<RootResponse>(real, "network_passphrase", 2015).startsWith("network_passphrase: "));
        QVERIFY(refusalOf<RootResponse>(real, "current_protocol_version", QString("28a")).startsWith("current_protocol_version: "));
        QVERIFY(refusalOf<RootResponse>(real, "history_latest_ledger", 4917001.5).startsWith("history_latest_ledger: "));
    }

    /** The real testnet root loads. */
    void testRealHorizonRoot()
    {
        RootResponse root;
        root.loadFromJson(realTestnetRoot());
        QCOMPARE(root.getNetworkPassphrase(), QString("Test SDF Network ; September 2015"));
        QCOMPARE(root.getCurrentProtocolVersion(), 28);
        QCOMPARE(root.getCoreSupportedProtocolVersion(), 29);
        QCOMPARE(root.getHistoryLatestLedger(), 4917001);
    }

    /**
     * Every member Horizon always sends in its root is required: missing or
     * null, the versions and ledgers read empty or 0, as if Horizon had
     * ingested nothing and knew no protocol.
     */
    void testMembersHorizonAlwaysSendsAreRequired()
    {
        const QByteArray real = realTestnetRoot();
        QVERIFY(refusalOf<RootResponse>(real, "history_elder_ledger", 128).isEmpty());
        for (const char* key : { "horizon_version", "core_version", "history_latest_ledger", "history_elder_ledger",
                                 "core_latest_ledger", "network_passphrase", "current_protocol_version",
                                 "core_supported_protocol_version" }) {
            QVERIFY2(refusalOf<RootResponse>(real, key, QJsonValue(QJsonValue::Undefined)).startsWith(QString(key) + ": "), key);
            QVERIFY2(refusalOf<RootResponse>(real, key, QJsonValue()).startsWith(QString(key) + ": "), key);
        }
        // protocol_version was replaced by current_protocol_version.
        QVERIFY(refusalOf<RootResponse>(real, "protocol_version", QJsonValue(QJsonValue::Undefined)).isEmpty());
    }
private:
    /** Real testnet root (links dropped). */
    static QByteArray realTestnetRoot()
    {
        return QByteArray(
            "{\"horizon_version\":\"29.0.0-c9e48b645f0777f19135109d3de5c46160cea737\",\"core_version\":\"stellar-core 29.0.0 (4eb83337380a29ad0907f4e32196ce97b8dc7649)\",\"ingest_latest_ledger\":4917001,\"history_latest_ledger\":4917001,\"history_latest_ledger_closed_at\":\"2026-09-28T15:16:32Z\",\"history_elder_ledger\":128,\"core_latest_ledger\":4917001,\"network_passphrase\":\"Test SDF Network ; September 2015\",\"current_protocol_version\":28,\"supported_protocol_version\":29,\"core_supported_protocol_version\":29}");
    }

private slots:
};

ADD_TEST(RootDeserializerTest)
#endif // ROOTDESERIALIZERTEST_H
