#ifndef SENDSIMULATEDESERIALIZERTEST_H
#define SENDSIMULATEDESERIALIZERTEST_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <stdexcept>
#include <QtTest>
#include "../../testcollector.h"

#include "../../../src/responses/sorobanrpc/sendtransactionresponse.h"
#include "../../../src/responses/sorobanrpc/simulatetransactionresponse.h"
#include "readonlyresponsestest.h"

class SorobanRpcSendSimulateDeserializerTest: public QObject, private RpcAnswerChecks
{
    Q_OBJECT

    // Real answers, saved as soroban-testnet.stellar.org gave them on
    // 2026-09-28 (stellar-rpc 29.0.0, protocol 28).

    /** A payment sent once... */
    static QByteArray realSendPending() { return QByteArray("{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"status\":\"PENDING\",\"hash\":\"1c2eb969d67a1846070bbbb799fcfc2c58c5272f9dc597f67c0e469726dfea96\",\"latestLedger\":4919962,\"latestLedgerCloseTime\":\"1790623397\"}}"); }
    /** ...and again. */
    static QByteArray realSendDuplicate() { return QByteArray("{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"status\":\"DUPLICATE\",\"hash\":\"1c2eb969d67a1846070bbbb799fcfc2c58c5272f9dc597f67c0e469726dfea96\",\"latestLedger\":4919962,\"latestLedgerCloseTime\":\"1790623397\"}}"); }
    /** A payment with a sequence number ahead of the account's: txBAD_SEQ. */
    static QByteArray realSendError() { return QByteArray("{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"errorResultXdr\":\"AAAAAAAAAGT////7AAAAAA==\",\"status\":\"ERROR\",\"hash\":\"5c9702cc0a7b3a398386336c61fd079cd66d953b7473e5da97dff098a13d90d3\",\"latestLedger\":4919962,\"latestLedgerCloseTime\":\"1790623397\"}}"); }
    /** The native asset contract's balance(): no auth, no state changes. */
    static QByteArray realSimulateReadOnly() { return QByteArray("{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"transactionData\":\"AAAAAAAAAAIAAAAAAAAAAHoo3htrqIeTVjLBlsRnCBpnFTtHBaULYqRTOFENlMpUAAAABgAAAAHXkotywnA8z+r365/0701QSlWouXn8m0UOoshCtNHOYQAAABQAAAABAAAAAAACVeUAAACQAAAAAAAAAAAAADgU\",\"events\":[\"AAAAAQAAAAAAAAAAAAAAAgAAAAAAAAADAAAADwAAAAdmbl9jYWxsAAAAAA0AAAAg15KLcsJwPM/q9+uf9O9NUEpVqLl5/JtFDqLIQrTRzmEAAAAPAAAAB2JhbGFuY2UAAAAAEgAAAAAAAAAAeijeG2uoh5NWMsGWxGcIGmcVO0cFpQtipFM4UQ2UylQ=\",\"AAAAAQAAAAAAAAAB15KLcsJwPM/q9+uf9O9NUEpVqLl5/JtFDqLIQrTRzmEAAAACAAAAAAAAAAIAAAAPAAAACWZuX3JldHVybgAAAAAAAA8AAAAHYmFsYW5jZQAAAAAKAAAAAAAAAAAAAAAXSHbnnA==\"],\"minResourceFee\":\"14356\",\"results\":[{\"auth\":[],\"xdr\":\"AAAACgAAAAAAAAAAAAAAF0h255w=\"}],\"latestLedger\":4919964}}"); }
    /** Its transfer() to a contract: an auth entry, an updated account and a
     *  created balance (whose `before` is null). */
    static QByteArray realSimulateTransfer() { return QByteArray("{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"transactionData\":\"AAAAAQAAAAEAAAABAAAAAQAAAAYAAAAB15KLcsJwPM/q9+uf9O9NUEpVqLl5/JtFDqLIQrTRzmEAAAAUAAAAAQAAAAIAAAAAAAAAAHoo3htrqIeTVjLBlsRnCBpnFTtHBaULYqRTOFENlMpUAAAABgAAAAHXkotywnA8z+r365/0701QSlWouXn8m0UOoshCtNHOYQAAABAAAAABAAAAAgAAAA8AAAAHQmFsYW5jZQAAAAASAAAAASW0/NhZrsL6Y0hDjEibPDwQyYttIb5P08swy2iVPvl3AAAAAQAEQ2wAAAFwAAABcAAAAAAABu3Y\",\"events\":[\"AAAAAQAAAAAAAAAAAAAAAgAAAAAAAAADAAAADwAAAAdmbl9jYWxsAAAAAA0AAAAg15KLcsJwPM/q9+uf9O9NUEpVqLl5/JtFDqLIQrTRzmEAAAAPAAAACHRyYW5zZmVyAAAAEAAAAAEAAAADAAAAEgAAAAAAAAAAeijeG2uoh5NWMsGWxGcIGmcVO0cFpQtipFM4UQ2UylQAAAASAAAAASW0/NhZrsL6Y0hDjEibPDwQyYttIb5P08swy2iVPvl3AAAACgAAAAAAAAAAAAAAAAAAAAo=\",\"AAAAAQAAAAAAAAAB15KLcsJwPM/q9+uf9O9NUEpVqLl5/JtFDqLIQrTRzmEAAAABAAAAAAAAAAQAAAAPAAAACHRyYW5zZmVyAAAAEgAAAAAAAAAAeijeG2uoh5NWMsGWxGcIGmcVO0cFpQtipFM4UQ2UylQAAAASAAAAASW0/NhZrsL6Y0hDjEibPDwQyYttIb5P08swy2iVPvl3AAAADgAAAAZuYXRpdmUAAAAAAAoAAAAAAAAAAAAAAAAAAAAK\",\"AAAAAQAAAAAAAAAB15KLcsJwPM/q9+uf9O9NUEpVqLl5/JtFDqLIQrTRzmEAAAACAAAAAAAAAAIAAAAPAAAACWZuX3JldHVybgAAAAAAAA8AAAAIdHJhbnNmZXIAAAAB\"],\"minResourceFee\":\"454104\",\"results\":[{\"auth\":[\"AAAAAAAAAAAAAAAB15KLcsJwPM/q9+uf9O9NUEpVqLl5/JtFDqLIQrTRzmEAAAAIdHJhbnNmZXIAAAADAAAAEgAAAAAAAAAAeijeG2uoh5NWMsGWxGcIGmcVO0cFpQtipFM4UQ2UylQAAAASAAAAASW0/NhZrsL6Y0hDjEibPDwQyYttIb5P08swy2iVPvl3AAAACgAAAAAAAAAAAAAAAAAAAAoAAAAA\"],\"xdr\":\"AAAAAQ==\"}],\"stateChanges\":[{\"type\":\"updated\",\"key\":\"AAAAAAAAAAB6KN4ba6iHk1YywZbEZwgaZxU7RwWlC2KkUzhRDZTKVA==\",\"before\":\"AEsSmwAAAAAAAAAAeijeG2uoh5NWMsGWxGcIGmcVO0cFpQtipFM4UQ2UylQAAAAXSHbnnABLEpkAAAABAAAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAwAAAAAASxKbAAAAAGq6vqoAAAAA\",\"after\":\"AAAAAAAAAAAAAAAAeijeG2uoh5NWMsGWxGcIGmcVO0cFpQtipFM4UQ2UylQAAAAXSHbnkgBLEpkAAAABAAAAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAwAAAAAASxKbAAAAAGq6vqoAAAAA\"},{\"type\":\"created\",\"key\":\"AAAABgAAAAHXkotywnA8z+r365/0701QSlWouXn8m0UOoshCtNHOYQAAABAAAAABAAAAAgAAAA8AAAAHQmFsYW5jZQAAAAASAAAAASW0/NhZrsL6Y0hDjEibPDwQyYttIb5P08swy2iVPvl3AAAAAQ==\",\"before\":null,\"after\":\"ABRlkgAAAAYAAAAAAAAAAdeSi3LCcDzP6vfrn/TvTVBKVai5efybRQ6iyEK00c5hAAAAEAAAAAEAAAACAAAADwAAAAdCYWxhbmNlAAAAABIAAAABJbT82FmuwvpjSEOMSJs8PBDJi20hvk/TyzDLaJU++XcAAAABAAAAEQAAAAEAAAADAAAADwAAAAZhbW91bnQAAAAAAAoAAAAAAAAAAAAAABdIdugKAAAADwAAAAphdXRob3JpemVkAAAAAAAAAAAAAQAAAA8AAAAIY2xhd2JhY2sAAAAAAAAAAAAAAAA=\"}],\"latestLedger\":4919969}}"); }
    /** A function the contract does not have. */
    static QByteArray realSimulateError() { return QByteArray("{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"error\":\"HostError: Error(Value, InvalidInput)\\n\\nEvent log (newest first):\\n   0: [Diagnostic Event] contract:CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC, topics:[error, Error(Value, InvalidInput)], data:[\\\"symbol not found in slice of strs\\\", no_such_fn]\\n   1: [Diagnostic Event] topics:[fn_call, CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC, no_such_fn], data:Void\\n\",\"events\":[\"AAAAAAAAAAAAAAAAAAAAAgAAAAAAAAADAAAADwAAAAdmbl9jYWxsAAAAAA0AAAAg15KLcsJwPM/q9+uf9O9NUEpVqLl5/JtFDqLIQrTRzmEAAAAPAAAACm5vX3N1Y2hfZm4AAAAAAAE=\",\"AAAAAAAAAAAAAAAB15KLcsJwPM/q9+uf9O9NUEpVqLl5/JtFDqLIQrTRzmEAAAACAAAAAAAAAAIAAAAPAAAABWVycm9yAAAAAAAAAgAAAAgAAAACAAAAEAAAAAEAAAACAAAADgAAACFzeW1ib2wgbm90IGZvdW5kIGluIHNsaWNlIG9mIHN0cnMAAAAAAAAPAAAACm5vX3N1Y2hfZm4AAA==\"],\"latestLedger\":4919964}}"); }
    /** A transaction the RPC could not read: latestLedger 0. */
    static QByteArray realSimulateUnreadable() { return QByteArray("{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"error\":\"Could not unmarshal transaction\",\"latestLedger\":0}}"); }
    /** An ExtendFootprintTTL of an archived contract instance. */
    static QByteArray realSimulateRestore() { return QByteArray("{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"transactionData\":\"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAALbw=\",\"minResourceFee\":\"11708\",\"restorePreamble\":{\"transactionData\":\"AAAAAAAAAAAAAAABAAAABgAAAAGO6RTFCkhAo5jXdOOWx61gLF/Lr+Pk/EA33oqAxqxpxgAAABQAAAABAAAAAAAAAVgAAAFYAAAAAAACm/Q=\",\"minResourceFee\":\"170996\"},\"latestLedger\":4920000}}"); }

