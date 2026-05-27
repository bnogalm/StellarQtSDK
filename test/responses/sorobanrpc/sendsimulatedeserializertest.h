#ifndef SENDSIMULATEDESERIALIZERTEST_H
#define SENDSIMULATEDESERIALIZERTEST_H

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QtTest>
#include "../../testcollector.h"

#include "../../../src/responses/sorobanrpc/sendtransactionresponse.h"
#include "../../../src/responses/sorobanrpc/simulatetransactionresponse.h"

class SorobanRpcSendSimulateDeserializerTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    // ---- sendTransaction ---------------------------------------------------

    void testSendTransactionPending()
    {
        QJsonObject o;
        o.insert("status", QStringLiteral("PENDING"));
        o.insert("hash", QStringLiteral("d1234"));
        o.insert("latestLedger", 100);
        o.insert("latestLedgerCloseTime", QStringLiteral("2026-05-01T00:00:00Z"));
        SendTransactionResponse r = SendTransactionResponse::fromJson(o);
        QCOMPARE(r.getStatus(), SendTransactionResponse::Status::PENDING);
        QCOMPARE(r.getHash(), QString("d1234"));
        QCOMPARE(r.getLatestLedger(), quint32(100));
    }

    void testSendTransactionDuplicate()
    {
        QJsonObject o;
        o.insert("status", QStringLiteral("DUPLICATE"));
        o.insert("hash", QStringLiteral("h2"));
        o.insert("latestLedger", 200);
        SendTransactionResponse r = SendTransactionResponse::fromJson(o);
        QCOMPARE(r.getStatus(), SendTransactionResponse::Status::DUPLICATE);
        QCOMPARE(r.getHash(), QString("h2"));
    }

    void testSendTransactionTryAgainLater()
    {
        QJsonObject o;
        o.insert("status", QStringLiteral("TRY_AGAIN_LATER"));
        o.insert("hash", QStringLiteral("h3"));
        SendTransactionResponse r = SendTransactionResponse::fromJson(o);
        QCOMPARE(r.getStatus(), SendTransactionResponse::Status::TRY_AGAIN_LATER);
    }

    void testSendTransactionError()
    {
        QJsonObject o;
        o.insert("status", QStringLiteral("ERROR"));
        o.insert("hash", QStringLiteral("h4"));
        o.insert("errorResultXdr", QStringLiteral("AAAA"));
        QJsonArray diag;
        diag.append(QStringLiteral("event1base64"));
        diag.append(QStringLiteral("event2base64"));
        o.insert("diagnosticEventsXdr", diag);
        SendTransactionResponse r = SendTransactionResponse::fromJson(o);
        QCOMPARE(r.getStatus(), SendTransactionResponse::Status::ERROR_);
        QCOMPARE(r.getErrorResultXdr(), QString("AAAA"));
        QCOMPARE(r.getDiagnosticEventsXdr().size(), 2);
        QCOMPARE(r.getDiagnosticEventsXdr().at(0), QString("event1base64"));
    }

    void testSendTransactionLatestLedgerString()
    {
        // RPC servers may emit latestLedger as a string to preserve uint64.
        QJsonObject o;
        o.insert("status", QStringLiteral("PENDING"));
        o.insert("hash", QStringLiteral("h5"));
        o.insert("latestLedger", QStringLiteral("4294967295"));
        SendTransactionResponse r = SendTransactionResponse::fromJson(o);
        QCOMPARE(r.getLatestLedger(), quint32(4294967295U));
    }

    // ---- simulateTransaction ----------------------------------------------

    void testSimulateMinimal()
    {
        QJsonObject o;
        o.insert("latestLedger", 1000);
        o.insert("transactionData", QStringLiteral("txdata-b64"));
        o.insert("minResourceFee", QStringLiteral("12345"));
        SimulateTransactionResponse r = SimulateTransactionResponse::fromJson(o);
        QCOMPARE(r.getLatestLedger(), quint32(1000));
        QCOMPARE(r.getTransactionData(), QString("txdata-b64"));
        QCOMPARE(r.getMinResourceFee(), QString("12345"));
        QVERIFY(!r.isError());
        QCOMPARE(r.getResults().size(), 0);
        QCOMPARE(r.getEvents().size(), 0);
    }

    void testSimulateWithResults()
    {
        QJsonObject o;
        o.insert("latestLedger", 1001);
        o.insert("transactionData", QStringLiteral("td"));
        o.insert("minResourceFee", QStringLiteral("999"));
        QJsonArray events;
        events.append(QStringLiteral("ev-1"));
        events.append(QStringLiteral("ev-2"));
        o.insert("events", events);

        QJsonObject res0;
        res0.insert("xdr", QStringLiteral("scval-b64"));
        QJsonArray auth0;
        auth0.append(QStringLiteral("auth-entry-b64"));
        res0.insert("auth", auth0);
        QJsonArray results;
        results.append(res0);
        o.insert("results", results);

        SimulateTransactionResponse r = SimulateTransactionResponse::fromJson(o);
        QCOMPARE(r.getEvents().size(), 2);
        QCOMPARE(r.getResults().size(), 1);
        QCOMPARE(r.getResults().first().xdr, QString("scval-b64"));
        QCOMPARE(r.getResults().first().auth.size(), 1);
        QCOMPARE(r.getResults().first().auth.first(), QString("auth-entry-b64"));
    }

    void testSimulateStateChanges()
    {
        QJsonObject o;
        o.insert("latestLedger", 1002);
        QJsonArray changes;
        QJsonObject c0;
        c0.insert("type", QStringLiteral("updated"));
        c0.insert("key", QStringLiteral("ledgerkey-b64"));
        c0.insert("before", QStringLiteral("before-b64"));
        c0.insert("after", QStringLiteral("after-b64"));
        changes.append(c0);
        QJsonObject c1;
        c1.insert("type", QStringLiteral("created"));
        c1.insert("key", QStringLiteral("k2"));
        c1.insert("after", QStringLiteral("a2"));
        changes.append(c1);
        o.insert("stateChanges", changes);

        SimulateTransactionResponse r = SimulateTransactionResponse::fromJson(o);
        QCOMPARE(r.getStateChanges().size(), 2);
        QCOMPARE(r.getStateChanges().at(0).type, QString("updated"));
        QCOMPARE(r.getStateChanges().at(0).before, QString("before-b64"));
        QCOMPARE(r.getStateChanges().at(1).type, QString("created"));
        QVERIFY(r.getStateChanges().at(1).before.isEmpty());
    }

    void testSimulateError()
    {
        QJsonObject o;
        o.insert("latestLedger", 1003);
        o.insert("error", QStringLiteral("HostError: out of budget"));
        SimulateTransactionResponse r = SimulateTransactionResponse::fromJson(o);
        QVERIFY(r.isError());
        QCOMPARE(r.getError(), QString("HostError: out of budget"));
    }

    void testSimulateMinResourceFeeAsInt()
    {
        QJsonObject o;
        o.insert("latestLedger", 1004);
        o.insert("minResourceFee", 4242);
        SimulateTransactionResponse r = SimulateTransactionResponse::fromJson(o);
        QCOMPARE(r.getMinResourceFee(), QString("4242"));
    }
};

ADD_TEST(SorobanRpcSendSimulateDeserializerTest)
#endif // SENDSIMULATEDESERIALIZERTEST_H
