#ifndef READONLYRESPONSESTEST_H
#define READONLYRESPONSESTEST_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QtTest>
#include "../../testcollector.h"

#include "../../../src/responses/sorobanrpc/getnetworkresponse.h"
#include "../../../src/responses/sorobanrpc/getversioninforesponse.h"
#include "../../../src/responses/sorobanrpc/getlatestledgerresponse.h"
#include "../../../src/responses/sorobanrpc/getfeestatsresponse.h"
#include "../../../src/responses/sorobanrpc/gettransactionresponse.h"
#include "../../../src/responses/sorobanrpc/gettransactionsresponse.h"
#include "../../../src/responses/sorobanrpc/getledgerentriesresponse.h"
#include "../../../src/responses/sorobanrpc/getledgersresponse.h"
#include "../../../src/responses/sorobanrpc/geteventsresponse.h"

class SorobanRpcReadOnlyResponsesTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testGetNetwork()
    {
        QJsonObject o;
        o.insert("friendbotUrl", QStringLiteral("https://friendbot.stellar.org"));
        o.insert("passphrase", QStringLiteral("Test SDF Network ; September 2015"));
        o.insert("protocolVersion", 21);
        GetNetworkResponse r = GetNetworkResponse::fromJson(o);
        QCOMPARE(r.getFriendbotUrl(), QString("https://friendbot.stellar.org"));
        QCOMPARE(r.getPassphrase(), QString("Test SDF Network ; September 2015"));
        QCOMPARE(r.getProtocolVersion(), 21);
    }

    void testGetVersionInfo()
    {
        QJsonObject o;
        o.insert("version", QStringLiteral("21.4.0"));
        o.insert("commitHash", QStringLiteral("abc123"));
        o.insert("buildTimestamp", QStringLiteral("2026-01-01T00:00:00Z"));
        o.insert("captiveCoreVersion", QStringLiteral("21.0.0"));
        o.insert("protocolVersion", 21);
        GetVersionInfoResponse r = GetVersionInfoResponse::fromJson(o);
        QCOMPARE(r.getVersion(), QString("21.4.0"));
        QCOMPARE(r.getCommitHash(), QString("abc123"));
        QCOMPARE(r.getProtocolVersion(), 21);
    }

    void testGetLatestLedger()
    {
        QJsonObject o;
        o.insert("id", QStringLiteral("hash"));
        o.insert("protocolVersion", 21);
        o.insert("sequence", 12345);
        GetLatestLedgerResponse r = GetLatestLedgerResponse::fromJson(o);
        QCOMPARE(r.getId(), QString("hash"));
        QCOMPARE(r.getSequence(), quint32(12345));
    }

    void testGetTransactionStatus()
    {
        QJsonObject o;
        o.insert("status", QStringLiteral("SUCCESS"));
        o.insert("txHash", QStringLiteral("deadbeef"));
        GetTransactionResponse r = GetTransactionResponse::fromJson(o);
        QCOMPARE(static_cast<int>(r.getStatus()),
                 static_cast<int>(GetTransactionResponse::Status::SUCCESS));
        QCOMPARE(r.getTxHash(), QString("deadbeef"));
    }

    void testGetTransactionNotFound()
    {
        QJsonObject o;
        o.insert("status", QStringLiteral("NOT_FOUND"));
        GetTransactionResponse r = GetTransactionResponse::fromJson(o);
        QCOMPARE(static_cast<int>(r.getStatus()),
                 static_cast<int>(GetTransactionResponse::Status::NOT_FOUND));
    }

    void testGetTransactionRichFields()
    {
        QByteArray json = "{"
            "  \"status\": \"SUCCESS\","
            "  \"txHash\": \"abc123\","
            "  \"latestLedger\": 100,"
            "  \"applicationOrder\": 1,"
            "  \"feeBump\": true,"
            "  \"envelopeXdr\": \"AAAA\","
            "  \"resultXdr\": \"BBBB\","
            "  \"resultMetaXdr\": \"CCCC\","
            "  \"ledger\": 95,"
            "  \"createdAt\": \"1700000000\","
            "  \"diagnosticEventsXdr\": [ \"DGE1\", \"DGE2\" ],"
            "  \"events\": {"
            "    \"diagnosticEventsXdr\": [ \"EVDIAG1\" ],"
            "    \"transactionEventsXdr\": [ \"TXEV1\", \"TXEV2\" ],"
            "    \"contractEventsXdr\": [ [ \"OP0EV0\", \"OP0EV1\" ], [ \"OP1EV0\" ] ]"
            "  }"
            "}";
        GetTransactionResponse r = GetTransactionResponse::fromJson(parseObj(json));

        QCOMPARE(static_cast<int>(r.getStatus()),
                 static_cast<int>(GetTransactionResponse::Status::SUCCESS));
        QVERIFY(r.getFeeBump());
        QCOMPARE(r.getDiagnosticEventsXdr().size(), 2);
        QCOMPARE(r.getDiagnosticEventsXdr().at(0), QString("DGE1"));
        QCOMPARE(r.getDiagnosticEventsXdr().at(1), QString("DGE2"));

        GetTransactionResponse::Events ev = r.getEvents();
        QCOMPARE(ev.diagnosticEventsXdr.size(), 1);
        QCOMPARE(ev.diagnosticEventsXdr.at(0), QString("EVDIAG1"));
        QCOMPARE(ev.transactionEventsXdr.size(), 2);
        QCOMPARE(ev.transactionEventsXdr.at(1), QString("TXEV2"));
        QCOMPARE(ev.contractEventsXdr.size(), 2);
        QCOMPARE(ev.contractEventsXdr.at(0).size(), 2);
        QCOMPARE(ev.contractEventsXdr.at(0).at(1), QString("OP0EV1"));
        QCOMPARE(ev.contractEventsXdr.at(1).size(), 1);
        QCOMPARE(ev.contractEventsXdr.at(1).at(0), QString("OP1EV0"));
    }

    static QJsonObject parseObj(const QByteArray& json)
    {
        return QJsonDocument::fromJson(json).object();
    }

    void testGetFeeStats()
    {
        QByteArray json =
            "{"
            "\"sorobanInclusionFee\":{\"max\":\"100\",\"min\":\"50\",\"mode\":\"75\","
            "\"p10\":\"55\",\"p20\":\"60\",\"p30\":\"65\",\"p40\":\"70\",\"p50\":\"75\","
            "\"p60\":\"80\",\"p70\":\"85\",\"p80\":\"90\",\"p90\":\"95\",\"p95\":\"98\","
            "\"p99\":\"99\",\"transactionCount\":\"50\",\"ledgerCount\":10},"
            "\"inclusionFee\":{\"max\":\"200\",\"min\":\"100\",\"mode\":\"150\","
            "\"p10\":\"110\",\"p20\":\"120\",\"p30\":\"130\",\"p40\":\"140\",\"p50\":\"150\","
            "\"p60\":\"160\",\"p70\":\"170\",\"p80\":\"180\",\"p90\":\"190\",\"p95\":\"195\","
            "\"p99\":\"199\",\"transactionCount\":\"100\",\"ledgerCount\":10},"
            "\"latestLedger\":12345"
            "}";
        GetFeeStatsResponse r = GetFeeStatsResponse::fromJson(parseObj(json));
        QCOMPARE(r.getLatestLedger(), quint32(12345));
        QCOMPARE(r.getSorobanInclusionFee().max, QString("100"));
        QCOMPARE(r.getSorobanInclusionFee().p50, QString("75"));
        QCOMPARE(r.getInclusionFee().mode, QString("150"));
    }

    void testGetLedgerEntries()
    {
        QByteArray json =
            "{\"entries\":["
            "{\"key\":\"k1\",\"xdr\":\"x1\",\"lastModifiedLedgerSeq\":100,\"liveUntilLedgerSeq\":200},"
            "{\"key\":\"k2\",\"xdr\":\"x2\",\"lastModifiedLedgerSeq\":150}"
            "],\"latestLedger\":300}";
        GetLedgerEntriesResponse r = GetLedgerEntriesResponse::fromJson(parseObj(json));
        QCOMPARE(r.getLatestLedger(), quint32(300));
        QCOMPARE(r.getEntries().size(), 2);
        QCOMPARE(r.getEntries().at(0).key, QString("k1"));
        QCOMPARE(r.getEntries().at(0).liveUntilLedgerSeq, quint32(200));
        QCOMPARE(r.getEntries().at(1).liveUntilLedgerSeq, quint32(0));
    }

    void testGetLedgers()
    {
        QByteArray json =
            "{\"ledgers\":["
            "{\"hash\":\"h1\",\"sequence\":100,\"ledgerCloseTime\":\"t1\","
            "\"headerXdr\":\"h_x\",\"metadataXdr\":\"m_x\"},"
            "{\"hash\":\"h2\",\"sequence\":101,\"ledgerCloseTime\":\"t2\","
            "\"headerXdr\":\"h_x2\",\"metadataXdr\":\"m_x2\"}"
            "],\"latestLedger\":200,\"latestLedgerCloseTime\":\"now\","
            "\"oldestLedger\":1,\"oldestLedgerCloseTime\":\"old\",\"cursor\":\"c1\"}";
        GetLedgersResponse r = GetLedgersResponse::fromJson(parseObj(json));
        QCOMPARE(r.getLedgers().size(), 2);
        QCOMPARE(r.getLedgers().at(0).hash, QString("h1"));
        QCOMPARE(r.getLedgers().at(0).sequence, quint32(100));
        QCOMPARE(r.getCursor(), QString("c1"));
    }

    void testGetTransactions()
    {
        QByteArray json =
            "{\"transactions\":["
            "{\"status\":\"SUCCESS\",\"txHash\":\"a\",\"ledger\":100},"
            "{\"status\":\"FAILED\",\"txHash\":\"b\",\"ledger\":101}"
            "],\"latestLedger\":200,\"cursor\":\"c\"}";
        GetTransactionsResponse r = GetTransactionsResponse::fromJson(parseObj(json));
        QCOMPARE(r.getTransactions().size(), 2);
        QCOMPARE(static_cast<int>(r.getTransactions().at(0).getStatus()),
                 static_cast<int>(GetTransactionResponse::Status::SUCCESS));
        QCOMPARE(static_cast<int>(r.getTransactions().at(1).getStatus()),
                 static_cast<int>(GetTransactionResponse::Status::FAILED));
    }

    void testGetEvents()
    {
        QByteArray json =
            "{\"events\":["
            "{\"type\":\"contract\",\"ledger\":100,\"ledgerClosedAt\":\"2026-01-01\","
            "\"contractId\":\"CABC\",\"id\":\"event-1\",\"pagingToken\":1,"
            "\"operationIndex\":3,\"transactionIndex\":5,"
            "\"inSuccessfulContractCall\":true,\"txHash\":\"tx1\","
            "\"topic\":[\"t1-base64\",\"t2-base64\"],\"value\":\"value-base64\"}"
            "],\"latestLedger\":200,\"oldestLedger\":150,"
            "\"latestLedgerCloseTime\":\"1700000200\",\"oldestLedgerCloseTime\":\"1700000150\","
            "\"cursor\":\"event-1\"}";
        GetEventsResponse r = GetEventsResponse::fromJson(parseObj(json));
        QCOMPARE(r.getEvents().size(), 1);
        const auto& ev = r.getEvents().first();
        QCOMPARE(ev.type, QString("contract"));
        QCOMPARE(ev.contractId, QString("CABC"));
        QCOMPARE(ev.topic.size(), 2);
        QCOMPARE(ev.operationIndex, 3);
        QCOMPARE(ev.transactionIndex, 5);
        QCOMPARE(r.getOldestLedger(), quint32(150));
        QCOMPARE(r.getLatestLedgerCloseTime(), QString("1700000200"));
        QCOMPARE(r.getOldestLedgerCloseTime(), QString("1700000150"));
        QCOMPARE(r.getCursor(), QString("event-1"));
    }
};

ADD_TEST(SorobanRpcReadOnlyResponsesTest)
#endif // READONLYRESPONSESTEST_H
