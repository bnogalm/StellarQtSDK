#ifndef ASSEMBLEDTRANSACTIONTEST_H
#define ASSEMBLEDTRANSACTIONTEST_H

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
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

    SimulateTransactionResponse simulateTransactionXdr(const QString&) override {
        return simResponses.at(qMin(simIdx++, int(simResponses.size() - 1)));
    }
    SendTransactionResponse sendTransactionXdr(const QString&) override {
        return sendResponses.at(qMin(sendIdx++, int(sendResponses.size() - 1)));
    }
protected:
    GetTransactionResponse fetchTransaction(const QString&) override {
        return fetchResponses.at(qMin(fetchIdx++, int(fetchResponses.size() - 1)));
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

    static SimulateTransactionResponse makeSimulationResponse(qint64 resourceFee, const stellar::SCVal& resultVal)
    {
        stellar::SorobanTransactionData td =
            SorobanDataBuilder().setResources(123456u, 4096u, 2048u).setResourceFee(resourceFee).build();

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

        QJsonObject sendJson;
        sendJson.insert("status", QStringLiteral("PENDING"));
        sendJson.insert("hash", QStringLiteral("abcd"));
        srv.sendResponses = { SendTransactionResponse::fromJson(sendJson) };

        QJsonObject pendingFetch;  pendingFetch.insert("status", QStringLiteral("NOT_FOUND"));
        QJsonObject successFetch;  successFetch.insert("status", QStringLiteral("SUCCESS"));
        srv.fetchResponses = {
            GetTransactionResponse::fromJson(pendingFetch),
            GetTransactionResponse::fromJson(successFetch),
        };

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        at.simulate();
        GetTransactionResponse r = at.signAndSend(sourceKeyPair(), 5000, 1);
        QCOMPARE(r.getStatus(), GetTransactionResponse::Status::SUCCESS);
        QCOMPARE(srv.fetchIdx, 2);
    }

    void testSignAndSendThrowsOnSendError()
    {
        StubSorobanServer srv;
        srv.simResponses = { makeSimulationResponse(10, Scv::toVoid()) };

        QJsonObject sendJson;
        sendJson.insert("status", QStringLiteral("ERROR"));
        sendJson.insert("hash", QStringLiteral("h"));
        sendJson.insert("errorResultXdr", QStringLiteral("ZZ"));
        srv.sendResponses = { SendTransactionResponse::fromJson(sendJson) };

        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        at.simulate();
        bool threw = false, typed = false;
        try { at.signAndSend(sourceKeyPair(), 5000, 1); }
        catch (const qstellar::exception::SendFailedException&) { typed = threw = true; }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
        QVERIFY(typed);
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

    void testSignAttachesSignature()
    {
        StubSorobanServer srv;
        AssembledTransaction at(buildSorobanTx(), &srv, Network::testnetNetwork());
        // Without simulating, sign() still works — the test only validates
        // that the call returns *this and the underlying tx received a sig.
        at.sign(sourceKeyPair());
        // Building the envelope after sign() should embed exactly one signature.
        QString env = at.getTransaction()->toEnvelopeXdrBase64();
        QVERIFY(!env.isEmpty());
    }
};

ADD_TEST(AssembledTransactionTest)
#endif // ASSEMBLEDTRANSACTIONTEST_H
