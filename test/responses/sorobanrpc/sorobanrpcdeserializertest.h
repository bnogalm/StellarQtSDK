#ifndef SOROBANRPCDESERIALIZERTEST_H
#define SOROBANRPCDESERIALIZERTEST_H

#include <QObject>
#include <QtTest>
#include "../../testcollector.h"

#include "../../../src/sorobanserver.h"
#include "../../../src/exception/sorobanrpcexception.h"

/**
 * JSON-RPC 2.0 envelope plumbing for the Soroban RPC client (Iter 1):
 * request building, success/error parse, and getHealth round-trip via
 * the parse layer (no HTTP).
 */
class SorobanRpcDeserializerTest: public QObject
{
    Q_OBJECT
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

    /** The spec allows numeric fields to be sent as JSON strings to preserve
     *  uint64 precision; our parser accepts both. */
    void testGetHealthFromJsonStringNumbers()
    {
        QJsonObject result;
        result.insert("status", QStringLiteral("healthy"));
        result.insert("latestLedger", QStringLiteral("4294967290"));
        result.insert("oldestLedger", QStringLiteral("4294967000"));
        result.insert("ledgerRetentionWindow", QStringLiteral("17280"));
        GetHealthResponse r = GetHealthResponse::fromJson(result);
        QCOMPARE(r.getLatestLedger(), quint32(4294967290u));
        QCOMPARE(r.getLedgerRetentionWindow(), quint32(17280));
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