    static QJsonObject realSendResult() { return resultOf(realSendPending()); }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    // ---- sendTransaction ---------------------------------------------------

    // The hash and close time are what the RPC sends: 64 hex digits and a
    // decimal string (they were "d1234" and a date, which are now refused).
    void testSendTransactionPending()
    {
        QJsonObject o;
        o.insert("status", QStringLiteral("PENDING"));
        o.insert("hash", QStringLiteral("1c2eb969d67a1846070bbbb799fcfc2c58c5272f9dc597f67c0e469726dfea96"));
        o.insert("latestLedger", 100);
        o.insert("latestLedgerCloseTime", QStringLiteral("1790623397"));
        SendTransactionResponse r = SendTransactionResponse::fromJson(o);
        QCOMPARE(r.getStatus(), SendTransactionResponse::Status::PENDING);
        QCOMPARE(r.getHash(), QString("1c2eb969d67a1846070bbbb799fcfc2c58c5272f9dc597f67c0e469726dfea96"));
        QCOMPARE(r.getLatestLedger(), quint32(100));
    }

    void testSendTransactionDuplicate()
    {
        QJsonObject o = realSendResult();
        o.insert("status", QStringLiteral("DUPLICATE"));
        o.insert("latestLedger", 200);
        SendTransactionResponse r = SendTransactionResponse::fromJson(o);
        QCOMPARE(r.getStatus(), SendTransactionResponse::Status::DUPLICATE);
        QCOMPARE(r.getHash(), QString("1c2eb969d67a1846070bbbb799fcfc2c58c5272f9dc597f67c0e469726dfea96"));
    }

