#ifndef SOROBANSERVERHELPERSTEST_H
#define SOROBANSERVERHELPERSTEST_H

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QtTest>
#include <cstring>
#include "../../testcollector.h"

#include "../../../src/sorobanserver.h"
#include "../../../src/sorobandatabuilder.h"
#include "../../../src/transactionbuilder.h"
#include "../../../src/transaction.h"
#include "../../../src/account.h"
#include "../../../src/keypair.h"
#include "../../../src/network.h"
#include "../../../src/asset.h"
#include "../../../src/invokehostfunctionoperation.h"
#include "../../../src/scval/scv.h"
#include "../../../src/stellaraddress.h"
#include "../../../src/util.h"

/** SorobanServer subclass that scripts `fetchTransaction` responses for tests.
 *  Declared at file scope (not nested) because moc dislikes class
 *  declarations inside a Q_OBJECT body. */
class ScriptedSorobanServer : public SorobanServer
{
public:
    ScriptedSorobanServer() : SorobanServer(QUrl("http://127.0.0.1:1/")) {}
    QList<GetTransactionResponse::Status> script;
    int idx = 0;
protected:
    GetTransactionResponse fetchTransaction(const QString& /*hash*/) override {
        QJsonObject o;
        const auto s = script.at(qMin(idx, int(script.size() - 1)));
        switch (s) {
        case GetTransactionResponse::Status::NOT_FOUND: o.insert("status", QStringLiteral("NOT_FOUND")); break;
        case GetTransactionResponse::Status::SUCCESS:   o.insert("status", QStringLiteral("SUCCESS")); break;
        case GetTransactionResponse::Status::FAILED:    o.insert("status", QStringLiteral("FAILED")); break;
        }
        ++idx;
        return GetTransactionResponse::fromJson(o);
    }
};

/** Tests for Iter 4 helpers: prepareTransaction (via applyTransactionSimulation),
 *  pollTransaction, and the SAC balance utilities. */
class SorobanServerHelpersTest: public QObject
{
    Q_OBJECT

    static KeyPair* sourceKeyPair() {
        return KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
    }

