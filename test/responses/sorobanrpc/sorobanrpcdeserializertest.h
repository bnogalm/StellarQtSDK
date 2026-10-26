#ifndef SOROBANRPCDESERIALIZERTEST_H
#define SOROBANRPCDESERIALIZERTEST_H

#include <QObject>
#include <QtTest>
#include "../../testcollector.h"

#include "../../../src/sorobanserver.h"
#include "../../../src/exception/sorobanrpcexception.h"
#include "readonlyresponsestest.h"

/**
 * JSON-RPC 2.0 envelope plumbing for the Soroban RPC client (Iter 1):
 * request building, success/error parse, and getHealth round-trip via
 * the parse layer (no HTTP).
 */
class SorobanRpcDeserializerTest: public QObject, private RpcAnswerChecks
{
    Q_OBJECT

    // Real getHealth answers, saved as the public RPCs gave them on
    // 2026-09-28 (stellar-rpc 29.0.0, protocol 28).
    static QByteArray realGetHealthTestnet() { return QByteArray("{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"status\":\"healthy\",\"latestLedger\":4919927,\"latestLedgerCloseTime\":\"1790623222\",\"oldestLedger\":4798968,\"oldestLedgerCloseTime\":\"1790018427\",\"ledgerRetentionWindow\":120960}}"); }
    static QByteArray realGetHealthMainnet() { return QByteArray("{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"status\":\"healthy\",\"latestLedger\":64667569,\"latestLedgerCloseTime\":\"1790623626\",\"oldestLedger\":64546610,\"oldestLedgerCloseTime\":\"1790018828\",\"ledgerRetentionWindow\":120960}}"); }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testBuildRpcRequestMinimal()
    {
        SorobanServer s(QUrl(QStringLiteral("http://rpc.test")));
        QByteArray bytes = s.buildRpcRequest("getHealth", QJsonValue());
        QJsonDocument doc = QJsonDocument::fromJson(bytes);
        QVERIFY(doc.isObject());
        QJsonObject o = doc.object();
        QCOMPARE(o.value("jsonrpc").toString(), QString("2.0"));
        QCOMPARE(o.value("method").toString(), QString("getHealth"));
        QVERIFY(!o.contains("params"));   // omitted when null/undefined
        QVERIFY(o.contains("id"));
        // Per-instance counter starts at 1.
        QCOMPARE(o.value("id").toString(), QString("1"));
    }

    void testBuildRpcRequestIncludesParams()
    {
        SorobanServer s(QUrl(QStringLiteral("http://rpc.test")));
        QJsonObject params;
        params.insert("hash", QStringLiteral("deadbeef"));
        QByteArray bytes = s.buildRpcRequest("getTransaction", params);
        QJsonObject o = QJsonDocument::fromJson(bytes).object();
        QCOMPARE(o.value("method").toString(), QString("getTransaction"));
        QCOMPARE(o.value("params").toObject().value("hash").toString(),
                 QString("deadbeef"));
    }

    void testParseRpcResultSuccess()
    {
        QByteArray json = R"({"jsonrpc":"2.0","id":"7","result":{"status":"healthy"}})";
        QJsonValue result = SorobanServer::parseRpcResult(json);
        QCOMPARE(result.toObject().value("status").toString(), QString("healthy"));
    }

    void testParseRpcResultErrorThrows()
    {
        QByteArray json = R"({"jsonrpc":"2.0","id":"7","error":{"code":-32601,"message":"Method not found"}})";
        bool threw = false;
        try {
            SorobanServer::parseRpcResult(json);
        } catch (const qstellar::exception::SorobanRpcException& e) {
            threw = true;
            QCOMPARE(e.getCode(), -32601);
            QCOMPARE(e.getRpcMessage(), QString("Method not found"));
        }
        QVERIFY(threw);
    }