    void testSendTransactionTryAgainLater()
    {
        QJsonObject o = realSendResult();
        o.insert("status", QStringLiteral("TRY_AGAIN_LATER"));
        SendTransactionResponse r = SendTransactionResponse::fromJson(o);
        QCOMPARE(r.getStatus(), SendTransactionResponse::Status::TRY_AGAIN_LATER);
    }

    // The result and events are base64 XDR (they were "AAAA", three bytes,
    // and "event1base64", which are now refused).
    void testSendTransactionError()
    {
        QJsonObject o = resultOf(realSendError());
        QJsonArray diag;
        diag.append(QStringLiteral("AAAAAQ=="));
        diag.append(QStringLiteral("AAAAAg=="));
        o.insert("diagnosticEventsXdr", diag);
        SendTransactionResponse r = SendTransactionResponse::fromJson(o);
        QCOMPARE(r.getStatus(), SendTransactionResponse::Status::ERROR_);
        QCOMPARE(r.getErrorResultXdr(), QString("AAAAAAAAAGT////7AAAAAA=="));
        QCOMPARE(r.getDiagnosticEventsXdr().size(), 2);
        QCOMPARE(r.getDiagnosticEventsXdr().at(0), QString("AAAAAQ=="));
    }

    /**
     * latestLedger is a uint32 the RPC sends as a JSON integer. This test
     * expected a string to be read, as toUInt() read it (so "abc" was ledger
     * 0); a string is now refused, and the largest JSON integer read exactly.
     */
    void testSendTransactionLatestLedgerString()
    {
        QJsonObject o = realSendResult();
        o.insert("latestLedger", QStringLiteral("4294967295"));
        const QString message = refusal([&] { SendTransactionResponse::fromJson(o); });
        QVERIFY2(message.contains(QStringLiteral("latestLedger")), qPrintable(message));
        o.insert("latestLedger", qint64(4294967295LL));
        SendTransactionResponse r = SendTransactionResponse::fromJson(o);
        QCOMPARE(r.getLatestLedger(), quint32(4294967295U));
    }

