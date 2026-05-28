#ifndef ASSEMBLEDTRANSACTIONAUTHTEST_H
#define ASSEMBLEDTRANSACTIONAUTHTEST_H

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QtTest>
#include <cstring>
#include "../testcollector.h"

#include "../../src/contract/assembledtransaction.h"
#include "../../src/contract/sorobancredentialssigner.h"
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
#include "../../src/stellaraddress.h"
#include "../../src/util.h"

// Reuses StubSorobanServer declared in assembledtransactiontest.h. To avoid
// double-definition / linkage issues, this test file declares a small local
// variant rather than pulling that header in.
class StubAuthSorobanServer : public SorobanServer
{
public:
    StubAuthSorobanServer() : SorobanServer(QUrl("http://127.0.0.1:1/")) {}
    QList<SimulateTransactionResponse> simResponses;
    QList<SendTransactionResponse> sendResponses;
    QList<GetTransactionResponse> fetchResponses;
    int simIdx = 0, sendIdx = 0, fetchIdx = 0;
    QStringList capturedSendEnvelopes;

    SimulateTransactionResponse simulateTransactionXdr(const QString&) override {
        return simResponses.at(qMin(simIdx++, int(simResponses.size() - 1)));
    }
    SendTransactionResponse sendTransactionXdr(const QString& env) override {
        capturedSendEnvelopes.append(env);
        return sendResponses.at(qMin(sendIdx++, int(sendResponses.size() - 1)));
    }
protected:
    GetTransactionResponse fetchTransaction(const QString&) override {
        return fetchResponses.at(qMin(fetchIdx++, int(fetchResponses.size() - 1)));
    }
};

class AssembledTransactionAuthTest: public QObject
{
    Q_OBJECT

    static KeyPair* sourceKeyPair() {
        return KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
    }

    /** Build a Soroban tx with a custom set of pre-attached auth entries on
     *  its InvokeHostFunctionOperation. */
    static Transaction* buildTxWithAuth(const QList<stellar::SorobanAuthorizationEntry>& auth)
    {
        Account* acc = new Account(sourceKeyPair(), 41);
        TransactionBuilder b(acc, Network::testnetNetwork());
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        InvokeHostFunctionOperation* op =
            InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(native);
        op->setAuth(auth);
        b.addOperation(op);
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        Transaction* tx = b.build();
        delete acc;
        return tx;
    }

    template<typename T> static QString xdrToBase64(const T& value) {
        QByteArray buf;
        QDataStream s(&buf, QIODevice::WriteOnly);
        s << value;
        return QString::fromLatin1(buf.toBase64(XDR_BASE64ENCODING));
    }

    static SimulateTransactionResponse makeSimResponse(qint64 resourceFee, quint32 latestLedger = 1000)
    {
        stellar::SorobanTransactionData td =
            SorobanDataBuilder().setResources(123456u, 4096u, 2048u).setResourceFee(resourceFee).build();
        QJsonObject simJson;
        simJson.insert("latestLedger", int(latestLedger));
        simJson.insert("transactionData", xdrToBase64(td));
        simJson.insert("minResourceFee", QString::number(resourceFee));
        QJsonObject result0; result0.insert("xdr", xdrToBase64(Scv::toVoid()));
        QJsonArray results; results.append(result0);
        simJson.insert("results", results);
        return SimulateTransactionResponse::fromJson(simJson);
    }