    void testParseRpcResultWithErrorData()
    {
        QByteArray json = R"({"jsonrpc":"2.0","id":"1","error":{"code":-1,"message":"oops","data":{"reason":"x"}}})";
        bool threw = false;
        try {
            SorobanServer::parseRpcResult(json);
        } catch (const qstellar::exception::SorobanRpcException& e) {
            threw = true;
            QVERIFY(e.getData().isValid());
            QCOMPARE(e.getData().toMap().value("reason").toString(), QString("x"));
        }
        QVERIFY(threw);
    }

    void testParseRpcRejectsMissingResultAndError()
    {
        QByteArray json = R"({"jsonrpc":"2.0","id":"1"})";
        bool threw = false;
        try { SorobanServer::parseRpcResult(json); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    void testParseRpcRejectsBadJson()
    {
        QByteArray json = "not json at all";
        bool threw = false;
        try { SorobanServer::parseRpcResult(json); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    void testParseRpcRejectsWrongJsonrpcVersion()
    {
        QByteArray json = R"({"jsonrpc":"1.0","id":"1","result":{}})";
        bool threw = false;
        try { SorobanServer::parseRpcResult(json); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    // ─── getHealth payload deserialization ───────────────────────────

    void testGetHealthFromJsonNumeric()
    {
        QJsonObject result;
        result.insert("status", QStringLiteral("healthy"));
        result.insert("latestLedger", 1000);
        result.insert("oldestLedger", 500);
        result.insert("ledgerRetentionWindow", 600);
        GetHealthResponse r = GetHealthResponse::fromJson(result);
        QCOMPARE(r.getStatus(), QString("healthy"));
        QCOMPARE(r.getLatestLedger(), quint32(1000));
        QCOMPARE(r.getOldestLedger(), quint32(500));
        QCOMPARE(r.getLedgerRetentionWindow(), quint32(600));
    }

    /**
     * getHealth's ledger numbers are uint32s the RPC sends as JSON integers,
     * never as strings. A string used to be accepted and read with toUInt(),
     * so "abc" read as ledger 0 and "4294967296" as 0; this test expected
     * strings to be read, and now expects them refused.
     */
    void testGetHealthRefusesLedgerNumbersSentAsStrings()
    {
        QJsonObject result;
        result.insert("status", QStringLiteral("healthy"));
        result.insert("latestLedger", QStringLiteral("4294967290"));
        result.insert("oldestLedger", QStringLiteral("4294967000"));
        result.insert("ledgerRetentionWindow", QStringLiteral("17280"));
        const QString message = refusal([&] { GetHealthResponse::fromJson(result); });
        QVERIFY2(message.contains(QStringLiteral("latestLedger")), qPrintable(message));

        // The same numbers as JSON integers are read exactly.
        result.insert("latestLedger", qint64(4294967290LL));
        result.insert("oldestLedger", qint64(4294967000LL));
        result.insert("ledgerRetentionWindow", 17280);
        GetHealthResponse r = GetHealthResponse::fromJson(result);
        QCOMPARE(r.getLatestLedger(), quint32(4294967290u));
        QCOMPARE(r.getLedgerRetentionWindow(), quint32(17280));
    }

    /** Real getHealth answers, from testnet and from mainnet, are read as sent. */
    void testReadsRealGetHealthAnswers()
    {
        const GetHealthResponse testnet = GetHealthResponse::fromJson(resultOf(realGetHealthTestnet()));
        QCOMPARE(testnet.getStatus(), QString("healthy"));
        QCOMPARE(testnet.getLatestLedger(), quint32(4919927));
        QCOMPARE(testnet.getOldestLedger(), quint32(4798968));
        QCOMPARE(testnet.getLedgerRetentionWindow(), quint32(120960));
        const GetHealthResponse mainnet = GetHealthResponse::fromJson(resultOf(realGetHealthMainnet()));
        QCOMPARE(mainnet.getStatus(), QString("healthy"));
        QCOMPARE(mainnet.getLatestLedger(), quint32(64667569));
        QCOMPARE(mainnet.getOldestLedger(), quint32(64546610));
    }

    /**
     * Anything else is refused, naming the field. A missing or unknown status
     * used to be kept as it came (an unhealthy RPC answers with an error, so
     * "healthy" is the only status a result carries), and a ledger number that
     * was missing, text, a fraction, negative or past 32 bits read as 0 or
     * cut to 32 bits.
     */
    void testGetHealthRefusesWhatTheRpcDoesNotSend()
    {
        QList<Case> cases = badUint32(QStringLiteral("latestLedger"));
        cases.append(badUint32(QStringLiteral("oldestLedger")));
        cases.append(badUint32(QStringLiteral("ledgerRetentionWindow")));
        for (const QJsonValue& status : { QJsonValue(QStringLiteral("unhealthy")), QJsonValue(QStringLiteral("Healthy")),
                                          QJsonValue(QString()), QJsonValue(true), QJsonValue(QJsonValue::Null), missing() })
            cases.append({ QStringLiteral("status"), status });
        const QString problem = checkRefusals(QStringLiteral("getHealth"), resultOf(realGetHealthTestnet()), cases,
                                              [](const QJsonObject& o) { GetHealthResponse::fromJson(o); });
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
    }

    /**
     * A refusal shows the value refused as it was sent. It used to be written
     * by QJsonDocument, which on Qt 5.15 writes a 64-bit integer through a
     * double: 9007199254740993 was shown as 9007199254740992, and
     * 9223372036854775807 as 9223372036854776000, other numbers than the
     * ones sent (Qt 6 wrote them right).
     */
    void testARefusalShowsTheNumberSent()
    {
        const auto refusalOf = [](const QByteArray& sent) {
            const QJsonObject result = QJsonDocument::fromJson(
                "{\"status\":\"healthy\",\"latestLedger\":" + sent
                + ",\"oldestLedger\":1,\"ledgerRetentionWindow\":1}").object();
            return RpcAnswerChecks::refusal([&] { GetHealthResponse::fromJson(result); });
        };
        const QString prefix = QStringLiteral("Soroban RPC: getHealth answered latestLedger, which is not a whole number from 0 to 4294967295: ");
        for (const char* sent : { "9007199254740993", "9223372036854775807", "-9007199254740993",
                                  "-9223372036854775807", "4294967296", "-1" })
            QCOMPARE(refusalOf(sent), prefix + QString::fromLatin1(sent));
        // Inside a list or an object too.
        for (const char* sent : { "[9007199254740993,-1,[9223372036854775807]]",
                                  "{\"n\":9007199254740993,\"s\":\"x\"}" })
            QCOMPARE(refusalOf(sent), prefix + QString::fromLatin1(sent));
        // Anything else as before: text quoted, a fraction and true as JSON.
        for (const char* sent : { "\"12\"", "1.5", "true", "[\"a\",null,false]" })
            QCOMPARE(refusalOf(sent), prefix + QString::fromLatin1(sent));
        // Cut short past 64 characters, where it is cut.
        QByteArray many = "[9007199254740993";
        for (int i = 0; i < 9; ++i)
            many += ",9007199254740993";
        many += ']';
        QCOMPARE(refusalOf(many), prefix + QString::fromLatin1(many.left(61)) + QStringLiteral("..."));
    }

    void testGetHealthEndToEndViaEnvelope()
    {
        QByteArray json =
            R"({"jsonrpc":"2.0","id":"1","result":{"status":"healthy","latestLedger":42,)"
            R"("oldestLedger":1,"ledgerRetentionWindow":17280}})";
        GetHealthResponse r = GetHealthResponse::fromJson(
            SorobanServer::parseRpcResult(json).toObject());
        QCOMPARE(r.getStatus(), QString("healthy"));
        QCOMPARE(r.getLatestLedger(), quint32(42));
    }
};

ADD_TEST(SorobanRpcDeserializerTest)
#endif // SOROBANRPCDESERIALIZERTEST_H
