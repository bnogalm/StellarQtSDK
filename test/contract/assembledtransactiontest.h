#ifndef ASSEMBLEDTRANSACTIONTEST_H
#define ASSEMBLEDTRANSACTIONTEST_H

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QTcpServer>
#include <QtTest>
#include <cstring>
#include "../testcollector.h"

#include "../../src/contract/assembledtransaction.h"
#include "../../src/sorobanserver.h"
#include "../../src/sorobandatabuilder.h"
#include "../../src/transactionbuilder.h"
#include "../../src/transaction.h"
#include "../../src/account.h"
#include "../../src/keypair.h"
#include "../../src/network.h"
#include "../../src/asset.h"
#include "../../src/invokehostfunctionoperation.h"
#include "../../src/scval/scv.h"
#include "../../src/util.h"
#include "../../src/exception/notyetsimulatedexception.h"
#include "../../src/exception/simulationfailedexception.h"
#include "../../src/exception/sendfailedexception.h"
#include "../../src/exception/assembledtransactionexception.h"

/** Scripted server: feeds prebuilt sim/send/fetch responses without
 *  hitting the network. Declared at file scope (not nested) so moc parses cleanly. */
class StubSorobanServer : public SorobanServer
{
public:
    StubSorobanServer() : SorobanServer(QUrl("http://127.0.0.1:1/")) {}

    QList<SimulateTransactionResponse> simResponses;
    QList<SendTransactionResponse> sendResponses;
    QList<GetTransactionResponse> fetchResponses;
    int simIdx = 0, sendIdx = 0, fetchIdx = 0;
    QStringList sentEnvelopes;

    SimulateTransactionResponse simulateTransactionXdr(const QString&) override {
        return simResponses.at(qMin(simIdx++, int(simResponses.size() - 1)));
    }
    SendTransactionResponse sendTransactionXdr(const QString& envelope) override {
        sentEnvelopes.append(envelope);
        return sendResponses.at(qMin(sendIdx++, int(sendResponses.size() - 1)));
    }
protected:
    GetTransactionResponse fetchTransaction(const QString&) override {
        return fetchResponses.at(qMin(fetchIdx++, int(fetchResponses.size() - 1)));
    }
};

/** Simulates and sends from a script, and polls for real, over HTTP to
 *  `endpoint`. */
class SendsThenPollsOverHttpServer : public SorobanServer
{
public:
    explicit SendsThenPollsOverHttpServer(const QUrl& endpoint) : SorobanServer(endpoint) {}
    SimulateTransactionResponse sim;
    SendTransactionResponse sent;
    SimulateTransactionResponse simulateTransactionXdr(const QString&) override { return sim; }
    SendTransactionResponse sendTransactionXdr(const QString&) override { return sent; }
};

/** Answers every request with `result`, as the JSON-RPC result, and keeps
 *  the method and params asked: SorobanServer's own simulateTransactionXdr
 *  overloads run. */
class AnsweringParamsSorobanServer : public SorobanServer
{
public:
    AnsweringParamsSorobanServer() : SorobanServer(QUrl("http://127.0.0.1:1/")) {}
    QJsonObject result;
    QString lastMethod;
    QJsonObject lastParams;
    QJsonValue sendRequest(const QString& method, const QJsonValue& params) override {
        lastMethod = method;
        lastParams = params.toObject();
        return result;
    }
};

class AssembledTransactionTest: public QObject
{
    Q_OBJECT

    static KeyPair* sourceKeyPair() {
        return KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
    }

    static Transaction* buildSorobanTx()
    {
        Account* acc = new Account(sourceKeyPair(), 41);
        TransactionBuilder b(acc, Network::testnetNetwork());
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        b.addOperation(
            InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(native));
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        Transaction* tx = b.build();
        delete acc;
        return tx;
    }

    /** Encode any XDR-serializable value to a base64 string. */
    template<typename T>
    static QString xdrToBase64(const T& value) {
        QByteArray buf;
        QDataStream s(&buf, QIODevice::WriteOnly);
        s << value;
        return QString::fromLatin1(buf.toBase64(XDR_BASE64ENCODING));
    }

    /** One entry the simulated call writes: without it, a call that asks for
     *  no authorization is a read call, which sign() refuses. */
    static stellar::SorobanLedgerKey writtenKey()
    {
        stellar::SorobanLedgerKey k;
        k.type = stellar::LedgerEntryType::CONTRACT_CODE;
        for (int i = 0; i < 32; ++i) k.contractCode.hash[i] = quint8(0x11);
        return k;
    }