    /**
     * A missing or unknown status is a malformed answer and is refused. It
     * used to read as PENDING: the caller polled for a transaction the RPC
     * never acknowledged, and kept the sequence number it took.
     */
    void testSendTransactionRefusesAMissingOrUnknownStatus()
    {
        const QList<QJsonValue> statuses = {
            QJsonValue(QJsonValue::Undefined), QJsonValue(QJsonValue::Null),
            QJsonValue(QString()), QJsonValue(QStringLiteral("pending")),
            QJsonValue(QStringLiteral("SUCCESS")), QJsonValue(QStringLiteral("PENDING ")),
            QJsonValue(0), QJsonValue(true), QJsonValue(QJsonObject()),
        };
        for (const QJsonValue& status : statuses) {
            QJsonObject o = realSendResult();
            o.remove("status");
            if (!status.isUndefined())
                o.insert("status", status);
            bool threw = false;
            try { SendTransactionResponse::fromJson(o); }
            catch (const std::runtime_error&) { threw = true; }
            QVERIFY2(threw, qPrintable(QString::fromUtf8(
                QJsonDocument(o).toJson(QJsonDocument::Compact))));
        }
    }

    /** Real sendTransaction answers are read as sent. */
    void testReadsRealSendTransactionAnswers()
    {
        const SendTransactionResponse pending = SendTransactionResponse::fromJson(resultOf(realSendPending()));
        QCOMPARE(pending.getStatus(), SendTransactionResponse::Status::PENDING);
        QCOMPARE(pending.getHash(), QString("1c2eb969d67a1846070bbbb799fcfc2c58c5272f9dc597f67c0e469726dfea96"));
        QCOMPARE(pending.getLatestLedger(), quint32(4919962));
        QCOMPARE(pending.getLatestLedgerCloseTime(), QString("1790623397"));
        QVERIFY(pending.getErrorResultXdr().isEmpty());

        const SendTransactionResponse duplicate = SendTransactionResponse::fromJson(resultOf(realSendDuplicate()));
        QCOMPARE(duplicate.getStatus(), SendTransactionResponse::Status::DUPLICATE);
        QCOMPARE(duplicate.getHash(), pending.getHash());

        const SendTransactionResponse error = SendTransactionResponse::fromJson(resultOf(realSendError()));
        QCOMPARE(error.getStatus(), SendTransactionResponse::Status::ERROR_);
        QCOMPARE(error.getHash(), QString("5c9702cc0a7b3a398386336c61fd079cd66d953b7473e5da97dff098a13d90d3"));
        QCOMPARE(error.getErrorResultXdr(), QString("AAAAAAAAAGT////7AAAAAA=="));
        QVERIFY(error.getDiagnosticEventsXdr().isEmpty());
    }