    /** Encode any XDR-serializable value to a base64 string. */
    template<typename T>
    static QString xdrToBase64(const T& value) {
        QByteArray buf;
        QDataStream s(&buf, QIODevice::WriteOnly);
        s << value;
        return QString::fromLatin1(buf.toBase64(XDR_BASE64ENCODING));
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    // ── applyTransactionSimulation ─────────────────────────────────────

    void testApplySimulationBumpsFeeAndAttachesData()
    {
        Account* acc = new Account(sourceKeyPair(), 41);
        TransactionBuilder b(acc, Network::testnetNetwork());
        // Use the SAC-from-asset variant so we don't depend on a real contract id.
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        b.addOperation(
            InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(native));
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        QScopedPointer<Transaction> tx(b.build());
        const qint64 originalFee = tx->getFee();   // 100 * 1 op

        // Hand-build a SorobanTransactionData and feed it via JSON.
        stellar::SorobanTransactionData td =
            SorobanDataBuilder().setResources(123456u, 4096u, 2048u).setResourceFee(777).build();

        QJsonObject simJson;
        simJson.insert("latestLedger", 1);
        simJson.insert("transactionData", xdrToBase64(td));
        simJson.insert("minResourceFee", QStringLiteral("999"));
        SimulateTransactionResponse sim = SimulateTransactionResponse::fromJson(simJson);

        SorobanServer::applyTransactionSimulation(tx.data(), sim);

        QCOMPARE(tx->getFee(), originalFee + 999);
        QVERIFY(!tx->getSorobanData().isNull());
        QCOMPARE(tx->getSorobanData()->resources.instructions, quint32(123456));
        QCOMPARE(tx->getSorobanData()->resourceFee, qint64(777));

        delete acc;
    }

    void testApplySimulationFillsAuthOnlyWhenEmpty()
    {
        Account* acc = new Account(sourceKeyPair(), 42);
        TransactionBuilder b(acc, Network::testnetNetwork());
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        InvokeHostFunctionOperation* op =
            InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(native);
        b.addOperation(op);
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        QScopedPointer<Transaction> tx(b.build());

        // Craft a SorobanAuthorizationEntry with SOURCE_ACCOUNT credentials and a
        // minimal invocation. Easiest: round-trip the default constructed value.
        stellar::SorobanAuthorizationEntry authEntry;
        authEntry.credentials.type =
            stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_SOURCE_ACCOUNT;

        QJsonObject result0;
        result0.insert("xdr", QStringLiteral(""));
        QJsonArray auth;
        auth.append(xdrToBase64(authEntry));
        result0.insert("auth", auth);

        QJsonArray results;
        results.append(result0);

        QJsonObject simJson;
        simJson.insert("latestLedger", 1);
        simJson.insert("minResourceFee", QStringLiteral("0"));
        simJson.insert("results", results);
        SimulateTransactionResponse sim = SimulateTransactionResponse::fromJson(simJson);

        // Locate the invoke op in the built transaction (the builder takes ownership).
        InvokeHostFunctionOperation* builtOp =
            dynamic_cast<InvokeHostFunctionOperation*>(tx->getOperations().first());
        QVERIFY(builtOp);
        QVERIFY(builtOp->getAuth().isEmpty());

        SorobanServer::applyTransactionSimulation(tx.data(), sim);
        QCOMPARE(builtOp->getAuth().size(), 1);

        // Calling again should NOT overwrite existing auth.
        SorobanServer::applyTransactionSimulation(tx.data(), sim);
        QCOMPARE(builtOp->getAuth().size(), 1);

        delete acc;
    }

    // ── pollTransaction ────────────────────────────────────────────────

    void testPollTransactionResolvesOnSuccess()
    {
        ScriptedSorobanServer srv;
        srv.script = {
            GetTransactionResponse::Status::NOT_FOUND,
            GetTransactionResponse::Status::NOT_FOUND,
            GetTransactionResponse::Status::SUCCESS,
        };
        GetTransactionResponse r = srv.pollTransaction(QStringLiteral("hash"), 5000, 1);
        QCOMPARE(r.getStatus(), GetTransactionResponse::Status::SUCCESS);
        QCOMPARE(srv.idx, 3);
    }

    void testPollTransactionTimesOut()
    {
        ScriptedSorobanServer srv;
        srv.script = { GetTransactionResponse::Status::NOT_FOUND };
        bool threw = false;
        try {
            srv.pollTransaction(QStringLiteral("hash"), 30, 5);
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
    }

    // ── SAC balance utilities ─────────────────────────────────────────

    void testBuildSACBalanceLedgerKeyDeterministic()
    {
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        const QString accountId =
            QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG");
        const QString key = SorobanServer::buildSACBalanceLedgerKey(
            accountId, native, Network::testnetNetwork()->getNetworkPassphrase());
        QVERIFY(!key.isEmpty());

        // Same inputs → same key (deterministic).
        const QString again = SorobanServer::buildSACBalanceLedgerKey(
            accountId, native, Network::testnetNetwork()->getNetworkPassphrase());
        QCOMPARE(key, again);

        // Different network → different key (the SAC contract id differs).
        const QString other = SorobanServer::buildSACBalanceLedgerKey(
            accountId, native, Network::publicNetwork()->getNetworkPassphrase());
        QVERIFY(other != key);
    }

    void testParseSACBalanceRoundTrip()
    {
        // Build a synthetic LedgerEntry that wraps a ContractData entry whose
        // val is an SCMap {amount: i128(987654321), authorized: true, clawback: false}.
        QList<stellar::SCMapEntry> entries;
        {
            stellar::SCMapEntry e;
            e.key = Scv::toSymbol(QStringLiteral("amount"));
            e.val = Scv::toInt128Parts(0, 987654321ULL);
            entries.append(e);
        }
        {
            stellar::SCMapEntry e;
            e.key = Scv::toSymbol(QStringLiteral("authorized"));
            e.val = Scv::toBoolean(true);
            entries.append(e);
        }
        {
            stellar::SCMapEntry e;
            e.key = Scv::toSymbol(QStringLiteral("clawback"));
            e.val = Scv::toBoolean(false);
            entries.append(e);
        }
        stellar::SCVal valMap = Scv::toMap(entries);

        // Hand-roll the LedgerEntry XDR: lastModified + LedgerEntryType + ContractDataEntry
        QByteArray buf;
        QDataStream s(&buf, QIODevice::WriteOnly);
        s << quint32(12345);                                              // lastModifiedLedgerSeq
        s << static_cast<qint32>(stellar::LedgerEntryType::CONTRACT_DATA); // entry data discriminator
        stellar::SCAddress contract;
        contract.type = stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT;
        // dummy 32-byte contract id
        QByteArray cid(32, char(0xAB));
        std::memcpy(contract.contractId, cid.constData(), 32);
        s << contract;
        // key — placeholder Balance vec (content doesn't matter for parsing the val)
        stellar::SCVal placeholderKey = Scv::toSymbol(QStringLiteral("ignore"));
        s << placeholderKey;
        s << static_cast<qint32>(stellar::ContractDataDurability::PERSISTENT);
        s << valMap;
        // ext discriminator (0 = void) — parseSACBalanceFromLedgerEntryXdr doesn't read it,
        // but emitting it keeps the buffer well-formed.
        s << qint32(0);

        const QString base64 = QString::fromLatin1(buf.toBase64(XDR_BASE64ENCODING));
        SACBalance b = SorobanServer::parseSACBalanceFromLedgerEntryXdr(base64);
        QVERIFY(b.exists);
        QCOMPARE(b.authorized, true);
        QCOMPARE(b.clawbackEnabled, false);
        // amount is the low 64 bits big-endian = 987654321 → bytes [0]*8 + be(987654321)
        QCOMPARE(b.amount.size(), 16);
        // The low 8 bytes encode 987654321 big-endian.
        quint64 lo = 0;
        for (int i = 8; i < 16; ++i) lo = (lo << 8) | quint8(b.amount.at(i));
        QCOMPARE(lo, quint64(987654321ULL));
    }
};

ADD_TEST(SorobanServerHelpersTest)
#endif // SOROBANSERVERHELPERSTEST_H