    static SimulateTransactionResponse makeSimulationResponse(qint64 resourceFee, const stellar::SCVal& resultVal)
    {
        stellar::SorobanTransactionData td = SorobanDataBuilder()
            .setReadWrite({writtenKey()}).setResources(123456u, 4096u, 2048u).setResourceFee(resourceFee).build();

        QJsonObject result0;
        result0.insert("xdr", xdrToBase64(resultVal));
        QJsonArray results;
        results.append(result0);

        QJsonObject simJson;
        simJson.insert("latestLedger", 1);
        simJson.insert("transactionData", xdrToBase64(td));
        simJson.insert("minResourceFee", QString::number(resourceFee));
        simJson.insert("results", results);
        return SimulateTransactionResponse::fromJson(simJson);
    }

    /** A sendTransaction answer with `status`, whole as the RPC sends it (a
     *  real testnet answer's hash, ledger and close time; txBAD_SEQ for ERROR). */
    static SendTransactionResponse sendStatus(const QString& status)
    {
        QJsonObject o;
        o.insert("status", status);
        o.insert("hash", QStringLiteral("1c2eb969d67a1846070bbbb799fcfc2c58c5272f9dc597f67c0e469726dfea96"));
        o.insert("latestLedger", 4919962);
        o.insert("latestLedgerCloseTime", QStringLiteral("1790623397"));
        if (status == QLatin1String("ERROR"))
            o.insert("errorResultXdr", QStringLiteral("AAAAAAAAAGT////7AAAAAA=="));
        return SendTransactionResponse::fromJson(o);
    }