    /**
     * Every other field as the RPC sends it, or the answer is refused. The
     * hash is what the transaction is then polled by: anything used to be
     * taken for one. A missing or non-numeric latestLedger read as 0, and
     * errorResultXdr was taken as it came, with any status.
     */
    void testSendTransactionRefusesWhatTheRpcDoesNotSend()
    {
        QList<Case> cases = badHash(QStringLiteral("hash"));
        cases.append(badUint32(QStringLiteral("latestLedger")));
        cases.append(badDecimal(QStringLiteral("latestLedgerCloseTime"), QStringLiteral("9223372036854775808")));
        cases.append({ QStringLiteral("errorResultXdr"), QStringLiteral("AAAAAAAAAGT////7AAAAAA==") });
        cases.append({ QStringLiteral("diagnosticEventsXdr"), QStringLiteral("AAAAAQ==") });
        cases.append({ QStringLiteral("diagnosticEventsXdr"), QJsonArray{ QStringLiteral("AAAA") } });
        auto read = [](const QJsonObject& o) { SendTransactionResponse::fromJson(o); };
        QString problem = checkRefusals(QStringLiteral("sendTransaction"), resultOf(realSendPending()), cases, read);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));

        // With ERROR, the result is what the network said: it must be there.
        problem = checkRefusals(QStringLiteral("sendTransaction"), resultOf(realSendError()),
                                badXdr(QStringLiteral("errorResultXdr")), read);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
    }

    // ---- simulateTransaction ----------------------------------------------

    // The footprint is base64 XDR (it was "txdata-b64", which is now refused).
    void testSimulateMinimal()
    {
        QJsonObject o;
        o.insert("latestLedger", 1000);
        o.insert("transactionData", QStringLiteral("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAALbw="));
        o.insert("minResourceFee", QStringLiteral("12345"));
        SimulateTransactionResponse r = SimulateTransactionResponse::fromJson(o);
        QCOMPARE(r.getLatestLedger(), quint32(1000));
        QCOMPARE(r.getTransactionData(), QString("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAALbw="));
        QCOMPARE(r.getMinResourceFee(), QString("12345"));
        QVERIFY(!r.isError());
        QCOMPARE(r.getResults().size(), 0);
        QCOMPARE(r.getEvents().size(), 0);
    }

    // Events, return value and auth are base64 XDR (they were "ev-1",
    // "scval-b64" and "auth-entry-b64", which are now refused).
    void testSimulateWithResults()
    {
        QJsonObject o;
        o.insert("latestLedger", 1001);
        o.insert("transactionData", QStringLiteral("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAALbw="));
        o.insert("minResourceFee", QStringLiteral("999"));
        QJsonArray events;
        events.append(QStringLiteral("AAAAAQ=="));
        events.append(QStringLiteral("AAAAAg=="));
        o.insert("events", events);

        QJsonObject res0;
        res0.insert("xdr", QStringLiteral("AAAAAQ=="));
        QJsonArray auth0;
        auth0.append(QStringLiteral("AAAAAAAAAAA="));
        res0.insert("auth", auth0);
        QJsonArray results;
        results.append(res0);
        o.insert("results", results);

        SimulateTransactionResponse r = SimulateTransactionResponse::fromJson(o);
        QCOMPARE(r.getEvents().size(), 2);
        QCOMPARE(r.getResults().size(), 1);
        QCOMPARE(r.getResults().first().xdr, QString("AAAAAQ=="));
        QCOMPARE(r.getResults().first().auth.size(), 1);
        QCOMPARE(r.getResults().first().auth.first(), QString("AAAAAAAAAAA="));
    }

    // Keys and entries are base64 XDR (they were "ledgerkey-b64" and the like,
    // which are now refused).
    void testSimulateStateChanges()
    {
        QJsonObject o;
        o.insert("latestLedger", 1002);
        QJsonArray changes;
        QJsonObject c0;
        c0.insert("type", QStringLiteral("updated"));
        c0.insert("key", QStringLiteral("AAAAAQ=="));
        c0.insert("before", QStringLiteral("AAAAAg=="));
        c0.insert("after", QStringLiteral("AAAAAw=="));
        changes.append(c0);
        QJsonObject c1;
        c1.insert("type", QStringLiteral("created"));
        c1.insert("key", QStringLiteral("AAAABA=="));
        c1.insert("after", QStringLiteral("AAAABQ=="));
        changes.append(c1);
        o.insert("stateChanges", changes);

        SimulateTransactionResponse r = SimulateTransactionResponse::fromJson(o);
        QCOMPARE(r.getStateChanges().size(), 2);
        QCOMPARE(r.getStateChanges().at(0).type, QString("updated"));
        QCOMPARE(r.getStateChanges().at(0).before, QString("AAAAAg=="));
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

    /**
     * minResourceFee is an int64 the RPC sends as a decimal string. This test
     * expected a JSON number to be read too; a number is now refused, as
     * anything but the string the RPC sends (on Qt 5 a large one was read
     * through a double).
     */
    void testSimulateMinResourceFeeAsInt()
    {
        QJsonObject o;
        o.insert("latestLedger", 1004);
        o.insert("minResourceFee", 4242);
        const QString message = refusal([&] { SimulateTransactionResponse::fromJson(o); });
        QVERIFY2(message.contains(QStringLiteral("minResourceFee")), qPrintable(message));
        o.insert("minResourceFee", QStringLiteral("4242"));
        QCOMPARE(SimulateTransactionResponse::fromJson(o).getMinResourceFee(), QString("4242"));
    }

    /** Real simulateTransaction answers, of every kind, are read as sent. */
    void testReadsRealSimulateTransactionAnswers()
    {
        const QJsonObject readOnlyJson = resultOf(realSimulateReadOnly());
        const SimulateTransactionResponse readOnly = SimulateTransactionResponse::fromJson(readOnlyJson);
        QVERIFY(!readOnly.isError());
        QVERIFY(!readOnly.needsRestore());
        QCOMPARE(readOnly.getLatestLedger(), quint32(4919964));
        QCOMPARE(readOnly.getMinResourceFee(), QString("14356"));
        QCOMPARE(readOnly.getTransactionData(), readOnlyJson.value("transactionData").toString());
        QCOMPARE(readOnly.getEvents().size(), 2);
        QCOMPARE(readOnly.getResults().size(), 1);
        QCOMPARE(readOnly.getResults().first().xdr, QString("AAAACgAAAAAAAAAAAAAAF0h255w="));
        QVERIFY(readOnly.getResults().first().auth.isEmpty());
        QVERIFY(readOnly.getStateChanges().isEmpty());

        const QJsonObject transferJson = resultOf(realSimulateTransfer());
        const SimulateTransactionResponse transfer = SimulateTransactionResponse::fromJson(transferJson);
        QCOMPARE(transfer.getMinResourceFee(), QString("454104"));
        QCOMPARE(transfer.getResults().first().xdr, QString("AAAAAQ=="));
        QCOMPARE(transfer.getResults().first().auth.size(), 1);
        QCOMPARE(transfer.getResults().first().auth.first(),
                 transferJson.value("results").toArray().first().toObject().value("auth").toArray().first().toString());
        QCOMPARE(transfer.getStateChanges().size(), 2);
        QCOMPARE(transfer.getStateChanges().at(0).type, QString("updated"));
        QCOMPARE(transfer.getStateChanges().at(0).key, QString("AAAAAAAAAAB6KN4ba6iHk1YywZbEZwgaZxU7RwWlC2KkUzhRDZTKVA=="));
        QVERIFY(!transfer.getStateChanges().at(0).before.isEmpty());
        QCOMPARE(transfer.getStateChanges().at(1).type, QString("created"));
        QVERIFY(transfer.getStateChanges().at(1).before.isEmpty());
        QVERIFY(!transfer.getStateChanges().at(1).after.isEmpty());

        const SimulateTransactionResponse error = SimulateTransactionResponse::fromJson(resultOf(realSimulateError()));
        QVERIFY(error.isError());
        QVERIFY(error.getError().startsWith(QStringLiteral("HostError: Error(Value, InvalidInput)")));
        QCOMPARE(error.getEvents().size(), 2);
        QCOMPARE(error.getLatestLedger(), quint32(4919964));
        QVERIFY(error.getTransactionData().isEmpty());
        QVERIFY(error.getMinResourceFee().isEmpty());

        const SimulateTransactionResponse unreadable = SimulateTransactionResponse::fromJson(resultOf(realSimulateUnreadable()));
        QCOMPARE(unreadable.getError(), QString("Could not unmarshal transaction"));
        QCOMPARE(unreadable.getLatestLedger(), quint32(0));

        const QJsonObject restoreJson = resultOf(realSimulateRestore());
        const SimulateTransactionResponse restore = SimulateTransactionResponse::fromJson(restoreJson);
        QVERIFY(!restore.isError());
        QVERIFY(restore.needsRestore());
        QCOMPARE(restore.getMinResourceFee(), QString("11708"));
        QCOMPARE(restore.getRestorePreamble().minResourceFee, QString("170996"));
        QCOMPARE(restore.getRestorePreamble().transactionData,
                 restoreJson.value("restorePreamble").toObject().value("transactionData").toString());
        QVERIFY(restore.getResults().isEmpty());
    }

    /**
     * Every field as the RPC sends it, or the answer is refused. A missing or
     * non-numeric latestLedger used to read as 0; a minResourceFee that was
     * not a number was kept as text, and a JSON number read; an empty error
     * read as a simulation that worked; a state change of any type, with its
     * entries on either side, was taken as it came.
     */
    void testSimulateTransactionRefusesWhatTheRpcDoesNotSend()
    {
        const QString i64Over = QStringLiteral("9223372036854775808");
        auto read = [](const QJsonObject& o) { SimulateTransactionResponse::fromJson(o); };

        QList<Case> cases = badUint32(QStringLiteral("latestLedger"));
        for (const Case& c : badDecimal(QStringLiteral("minResourceFee"), i64Over)) {
            if (!c.second.isUndefined())    // left out when it is 0
                cases.append(c);
        }
        for (const Case& c : badXdr(QStringLiteral("transactionData"))) {
            if (!c.second.isUndefined())    // left out when the simulation failed
                cases.append(c);
        }
        cases.append({ QStringLiteral("events"), QStringLiteral("AAAAAQ==") });
        cases.append({ QStringLiteral("events.1"), QStringLiteral("AAAA") });
        cases.append({ QStringLiteral("error"), QString() });
        cases.append({ QStringLiteral("error"), 12 });
        cases.append({ QStringLiteral("error"), QJsonValue(QJsonValue::Null) });
        cases.append({ QStringLiteral("results"), QJsonObject() });
        cases.append({ QStringLiteral("results.0"), QStringLiteral("AAAAAQ==") });
        cases.append(badXdr(QStringLiteral("results.0.xdr")));
        cases.append({ QStringLiteral("results.0.auth"), QStringLiteral("AAAAAQ==") });
        cases.append({ QStringLiteral("results.0.auth.0"), QStringLiteral("AAAA!AAA") });
        const QString s0 = QStringLiteral("stateChanges.0.");
        for (const QJsonValue& type : { QJsonValue(QStringLiteral("Updated")), QJsonValue(QStringLiteral("restored")),
                                        QJsonValue(2), QJsonValue(QJsonValue::Null), missing() })
            cases.append({ s0 + QStringLiteral("type"), type });
        cases.append(badXdr(s0 + QStringLiteral("key")));
        cases.append({ s0 + QStringLiteral("before"), QStringLiteral("AAAA") });
        cases.append({ s0 + QStringLiteral("before"), QJsonValue(QJsonValue::Null) });   // updated has both
        cases.append({ s0 + QStringLiteral("after"), missing() });
        cases.append({ QStringLiteral("stateChanges.1.before"), QStringLiteral("AAAAAQ==") });  // created has none
        QJsonObject deletedWithAfter;                                                            // deleted has no after
        deletedWithAfter.insert("type", QStringLiteral("deleted"));
        deletedWithAfter.insert("key", QStringLiteral("AAAAAQ=="));
        deletedWithAfter.insert("before", QStringLiteral("AAAAAg=="));
        deletedWithAfter.insert("after", QStringLiteral("AAAAAw=="));
        cases.append({ QStringLiteral("stateChanges.1"), deletedWithAfter });
        QString problem = checkRefusals(QStringLiteral("simulateTransaction"), resultOf(realSimulateTransfer()), cases, read);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));

        QList<Case> restore;
        restore.append({ QStringLiteral("restorePreamble"), QStringLiteral("AAAAAQ==") });
        restore.append({ QStringLiteral("restorePreamble"), QJsonValue(QJsonValue::Null) });
        restore.append(badXdr(QStringLiteral("restorePreamble.transactionData")));
        restore.append(badDecimal(QStringLiteral("restorePreamble.minResourceFee"), i64Over));
        problem = checkRefusals(QStringLiteral("simulateTransaction"), resultOf(realSimulateRestore()), restore, read);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));

        // What may be left out is: a failed simulation has no footprint or fee.
        const SimulateTransactionResponse bare = SimulateTransactionResponse::fromJson(
            edited(edited(resultOf(realSimulateTransfer()), QStringLiteral("minResourceFee"), missing()),
                   QStringLiteral("results.0.auth"), missing()));
        QVERIFY(bare.getMinResourceFee().isEmpty());
        QVERIFY(bare.getResults().first().auth.isEmpty());
    }
};

ADD_TEST(SorobanRpcSendSimulateDeserializerTest)
#endif // SENDSIMULATEDESERIALIZERTEST_H
