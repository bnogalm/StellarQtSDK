#ifndef LEDGERDESERIALIZERTEST_H
#define LEDGERDESERIALIZERTEST_H
#include <QObject>
#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/ledgerresponse.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <limits>

class LedgerDeserializerTest: public QObject
{
    Q_OBJECT
    const QByteArray json = "{\n"
            "  \"_links\": {\n"
            "    \"effects\": {\n"
            "      \"href\": \"/ledgers/898826/effects{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"operations\": {\n"
            "      \"href\": \"/ledgers/898826/operations{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    },\n"
            "    \"self\": {\n"
            "      \"href\": \"/ledgers/898826\"\n"
            "    },\n"
            "    \"transactions\": {\n"
            "      \"href\": \"/ledgers/898826/transactions{?cursor,limit,order}\",\n"
            "      \"templated\": true\n"
            "    }\n"
            "  },\n"
            "  \"id\": \"686bb246db89b099cd3963a4633eb5e4315d89dfd3c00594c80b41a483847bfa\",\n"
            "  \"paging_token\": \"3860428274794496\",\n"
            "  \"hash\": \"686bb246db89b099cd3963a4633eb5e4315d89dfd3c00594c80b41a483847bfa\",\n"
            "  \"prev_hash\": \"50c8695eb32171a19858413e397cc50b504ceacc819010bdf8ff873aff7858d7\",\n"
            "  \"sequence\": 898826,\n"
            "  \"transaction_count\": 5,\n"
            "  \"successful_transaction_count\": 3,\n"
            "  \"failed_transaction_count\": 2,\n"
            "  \"operation_count\": 10,\n"
            "  \"closed_at\": \"2015-11-19T21:35:59Z\",\n"
            "  \"total_coins\": \"101343867604.8975480\",\n"
            "  \"fee_pool\": \"1908.2248818\",\n"
            "  \"base_fee\": 100,\n"
            "  \"base_reserve\": \"10.0000000\",\n"
            "  \"max_tx_set_size\": 50,\n"
            "  \"protocol_version\": 10,\n"
            "  \"header_xdr\": \"AAAACvkxR60p1bwRO3PdsRy48pkWVtuyF08xyvB3jU7u437r9tK/G7DoXa+J8N5ptWhpHGrc/"
            "a+5k9Ak3kHnZSAaiPgAAAAAW4aHyQAAAAAAAAAAyz4xx/YIt7Z9PLeCHj/ZrFGcDaNSJpQX+jpL1IX3uqSCiFrU4CFidqxjmmJzXNWN1rY4cBwliHN21hicu/"
            "JyawCkgzIOdhy8pVTcbQA+gekx1NpNAAAA2QAAAAAACsLiAAAAZABMS0AAAAAykmy96ckoDVk3UDBm7B4n8oC6+cdCxGDnWu7tO6aU34xipth5GRNy+I5Y8m+E/"
            "bHlElCDX2J8y6kuKr7yU6xvyjMb9nwaii7EHN74adNyyUuselmxQcKfEVf/tj3j5l9yOMqgE05NDyU0+LycHY47062IWxvG0o2yDaaxp2Z72pwAAAAA\"\n"
"}";
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserialize() {
        LedgerResponse ledger;
        ledger.loadFromJson(json);
        QCOMPARE(ledger.getHash(), QString("686bb246db89b099cd3963a4633eb5e4315d89dfd3c00594c80b41a483847bfa"));
        QCOMPARE(ledger.getPagingToken(), QString("3860428274794496"));
        QCOMPARE(ledger.getPrevHash(), QString("50c8695eb32171a19858413e397cc50b504ceacc819010bdf8ff873aff7858d7"));
        QCOMPARE(ledger.getSequence(), 898826);
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
        QCOMPARE(ledger.getTransactionCount(), 5);
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
        QCOMPARE(ledger.getSuccessfulTransactionCount(), 3);
        QCOMPARE(ledger.getFailedTransactionCount(), 2);
        QCOMPARE(ledger.getOperationCount(), 10);
        QCOMPARE(ledger.getClosedAt(), QString("2015-11-19T21:35:59Z"));
        QCOMPARE(ledger.getTotalCoins(), QString("101343867604.8975480"));
        QCOMPARE(ledger.getFeePool(), QString("1908.2248818"));
        QCOMPARE(ledger.getBaseFee(), 100);
        QCOMPARE(ledger.getBaseReserve(), QString("10.0000000"));
        QCOMPARE(ledger.getMaxTxSetSize(), 50);
        QCOMPARE(ledger.getProtocolVersion(), 10);
        QCOMPARE(ledger.getHeaderXdr(), QString("AAAACvkxR60p1bwRO3PdsRy48pkWVtuyF08xyvB3jU7u437r9tK/G7DoXa+J8N5ptWhpHGrc/"
          "a+5k9Ak3kHnZSAaiPgAAAAAW4aHyQAAAAAAAAAAyz4xx/YIt7Z9PLeCHj/ZrFGcDaNSJpQX+jpL1IX3uqSCiFrU4CFidqxjmmJzXNWN1rY4cBwliHN21hicu/"
          "JyawCkgzIOdhy8pVTcbQA+gekx1NpNAAAA2QAAAAAACsLiAAAAZABMS0AAAAAykmy96ckoDVk3UDBm7B4n8oC6+cdCxGDnWu7tO6aU34xipth5GRNy+I5Y8m+E/"
          "bHlElCDX2J8y6kuKr7yU6xvyjMb9nwaii7EHN74adNyyUuselmxQcKfEVf/tj3j5l9yOMqgE05NDyU0+LycHY47062IWxvG0o2yDaaxp2Z72pwAAAAA"));
        QCOMPARE(ledger.getLinks().getEffects().getHref(), QString("/ledgers/898826/effects{?cursor,limit,order}"));
        QCOMPARE(ledger.getLinks().getOperations().getHref(), QString("/ledgers/898826/operations{?cursor,limit,order}"));
        QCOMPARE(ledger.getLinks().getSelf().getHref(), QString("/ledgers/898826"));
        QCOMPARE(ledger.getLinks().getTransactions().getHref(), QString("/ledgers/898826/transactions{?cursor,limit,order}"));
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
     * The base fee and reserve fees and minimum balances are computed from,
     * exactly: a fraction or text that is not a number used to read as 0,
     * and a ledger with neither name for them loaded with both at 0.
     */
    void testLedgerIsReadExactly()
    {
        QByteArray current = realPubnetLedger();
        QVERIFY(refusalOf<LedgerResponse>(current, "base_fee_in_stroops", 100.5).startsWith("base_fee_in_stroops: "));
        QVERIFY(refusalOf<LedgerResponse>(current, "base_reserve_in_stroops", QString("5000000x")).startsWith("base_reserve_in_stroops: "));
        QVERIFY(refusalOf<LedgerResponse>(current, "base_reserve_in_stroops", QJsonValue(QJsonValue::Undefined)).startsWith("base_reserve_in_stroops: "));
        QVERIFY(refusalOf<LedgerResponse>(current, "sequence", QString("64664562.0")).startsWith("sequence: "));
        QVERIFY(refusalOf<LedgerResponse>(current, "paging_token", QJsonValue(QJsonValue::Undefined)).startsWith("paging_token: "));
        QVERIFY(refusalOf<LedgerResponse>(current, "total_coins", 105443902087.0).startsWith("total_coins: "));
        QVERIFY(refusalOf<LedgerResponse>(current, "max_tx_set_size", 2147483648.0).startsWith("max_tx_set_size: "));
        QVERIFY(refusalOf<LedgerResponse>(json, "base_fee", QString("one hundred")).startsWith("base_fee: "));
    }

    /** A real pubnet ledger loads with its exact fee, reserve and coins. */
    void testRealHorizonLedger()
    {
        LedgerResponse ledger;
        ledger.loadFromJson(realPubnetLedger());
        QCOMPARE(ledger.getSequence(), Q_INT64_C(64664562));
        QCOMPARE(ledger.getBaseFeeInStroops(), QString("100"));
        QCOMPARE(ledger.getBaseReserveInStroops(), QString("5000000"));
        QCOMPARE(ledger.getMaxTxSetSize(), 1000);
        QCOMPARE(ledger.getTotalCoins(), QString("105443902087.3472865"));
        QCOMPARE(ledger.getFeePool(), QString("10712508.7844940"));
        QCOMPARE(ledger.getPagingToken(), QString("277732179000164352"));
    }

    /**
     * Every member Horizon always sends in a ledger is required. Missing or
     * null, the coins in existence and the fee pool read "", the counts,
     * the set size and the protocol 0, the header empty. The transaction
     * count, the base fee and the base reserve go by either name (the
     * older ones before Horizon 0.15 and 0.18); the counts Horizon may
     * leave null stay optional.
     */
    void testMembersHorizonAlwaysSendsAreRequired()
    {
        const QByteArray real = realPubnetLedger();
        QVERIFY(refusalOf<LedgerResponse>(real, "sequence", 64664562).isEmpty());
        for (const char* key : { "sequence", "hash", "paging_token", "closed_at", "total_coins", "fee_pool",
                                 "operation_count", "max_tx_set_size", "protocol_version", "header_xdr",
                                 "successful_transaction_count", "base_fee_in_stroops", "base_reserve_in_stroops",
                                 "_links", "_links.self", "_links.transactions", "_links.operations",
                                 "_links.effects", "_links.self.href" }) {
            QVERIFY2(refusalOf<LedgerResponse>(real, key, QJsonValue(QJsonValue::Undefined))
                         .startsWith(QString(key) + ": "), key);
            QVERIFY2(refusalOf<LedgerResponse>(real, key, QJsonValue()).startsWith(QString(key) + ": "), key);
        }
        for (const char* key : { "prev_hash", "failed_transaction_count", "tx_set_operation_count" })
            QVERIFY2(refusalOf<LedgerResponse>(real, key, QJsonValue()).isEmpty(), key);
        // The older name for the transaction count.
        QJsonObject old = QJsonDocument::fromJson(real).object();
        old.remove("successful_transaction_count");
        old.insert("transaction_count", 419);
        LedgerResponse ledger;
        ledger.loadFromJson(QJsonDocument(old).toJson());
        QCOMPARE(ledger.getSequence(), Q_INT64_C(64664562));
    }

    /**
     * A ledger's base reserve by its older name is an amount: text that was
     * not one was kept as it came, for the caller to read as 0.
     */
    void testOldBaseReserveIsAnAmount()
    {
        QVERIFY(refusalOf<LedgerResponse>(json, "base_reserve", QString("10 XLM")).startsWith("base_reserve: "));
        QVERIFY(refusalOf<LedgerResponse>(json, "base_reserve", QString("10.00000000")).startsWith("base_reserve: "));
        QVERIFY(refusalOf<LedgerResponse>(json, "base_reserve", QString("10.0000000")).isEmpty());
    }
private:
    /** A real pubnet ledger. */
    static QByteArray realPubnetLedger()
    {
        return QByteArray(
            "{\"_links\":{\"self\":{\"href\":\"https://horizon.stellar.org/ledgers/64664562\"},\"transactions\":{\"href\":\"https://horizon.stellar.org/ledgers/64664562/transactions{?cursor,limit,order}\",\"templated\":true},\"operations\":{\"href\":\"https://horizon.stellar.org/ledgers/64664562/operations{?cursor,limit,order}\",\"templated\":true},\"payments\":{\"href\":\"https://horizon.stellar.org/ledgers/64664562/payments{?cursor,limit,order}\",\"templated\":true},\"effects\":{\"href\":\"https://horizon.stellar.org/ledgers/64664562/effects{?cursor,limit,order}\",\"templated\":true}},\"id\":\"602928a5ae80ba606cd5b3d25c245b08ae9c397d04b34f4e835d659b366f9617\",\"paging_token\":\"277732179000164352\",\"hash\":\"602928a5ae80ba606cd5b3d25c245b08ae9c397d04b34f4e835d659b366f9617\",\"prev_hash\":\"2d60a099865c9f3e85c5f29e8205a49b558b08eb21de8a81419ef28ce2ca1e2f\",\"sequence\":64664562,\"successful_transaction_count\":324,\"failed_transaction_count\":95,\"operation_count\":836,\"tx_set_operation_count\":948,\"closed_at\":\"2026-09-28T15:16:31Z\",\"total_coins\":\"105443902087.3472865\",\"fee_pool\":\"10712508.7844940\",\"base_fee_in_stroops\":100,\"base_reserve_in_stroops\":5000000,\"max_tx_set_size\":1000,\"protocol_version\":28,\"header_xdr\":\"AAAAHC1goJmGXJ8+hcXynoIFpJtViwjrId6KgUGe8oziyh4vrORndSI9o+r4tgLuugR3YuO0X81MB0FDn/5hvj/sCkgAAAAAarqEzwAAAAAAAAABAAAAAAFdGFUq2t7rTo0wWu9k/6rxa0T+pf6CHmBj2vO56O1XAAAAQIH4u02+cfiw1aym4iPGp6srv7K6sTGBOHbRARHHPURYLWQ0BcMQ9aU6RbwSObEDfWaNd/NGB8p0WTvMJNo4qgcOX3klx43W9SB7B77E4Kn2bVUfHDvpl5AQnresJLNiwxL0fnpXHS/38VX/ojT4DCFa6AYAfFvRR4E4r21eaUxhA9qz8g6iHrPseVthAABhbgDOukwAAAEWAAAAAG7UCmMAAABkAExLQAAAA+iRkeJ0J2PW6u5GcGox2qSnl1phOCOl3SiHxBXOXrY2bFue2ouo3qnCl05iUc6lkHuJdwlJvQ015tLOadJS0MJlhbcdQLTsRS49myQKW7KgDBinzZiIxRXzNorLuZwAKPgqqw4jbU4IoqobhBged0Y0ebssSuWTHMczfE6dlLjXHgAAAAA=\"}");
    }

private slots:
};


ADD_TEST(LedgerDeserializerTest)
#endif // LEDGERDESERIALIZERTEST_H