    /** A getTransaction answer with `status`, whole as the RPC sends it: the
     *  ledger range, and the transaction's own fields once it is found (its
     *  XDR placeholders, which nothing here decodes). */
    static GetTransactionResponse txStatus(const QString& status)
    {
        const bool found = status != QLatin1String("NOT_FOUND");
        QJsonObject o;
        o.insert("status", status);
        o.insert("txHash", QStringLiteral("1c2eb969d67a1846070bbbb799fcfc2c58c5272f9dc597f67c0e469726dfea96"));
        o.insert("latestLedger", 4919964);
        o.insert("latestLedgerCloseTime", QStringLiteral("1790623407"));
        o.insert("oldestLedger", 4799005);
        o.insert("oldestLedgerCloseTime", QStringLiteral("1790018612"));
        o.insert("applicationOrder", found ? 2 : 0);
        o.insert("feeBump", false);
        o.insert("ledger", found ? 4919963 : 0);
        o.insert("createdAt", found ? QStringLiteral("1790623402") : QStringLiteral("0"));
        if (found) {
            o.insert("envelopeXdr", QStringLiteral("AAAAAg=="));
            o.insert("resultXdr", QStringLiteral("AAAAAA=="));
            o.insert("resultMetaXdr", QStringLiteral("AAAABA=="));
        }
        return GetTransactionResponse::fromJson(o);
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testSimulateAppliesFeeAndDataToTransaction()
    {
        StubSorobanServer srv;
        srv.simResponses = { makeSimulationResponse(999, Scv::toUint32(42)) };

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        const qint64 originalFee = at.getTransaction()->getFee();
        at.simulate();

        QVERIFY(at.isSimulated());
        QCOMPARE(at.getTransaction()->getFee(), originalFee + 999);
        QVERIFY(!at.getTransaction()->getSorobanData().isNull());
        QCOMPARE(at.getTransaction()->getSorobanData()->resources.instructions, quint32(123456));
    }

    /**
     * simulate(options) sends what SimulateTransactionOptions asks for and
     * applies the answer as simulate() does, which still sends the
     * transaction alone. There was no way to ask for a leeway or an auth mode.
     */
    void testSimulateWithOptionsSendsThemAndApplies()
    {
        AnsweringParamsSorobanServer srv;
        srv.result.insert("latestLedger", 1);
        srv.result.insert("transactionData", xdrToBase64(
            SorobanDataBuilder().setResources(123456u, 4096u, 2048u).setResourceFee(qint64(999)).build()));
        srv.result.insert("minResourceFee", QStringLiteral("999"));
        QJsonObject result0;
        result0.insert("xdr", xdrToBase64(Scv::toUint32(42)));
        srv.result.insert("results", QJsonArray{result0});

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        const QString built = at.getTransaction()->toEnvelopeXdrBase64();
        at.simulate();
        QCOMPARE(srv.lastMethod, QStringLiteral("simulateTransaction"));
        QCOMPARE(srv.lastParams.keys(), QStringList{QStringLiteral("transaction")});
        QCOMPARE(srv.lastParams.value("transaction").toString(), built);

        SimulateTransactionOptions options;
        options.instructionLeeway = 2000000;
        options.authMode = SimulateTransactionOptions::AuthMode::Enforce;
        options.useUpgradedAuth = SimulateTransactionOptions::UpgradedAuth::NotRequested;
        const QString simulated = at.getTransaction()->toEnvelopeXdrBase64();
        at.simulate(options);
        QCOMPARE(srv.lastParams.value("transaction").toString(), simulated);
        QCOMPARE(srv.lastParams.value("authMode").toString(), QStringLiteral("enforce"));
        QVERIFY(srv.lastParams.value("useUpgradedAuth") == QJsonValue(false));
        QCOMPARE(srv.lastParams.value("resourceConfig").toObject().value("instructionLeeway").toInt(), 2000000);
        QCOMPARE(at.getTransaction()->getFee(), qint64(100 + 999));
        QCOMPARE(at.result().u32, 42u);
    }

    /**
     * simulate() is meant to be called again (after a restore, say), and every
     * call must leave the caller's 100-stroop bid as it was. With an RPC whose
     * minResourceFee differs from its footprint's resourceFee the bid used to
     * move on each call (100 + 999 first, then 1099 - 777 + 999 = 1321).
     */
    void testReSimulatingKeepsTheBidWhenMinResourceFeeDiffers()
    {
        auto withMinFee = [](qint64 resourceFee, qint64 minResourceFee) {
            QJsonObject simJson;
            simJson.insert("latestLedger", 1);
            simJson.insert("transactionData", xdrToBase64(
                SorobanDataBuilder().setResources(1u, 1u, 1u).setResourceFee(resourceFee).build()));
            simJson.insert("minResourceFee", QString::number(minResourceFee));
            QJsonObject result0;
            result0.insert("xdr", xdrToBase64(Scv::toUint32(1)));
            simJson.insert("results", QJsonArray{result0});
            return SimulateTransactionResponse::fromJson(simJson);
        };
        StubSorobanServer srv;
        srv.simResponses = { withMinFee(777, 999), withMinFee(777, 999),
                             withMinFee(1234, 1234), withMinFee(40, 0) };

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        at.simulate();
        QCOMPARE(at.getTransaction()->getFee(), qint64(100 + 777));
        at.simulate();
        QCOMPARE(at.getTransaction()->getFee(), qint64(100 + 777));
        at.simulate();
        QCOMPARE(at.getTransaction()->getFee(), qint64(100 + 1234));
        at.simulate();
        QCOMPARE(at.getTransaction()->getFee(), qint64(100 + 40));
        QCOMPARE(at.getTransaction()->getSorobanData()->resourceFee, qint64(40));
    }

    void testResultDecodesSCVal()
    {
        StubSorobanServer srv;
        srv.simResponses = { makeSimulationResponse(10, Scv::toUint32(0xCAFE)) };

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        at.simulate();
        stellar::SCVal v = at.result();
        QCOMPARE(v.type, stellar::SCValType::SCV_U32);
        QCOMPARE(Scv::fromUint32(v), quint32(0xCAFE));
    }

    void testResultThrowsBeforeSimulate()
    {
        StubSorobanServer srv;
        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        bool threw = false, typed = false;
        try { (void)at.result(); }
        catch (const qstellar::exception::NotYetSimulatedException&) { typed = threw = true; }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
        QVERIFY(typed);   // the precise type, not just "something threw"
    }

    void testSimulateThrowsOnSimulationError()
    {
        StubSorobanServer srv;
        QJsonObject simJson;
        simJson.insert("latestLedger", 1);
        simJson.insert("error", QStringLiteral("HostError: out of budget"));
        srv.simResponses = { SimulateTransactionResponse::fromJson(simJson) };

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        bool threw = false, typed = false;
        try { at.simulate(); }
        catch (const qstellar::exception::SimulationFailedException&) { typed = threw = true; }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
        QVERIFY(typed);
        QVERIFY(!at.isSimulated());
    }

    void testSignAndSendPollsUntilSuccess()
    {
        StubSorobanServer srv;
        srv.simResponses = { makeSimulationResponse(10, Scv::toVoid()) };

        srv.sendResponses = { sendStatus(QStringLiteral("PENDING")) };
        srv.fetchResponses = {
            txStatus(QStringLiteral("NOT_FOUND")),
            txStatus(QStringLiteral("SUCCESS")),
        };

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        at.simulate();
        GetTransactionResponse r = at.signAndSend(sourceKeyPair(), 5000, 1);
        QCOMPARE(r.getStatus(), GetTransactionResponse::Status::SUCCESS);
        QCOMPARE(srv.fetchIdx, 2);
    }

    /**
     * The value the call returned ON CHAIN is read from the landed
     * transaction's meta (js SentTransaction.result), and is not the
     * simulation's: here the simulation said 42 and the transaction (a real
     * testnet SAC deploy's meta) returned the contract's address.
     */
    void testLandedResultIsTheOnChainValue()
    {
        StubSorobanServer srv;
        srv.simResponses = { makeSimulationResponse(10, Scv::toUint32(42)) };
        srv.sendResponses = { sendStatus(QStringLiteral("PENDING")) };
        QJsonObject landed;
        landed.insert("status", QStringLiteral("SUCCESS"));
        landed.insert("txHash", QStringLiteral("1c2eb969d67a1846070bbbb799fcfc2c58c5272f9dc597f67c0e469726dfea96"));
        landed.insert("latestLedger", 4919964);
        landed.insert("latestLedgerCloseTime", QStringLiteral("1790623407"));
        landed.insert("oldestLedger", 4799005);
        landed.insert("oldestLedgerCloseTime", QStringLiteral("1790018612"));
        landed.insert("applicationOrder", 2);
        landed.insert("feeBump", false);
        landed.insert("ledger", 4919963);
        landed.insert("createdAt", QStringLiteral("1790623402"));
        landed.insert("envelopeXdr", QStringLiteral("AAAAAg=="));
        landed.insert("resultXdr", QStringLiteral("AAAAAA=="));
        landed.insert("resultMetaXdr", QStringLiteral(
            "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"));
        srv.fetchResponses = { GetTransactionResponse::fromJson(landed) };

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        at.simulate();
        QVERIFY_EXCEPTION_THROWN(at.landedResult(), qstellar::exception::AssembledTransactionException);
        at.signAndSend(sourceKeyPair(), 5000, 1);
        QCOMPARE(at.result().u32, 42u);
        const stellar::SCVal onChain = at.landedResult();
        QCOMPARE(int(onChain.type), int(stellar::SCValType::SCV_ADDRESS));
        QCOMPARE(xdr::encodeOrThrow(onChain, "SCVal").toBase64(), QByteArray("AAAAEgAAAAGg52a06oXRnZg/P6mxnK1JTLubY3kYS977xH8BCHjwUg=="));
    }

    /**
     * A read call (no authorization asked for, nothing written) is not signed
     * or sent unless forced: it changes nothing, and sending it only pays the
     * fee. js and py refuse it the same way (NoSignatureNeeded; force: true,
     * force=True). It used to be signed and sent. A call that writes, or that
     * asks for authorization, is not one.
     */
    void testAReadCallIsSignedAndSentOnlyWhenForced()
    {
        auto simulation = [](const QStringList& auth) {
            QJsonObject simJson;
            simJson.insert("latestLedger", 1);
            simJson.insert("transactionData", xdrToBase64(
                SorobanDataBuilder().setResources(1000u, 100u, 0u).setResourceFee(qint64(77)).build()));
            simJson.insert("minResourceFee", QStringLiteral("77"));
            QJsonObject result0;
            result0.insert("xdr", xdrToBase64(Scv::toUint32(7)));
            result0.insert("auth", QJsonArray::fromStringList(auth));
            simJson.insert("results", QJsonArray{result0});
            return SimulateTransactionResponse::fromJson(simJson);
        };
        StubSorobanServer srv;
        srv.simResponses = { simulation(QStringList()) };
        srv.sendResponses = { sendStatus(QStringLiteral("PENDING")) };
        srv.fetchResponses = { txStatus(QStringLiteral("SUCCESS")) };

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        QVERIFY_EXCEPTION_THROWN(at.isReadCall(), qstellar::exception::NotYetSimulatedException);
        at.simulate();
        QVERIFY(at.isReadCall());
        QCOMPARE(at.result().u32, 7u);
        QScopedPointer<KeyPair> signer(sourceKeyPair());
        QVERIFY_EXCEPTION_THROWN(at.sign(signer.data()), qstellar::exception::NoSignatureNeededException);
        QVERIFY_EXCEPTION_THROWN(at.signAndSend(signer.data(), 5000, 1),
                                 qstellar::exception::NoSignatureNeededException);
        QVERIFY(at.getTransaction()->getSignatures().isEmpty());
        QVERIFY(srv.sentEnvelopes.isEmpty());

        QCOMPARE(at.signAndSend(signer.data(), 5000, 1, true).getStatus(),
                 GetTransactionResponse::Status::SUCCESS);
        QCOMPARE(srv.sentEnvelopes.size(), 1);

        StubSorobanServer writing;
        writing.simResponses = { makeSimulationResponse(10, Scv::toVoid()) };
        AssembledTransaction writer(buildSorobanTx(), &writing, Network::testnetNetwork());
        writer.simulate();
        QVERIFY(!writer.isReadCall());

        stellar::SorobanAuthorizationEntry invoker;
        invoker.credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_SOURCE_ACCOUNT;
        invoker.rootInvocation.function.type =
            stellar::SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CONTRACT_FN;
        StubSorobanServer authorizing;
        authorizing.simResponses = { simulation(QStringList{xdrToBase64(invoker)}) };
        AssembledTransaction authorize(buildSorobanTx(), &authorizing, Network::testnetNetwork());
        authorize.simulate();
        QVERIFY(!authorize.isReadCall());
        authorize.sign(signer.data());
        QCOMPARE(authorize.getTransaction()->getSignatures().size(), 1);
    }

    void testSignAndSendThrowsOnSendError()
    {
        StubSorobanServer srv;
        srv.simResponses = { makeSimulationResponse(10, Scv::toVoid()) };

        srv.sendResponses = { sendStatus(QStringLiteral("ERROR")) };

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        at.simulate();
        bool threw = false, typed = false;
        try { at.signAndSend(sourceKeyPair(), 5000, 1); }
        catch (const qstellar::exception::SendFailedException&) { typed = threw = true; }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
        QVERIFY(typed);
    }

    /**
     * TRY_AGAIN_LATER means the RPC did not take the transaction in, so there
     * is nothing to poll for: SendFailedException at once, as restoreFootprint
     * throws. It used to be polled until the timeout as if it were PENDING, and
     * then fail as a timeout.
     */
    void testSignAndSendThrowsAtOnceOnTryAgainLater()
    {
        StubSorobanServer srv;
        srv.simResponses = { makeSimulationResponse(10, Scv::toVoid()) };
        srv.sendResponses = { sendStatus(QStringLiteral("TRY_AGAIN_LATER")) };
        srv.fetchResponses = { txStatus(QStringLiteral("NOT_FOUND")) };

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        at.simulate();
        bool typed = false;
        try { at.signAndSend(sourceKeyPair(), 2000, 1); }
        catch (const qstellar::exception::SendFailedException&) { typed = true; }
        catch (const std::exception&) {}
        QVERIFY(typed);
        QCOMPARE(srv.fetchIdx, 0);                  // never polled
        QCOMPARE(at.getSendResponse().getStatus(),
                 SendTransactionResponse::Status::TRY_AGAIN_LATER);
    }

    /**
     * The transaction keeps its sequence number after TRY_AGAIN_LATER, and
     * signAndSend() again sends the same envelope: the signer's signature is
     * not added a second time. It used to be, and the network refuses a
     * transaction with an extra signature (txBAD_AUTH_EXTRA).
     */
    void testSendingAgainAfterTryAgainLaterSendsTheSameEnvelope()
    {
        StubSorobanServer srv;
        srv.simResponses = { makeSimulationResponse(10, Scv::toVoid()) };
        srv.sendResponses = { sendStatus(QStringLiteral("TRY_AGAIN_LATER")),
                              sendStatus(QStringLiteral("PENDING")) };
        srv.fetchResponses = { txStatus(QStringLiteral("SUCCESS")) };

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        at.simulate();
        const qint64 sequence = at.getTransaction()->getSequenceNumber();
        QScopedPointer<KeyPair> signer(sourceKeyPair());
        bool typed = false;
        try { at.signAndSend(signer.data(), 2000, 1); }
        catch (const qstellar::exception::SendFailedException&) { typed = true; }
        QVERIFY(typed);

        const GetTransactionResponse r = at.signAndSend(signer.data(), 2000, 1);
        QCOMPARE(r.getStatus(), GetTransactionResponse::Status::SUCCESS);
        QCOMPARE(srv.sentEnvelopes.size(), 2);
        QCOMPARE(srv.sentEnvelopes.at(1), srv.sentEnvelopes.at(0));
        QCOMPARE(at.getTransaction()->getSignatures().size(), 1);
        QCOMPARE(at.getTransaction()->getSequenceNumber(), sequence);
    }

    /**
     * Once the RPC has answered PENDING, a poll whose own connection never
     * opens does not say the transaction never left: SorobanTransportException
     * with mayHaveReachedServer() true, naming the transaction's hash. It came
     * through with false and "the request was not sent" — what a send that
     * never left says — so a caller could send the payment again while the
     * first could still land.
     */
    void testSignAndSendNeverSaysNotSentOnceTheRpcTookItIn()
    {
        QTcpServer probe;
        probe.listen(QHostAddress::LocalHost, 0);
        const quint16 port = probe.serverPort();
        probe.close();                              // nothing listens there now
        SendsThenPollsOverHttpServer srv(QUrl(QStringLiteral("http://127.0.0.1:%1/").arg(port)));
        srv.sim = makeSimulationResponse(10, Scv::toVoid());

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        at.simulate();
        const QString hash = at.getTransaction()->hashHex();
        QJsonObject sendJson;
        sendJson.insert("status", QStringLiteral("PENDING"));
        sendJson.insert("hash", hash);
        sendJson.insert("latestLedger", 4917043);
        sendJson.insert("latestLedgerCloseTime", QStringLiteral("1790608752"));
        srv.sent = SendTransactionResponse::fromJson(sendJson);

        bool typed = false, mayHaveReachedServer = false;
        QString message;
        try {
            at.signAndSend(sourceKeyPair(), 10000, 1);
            message = QStringLiteral("no exception");
        } catch (const qstellar::exception::SorobanTransportException& e) {
            typed = true;
            mayHaveReachedServer = e.mayHaveReachedServer();
            message = QString::fromUtf8(e.what());
        } catch (const std::exception& e) {
            message = QString::fromUtf8(e.what());
        }
        QVERIFY2(typed, qPrintable(message));
        QVERIFY2(mayHaveReachedServer, qPrintable(message));
        QVERIFY2(message.contains(hash) && message.contains(QStringLiteral("may still be applied")),
                 qPrintable(message));
        QCOMPARE(at.getSendResponse().getStatus(), SendTransactionResponse::Status::PENDING);
    }

    // Typed Soroban exceptions must remain catchable through the old base
    // types — they derive SdkException → std::runtime_error → std::exception —
    // so existing consumer `catch (std::runtime_error&)` clauses keep working.
    void testTypedExceptionsRemainCatchableAsRuntimeError()
    {
        StubSorobanServer srv;
        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        bool caughtAsRuntimeError = false;
        try { (void)at.result(); }   // throws NotYetSimulatedException
        catch (const std::runtime_error&) { caughtAsRuntimeError = true; }
        QVERIFY(caughtAsRuntimeError);
    }

    /**
     * sign() attaches one signature, after simulate(): before, it is refused
     * (NotYetSimulatedException), as js and py refuse it, since an unsimulated
     * Soroban transaction has no footprint and fails. It used to be signed.
     */
    void testSignAttachesSignature()
    {
        StubSorobanServer srv;
        srv.simResponses = { makeSimulationResponse(10, Scv::toVoid()) };
        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        QScopedPointer<KeyPair> signer(sourceKeyPair());
        QVERIFY_EXCEPTION_THROWN(at.sign(signer.data()), qstellar::exception::NotYetSimulatedException);
        QVERIFY_EXCEPTION_THROWN(at.signAndSend(signer.data(), 5000, 1),
                                 qstellar::exception::NotYetSimulatedException);
        QVERIFY(at.getTransaction()->getSignatures().isEmpty());
        QVERIFY(srv.sentEnvelopes.isEmpty());

        at.simulate();
        AssembledTransaction& chained = at.sign(signer.data());
        QCOMPARE(&chained, &at);
        QCOMPARE(at.getTransaction()->getSignatures().size(), 1);
        QVERIFY(!at.getTransaction()->toEnvelopeXdrBase64().isEmpty());
    }
};

ADD_TEST(AssembledTransactionTest)
#endif // ASSEMBLEDTRANSACTIONTEST_H