    static stellar::SorobanAuthorizationEntry unsignedAddressEntry(const QByteArray& accountPub)
    {
        stellar::SorobanAuthorizationEntry entry;
        entry.credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS;
        entry.credentials.address.address.type = stellar::SCAddressType::SC_ADDRESS_TYPE_ACCOUNT;
        std::memcpy(entry.credentials.address.address.accountId, accountPub.constData(), 32);
        entry.credentials.address.nonce = qint64(0xAABB);
        entry.credentials.address.signatureExpirationLedger = 0;
        // signature is SCV_VOID by default — the "needs signing" marker.
        entry.rootInvocation.function.type =
            stellar::SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CONTRACT_FN;
        return entry;
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    // ── needsNonInvokerSigningBy ──────────────────────────────────────

    void testNeedsSigningListsAddressEntries()
    {
        // The "other" account doesn't need to be a real KeyPair — we only use
        // its 32-byte pub key as a marker. Synthesize one.
        QByteArray otherPub(32, char(0x42));
        QList<stellar::SorobanAuthorizationEntry> auth;
        auth.append(unsignedAddressEntry(sourceKeyPair()->getPublicKey()));
        auth.append(unsignedAddressEntry(otherPub));
        // SOURCE_ACCOUNT entries don't show up.
        stellar::SorobanAuthorizationEntry srcOnly;
        srcOnly.credentials.type =
            stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_SOURCE_ACCOUNT;
        auth.append(srcOnly);

        StubAuthSorobanServer srv;
        srv.simResponses = { makeSimResponse(0) };
        AssembledTransaction at(buildTxWithAuth(auth), &srv, Network::testnetNetwork());
        at.simulate();

        QStringList needs = at.needsNonInvokerSigningBy();
        QCOMPARE(needs.size(), 2);
        StellarAddress a1(StellarAddress::Type::ACCOUNT, sourceKeyPair()->getPublicKey());
        StellarAddress a2(StellarAddress::Type::ACCOUNT, otherPub);
        QVERIFY(needs.contains(a1.toString()));
        QVERIFY(needs.contains(a2.toString()));
    }

    void testNeedsSigningSkipsAlreadySigned()
    {
        // Pre-sign one entry so it's no longer SCV_VOID.
        stellar::SorobanAuthorizationEntry signed_ = unsignedAddressEntry(sourceKeyPair()->getPublicKey());
        signed_ = SorobanCredentialsSigner::authorizeEntry(
            signed_, sourceKeyPair(), 9999,
            Network::testnetNetwork()->getNetworkPassphrase());

        QList<stellar::SorobanAuthorizationEntry> auth;
        auth.append(signed_);

        StubAuthSorobanServer srv;
        srv.simResponses = { makeSimResponse(0) };
        AssembledTransaction at(buildTxWithAuth(auth), &srv, Network::testnetNetwork());
        at.simulate();
        QCOMPARE(at.needsNonInvokerSigningBy().size(), 0);
    }

    // ── signAuthEntries ───────────────────────────────────────────────

    void testSignAuthEntriesOnlyMatchesSigner()
    {
        // "other" is a synthetic 32-byte pub key — it just has to be different
        // from sourceKeyPair's pub key.
        QByteArray otherPub(32, char(0x42));
        QList<stellar::SorobanAuthorizationEntry> auth;
        auth.append(unsignedAddressEntry(sourceKeyPair()->getPublicKey()));
        auth.append(unsignedAddressEntry(otherPub));

        StubAuthSorobanServer srv;
        srv.simResponses = { makeSimResponse(0) };
        AssembledTransaction at(buildTxWithAuth(auth), &srv, Network::testnetNetwork());
        at.simulate();

        at.signAuthEntries(sourceKeyPair(), 12345);

        // After signing, only the source's entry should be signed; the other
        // remains a placeholder.
        const auto& finalAuth =
            dynamic_cast<InvokeHostFunctionOperation*>(
                at.getTransaction()->getOperations().first())->getAuth();
        QCOMPARE(finalAuth.size(), 2);
        // Find the entry whose accountId matches sourceKeyPair.
        for (const auto& e : finalAuth) {
            const bool isSource =
                std::memcmp(e.credentials.address.address.accountId,
                            sourceKeyPair()->getPublicKey().constData(), 32) == 0;
            if (isSource) {
                QCOMPARE(e.credentials.address.signature.type, stellar::SCValType::SCV_VEC);
                QCOMPARE(e.credentials.address.signatureExpirationLedger, quint32(12345));
            } else {
                QCOMPARE(e.credentials.address.signature.type, stellar::SCValType::SCV_VOID);
            }
        }
        // needsNonInvokerSigningBy() now reports only the other key.
        QCOMPARE(at.needsNonInvokerSigningBy().size(), 1);
    }

    void testSignAuthEntriesDefaultsValidUntilLedger()
    {
        QList<stellar::SorobanAuthorizationEntry> auth;
        auth.append(unsignedAddressEntry(sourceKeyPair()->getPublicKey()));
        StubAuthSorobanServer srv;
        // latestLedger = 5000 → default validUntilLedger = 5100.
        srv.simResponses = { makeSimResponse(0, 5000) };
        AssembledTransaction at(buildTxWithAuth(auth), &srv, Network::testnetNetwork());
        at.simulate();
        at.signAuthEntries(sourceKeyPair() /* validUntilLedger=0 default */);

        const auto& finalAuth =
            dynamic_cast<InvokeHostFunctionOperation*>(
                at.getTransaction()->getOperations().first())->getAuth();
        QCOMPARE(finalAuth.first().credentials.address.signatureExpirationLedger, quint32(5100));
    }

    // ── restoreFootprint ─────────────────────────────────────────────

    void testRestoreFootprintBuildsSendsAndPolls()
    {
        // Simulation reports needsRestore.
        QJsonObject simJson;
        simJson.insert("latestLedger", 1000);
        simJson.insert("transactionData", QStringLiteral(""));
        simJson.insert("minResourceFee", QStringLiteral("0"));
        QJsonObject result0; result0.insert("xdr", xdrToBase64(Scv::toVoid()));
        QJsonArray results; results.append(result0);
        simJson.insert("results", results);

        // restorePreamble carries its own SorobanTransactionData.
        stellar::SorobanTransactionData restoreData =
            SorobanDataBuilder().setResources(100u, 100u, 100u).setResourceFee(qint64(555)).build();
        QJsonObject rp;
        rp.insert("transactionData", xdrToBase64(restoreData));
        rp.insert("minResourceFee", QStringLiteral("555"));
        simJson.insert("restorePreamble", rp);

        StubAuthSorobanServer srv;
        srv.simResponses = { SimulateTransactionResponse::fromJson(simJson) };

        // sendTransaction returns PENDING; pollTransaction sees SUCCESS first.
        QJsonObject sendJson;
        sendJson.insert("status", QStringLiteral("PENDING"));
        sendJson.insert("hash", QStringLiteral("rh"));
        srv.sendResponses = { SendTransactionResponse::fromJson(sendJson) };
        QJsonObject ok; ok.insert("status", QStringLiteral("SUCCESS"));
        srv.fetchResponses = { GetTransactionResponse::fromJson(ok) };

        QList<stellar::SorobanAuthorizationEntry> noAuth;
        AssembledTransaction at(buildTxWithAuth(noAuth), &srv, Network::testnetNetwork());
        at.simulate();
        QVERIFY(at.getSimulationResponse().needsRestore());

        Account* sourceAccount = new Account(sourceKeyPair(), 99);
        GetTransactionResponse r = at.restoreFootprint(sourceAccount, sourceKeyPair(),
                                                       /*baseFee*/ 100,
                                                       /*timeoutMs*/ 5000,
                                                       /*intervalMs*/ 1);
        QCOMPARE(r.getStatus(), GetTransactionResponse::Status::SUCCESS);
        QVERIFY(!srv.capturedSendEnvelopes.isEmpty());
        delete sourceAccount;
    }

    void testRestoreFootprintThrowsWhenNoPreamble()
    {
        StubAuthSorobanServer srv;
        srv.simResponses = { makeSimResponse(0) };  // no restorePreamble
        QList<stellar::SorobanAuthorizationEntry> noAuth;
        AssembledTransaction at(buildTxWithAuth(noAuth), &srv, Network::testnetNetwork());
        at.simulate();

        Account* sourceAccount = new Account(sourceKeyPair(), 99);
        bool threw = false;
        try {
            at.restoreFootprint(sourceAccount, sourceKeyPair(), 100, 5000, 1);
        } catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
        delete sourceAccount;
    }
};

ADD_TEST(AssembledTransactionAuthTest)
#endif // ASSEMBLEDTRANSACTIONAUTHTEST_H
