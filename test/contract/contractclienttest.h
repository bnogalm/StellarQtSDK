#ifndef CONTRACTCLIENTTEST_H
#define CONTRACTCLIENTTEST_H

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QtTest>
#include "../testcollector.h"

#include "../../src/contract/contractclient.h"
#include "../../src/contract/assembledtransaction.h"
#include "../../src/sorobanserver.h"
#include "../../src/sorobandatabuilder.h"
#include "../../src/transaction.h"
#include "../../src/account.h"
#include "../../src/keypair.h"
#include "../../src/network.h"
#include "../../src/invokehostfunctionoperation.h"
#include "../../src/scval/scv.h"
#include "../../src/stellaraddress.h"
#include "../../src/util.h"

/** Local stub. Duplicated from earlier test files so this header is self-contained. */
class StubCcSorobanServer : public SorobanServer
{
public:
    StubCcSorobanServer() : SorobanServer(QUrl("http://127.0.0.1:1/")) {}
    QList<SimulateTransactionResponse> simResponses;
    int simIdx = 0;
    SimulateTransactionResponse simulateTransactionXdr(const QString&) override {
        return simResponses.at(qMin(simIdx++, int(simResponses.size() - 1)));
    }
};

class ContractClientTest: public QObject
{
    Q_OBJECT

    static KeyPair* sourceKeyPair() {
        return KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
    }

    static const char* sacContractId() {
        // Canonical native SAC on testnet (verified against the 0.10.0 fixture).
        return "CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC";
    }

    template<typename T> static QString xdrToBase64(const T& value) {
        QByteArray buf;
        QDataStream s(&buf, QIODevice::WriteOnly);
        s << value;
        return QString::fromLatin1(buf.toBase64(XDR_BASE64ENCODING));
    }

    static SimulateTransactionResponse makeSimResponse(qint64 resourceFee, const stellar::SCVal& resultVal)
    {
        stellar::SorobanTransactionData td =
            SorobanDataBuilder().setResources(123456u, 4096u, 2048u).setResourceFee(resourceFee).build();
        QJsonObject simJson;
        simJson.insert("latestLedger", 1);
        simJson.insert("transactionData", xdrToBase64(td));
        simJson.insert("minResourceFee", QString::number(resourceFee));
        QJsonObject result0; result0.insert("xdr", xdrToBase64(resultVal));
        QJsonArray results; results.append(result0);
        simJson.insert("results", results);
        return SimulateTransactionResponse::fromJson(simJson);
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testCtorRejectsNonContractAddress()
    {
        StubCcSorobanServer srv;
        bool threw = false;
        try {
            // Pass a G-strkey instead of a C-strkey.
            ContractClient cc(QString(sourceKeyPair()->getAccountId()), &srv, Network::testnetNetwork());
        } catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    void testCtorAcceptsContractAddress()
    {
        StubCcSorobanServer srv;
        ContractClient cc(QString(sacContractId()), &srv, Network::testnetNetwork());
        QCOMPARE(cc.getContractAddress().getType(), StellarAddress::Type::CONTRACT);
        QCOMPARE(cc.getContractAddress().toString(), QString(sacContractId()));
    }

    void testInvokeBuildsAssembledTransactionAndSimulates()
    {
        StubCcSorobanServer srv;
        srv.simResponses = { makeSimResponse(999, Scv::toUint32(7)) };

        ContractClient cc(QString(sacContractId()), &srv, Network::testnetNetwork());
        Account* acc = new Account(sourceKeyPair(), 41);

        AssembledTransaction* at = cc.invoke(
            QStringLiteral("decimals"), QList<stellar::SCVal>{}, acc, /*baseFee*/ 100);
        QVERIFY(at != nullptr);
        QVERIFY(at->isSimulated());
        // After simulate the tx's fee bumped by minResourceFee=999.
        QCOMPARE(at->getTransaction()->getFee(), qint64(100 + 999));
        // The op should be an InvokeHostFunctionOperation targeting our contract.
        auto* op = dynamic_cast<InvokeHostFunctionOperation*>(at->getTransaction()->getOperations().first());
        QVERIFY(op != nullptr);
        QCOMPARE(op->getHostFunction().type,
                 stellar::HostFunctionType::HOST_FUNCTION_TYPE_INVOKE_CONTRACT);

        // result() returns the SCVal we stubbed in.
        QCOMPARE(Scv::fromUint32(at->result()), quint32(7));
        delete at;
        delete acc;
    }

    void testReadReturnsResultSCValAndDiscardsTx()
    {
        StubCcSorobanServer srv;
        srv.simResponses = { makeSimResponse(0, Scv::toUint64(42)) };

        ContractClient cc(QString(sacContractId()), &srv, Network::testnetNetwork());
        Account* acc = new Account(sourceKeyPair(), 41);
        stellar::SCVal r = cc.read(QStringLiteral("balance"), QList<stellar::SCVal>{}, acc);
        QCOMPARE(r.type, stellar::SCValType::SCV_U64);
        QCOMPARE(Scv::fromUint64(r), quint64(42));
        delete acc;
    }
};

ADD_TEST(ContractClientTest)
#endif // CONTRACTCLIENTTEST_H
