#ifndef SEP45CHALLENGETEST_H
#define SEP45CHALLENGETEST_H

#include <QJsonObject>
#include <QObject>
#include <QtTest>
#include "testcollector.h"

#include "../src/sep45challenge.h"
#include "../src/exception/invalidsep45challengeexception.h"
#include "../src/sorobanserver.h"
#include "../src/abstracttransaction.h"
#include "../src/transaction.h"
#include "../src/keypair.h"
#include "../src/network.h"
#include "../src/invokehostfunctionoperation.h"
#include "../src/scval/scv.h"
#include "../src/stellaraddress.h"
#include "../src/util.h"

/** Stub server used to exercise the simulate step of verifyChallenge.
 *  Declared at file scope to keep moc happy. */
class StubSep45SorobanServer : public SorobanServer
{
public:
    StubSep45SorobanServer() : SorobanServer(QUrl("http://127.0.0.1:1/")) {}
    SimulateTransactionResponse fixedSim;
    SimulateTransactionResponse simulateTransactionXdr(const QString&) override {
        return fixedSim;
    }
};

class Sep45ChallengeTest: public QObject
{
    Q_OBJECT

    static KeyPair* serverSigner() {
        return KeyPair::fromSecretSeed(QString("SC4CGETADVYTCR5HEAVZRB3DZQY5Y4J7RFNJTRA6ESMHIPEZUSTE2QDK"));
    }

    static const char* clientContract() {
        // Canonical native SAC on testnet — a real C-strkey we can reuse.
        return "CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC";
    }
    static const char* webAuthContract() {
        return "CAS3J7GYLGXMF6TDJBBYYSE3HQ6BBSMLNUQ34T6TZMYMW2EVH34XOWMA";
    }

    static QByteArray fixedNonce() {
        // 48-byte deterministic nonce so test envelopes are stable.
        QByteArray b(48, char(0));
        for (int i = 0; i < 48; ++i) b[i] = char(i);
        return b;
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    void testNewChallengeBuildsParsableEnvelope()
    {
        QString xdr = Sep45Challenge::newChallenge(
            serverSigner(), Network::testnetNetwork(),
            clientContract(), webAuthContract(),
            QStringLiteral("web_auth_verify"),
            QStringLiteral("example.com"),
            QStringLiteral("auth.example.com"),
            /*timeoutSec*/ 300,
            fixedNonce());

        // The envelope must round-trip as a v1 Transaction with one
        // InvokeHostFunction op carrying one auth entry.
        AbstractTransaction* atx = AbstractTransaction::fromEnvelopeXdr(
            xdr, Network::testnetNetwork());
        QScopedPointer<Transaction> tx(dynamic_cast<Transaction*>(atx));
        QVERIFY(tx);
        QCOMPARE(tx->getSourceAccount(), QString(serverSigner()->getAccountId()));
        QCOMPARE(tx->getOperations().size(), 1);

        auto* op = dynamic_cast<InvokeHostFunctionOperation*>(tx->getOperations().first());
        QVERIFY(op);
        QCOMPARE(op->getHostFunction().type,
                 stellar::HostFunctionType::HOST_FUNCTION_TYPE_INVOKE_CONTRACT);
        // Function name + args[0] (client address) + args[3] (nonce).
        QCOMPARE(QString::fromUtf8(op->getHostFunction().invokeContract.functionName),
                 QString("web_auth_verify"));
        QCOMPARE(op->getHostFunction().invokeContract.args.size(), 5);
        StellarAddress arg0 = Scv::fromAddress(op->getHostFunction().invokeContract.args.at(0));
        QCOMPARE(arg0.toString(), QString(clientContract()));
        QByteArray nonce = Scv::fromBytes(op->getHostFunction().invokeContract.args.at(3));
        QCOMPARE(nonce, fixedNonce());

        // Auth list has one unsigned ADDRESS-credentials entry referencing
        // the client contract.
        QCOMPARE(op->getAuth().size(), 1);
        const auto& entry = op->getAuth().first();
        QCOMPARE(entry.credentials.type,
                 stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS);
        QCOMPARE(entry.credentials.address.address.type,
                 stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT);
        QCOMPARE(entry.credentials.address.signature.type,
                 stellar::SCValType::SCV_VOID);  // unsigned marker
    }

    void testNewChallengeRejectsNonContractClient()
    {
        bool threw = false;
        try {
            Sep45Challenge::newChallenge(
                serverSigner(), Network::testnetNetwork(),
                serverSigner()->getAccountId(),  // G-strkey — invalid
                webAuthContract(),
                QStringLiteral("web_auth_verify"),
                QStringLiteral("example.com"),
                QStringLiteral("auth.example.com"));
        } catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    void testNewChallengeRejectsZeroTimeout()
    {
        bool threw = false;
        try {
            Sep45Challenge::newChallenge(
                serverSigner(), Network::testnetNetwork(),
                clientContract(), webAuthContract(),
                QStringLiteral("web_auth_verify"),
                QStringLiteral("example.com"),
                QStringLiteral("auth.example.com"),
                /*timeoutSec*/ 0,
                fixedNonce());
        } catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    void testNewChallengeRejectsBadNonceLength()
    {
        bool threw = false;
        try {
            Sep45Challenge::newChallenge(
                serverSigner(), Network::testnetNetwork(),
                clientContract(), webAuthContract(),
                QStringLiteral("web_auth_verify"),
                QStringLiteral("example.com"),
                QStringLiteral("auth.example.com"),
                300,
                QByteArray(32, char(0)));  // wrong size
        } catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
    }

    // ── verifyChallenge ───────────────────────────────────────────────

    QString validChallenge(const QString& homeDomain = QStringLiteral("example.com"),
                           const QString& webAuthDomain = QStringLiteral("auth.example.com"))
    {
        return Sep45Challenge::newChallenge(
            serverSigner(), Network::testnetNetwork(),
            clientContract(), webAuthContract(),
            QStringLiteral("web_auth_verify"),
            homeDomain, webAuthDomain,
            300, fixedNonce());
    }

    void testVerifyAcceptsValidChallenge()
    {
        QString xdr = validChallenge();
        auto v = Sep45Challenge::verifyChallenge(
            xdr, serverSigner()->getAccountId(), Network::testnetNetwork(),
            /*sorobanServer*/ nullptr,
            QStringList{QStringLiteral("example.com")},
            QStringLiteral("auth.example.com"));
        QCOMPARE(v.clientAccountId, QString(clientContract()));
        QCOMPARE(v.matchedHomeDomain, QString("example.com"));
        QCOMPARE(v.nonce, fixedNonce());
        QVERIFY(v.expirationUnix > 0);
    }

    void testVerifyRejectsWrongServerAccountId()
    {
        QString xdr = validChallenge();
        // KeyPair::random() avoids fabricating a strkey with a bad checksum.
        KeyPair* other = KeyPair::random();
        bool threw = false;
        try {
            Sep45Challenge::verifyChallenge(
                xdr, other->getAccountId(), Network::testnetNetwork(),
                nullptr,
                QStringList{QStringLiteral("example.com")},
                QStringLiteral("auth.example.com"));
        } catch (const qstellar::exception::InvalidSep45ChallengeException&) { threw = true; }
        QVERIFY(threw);
        delete other;
    }

    void testVerifyRejectsWrongWebAuthDomain()
    {
        QString xdr = validChallenge();
        bool threw = false;
        try {
            Sep45Challenge::verifyChallenge(
                xdr, serverSigner()->getAccountId(), Network::testnetNetwork(),
                nullptr,
                QStringList{QStringLiteral("example.com")},
                QStringLiteral("wrong.example.com"));
        } catch (const qstellar::exception::InvalidSep45ChallengeException&) { threw = true; }
        QVERIFY(threw);
    }

    void testVerifyRejectsHomeDomainNotInList()
    {
        QString xdr = validChallenge();
        bool threw = false;
        try {
            Sep45Challenge::verifyChallenge(
                xdr, serverSigner()->getAccountId(), Network::testnetNetwork(),
                nullptr,
                QStringList{QStringLiteral("other.com")},
                QStringLiteral("auth.example.com"));
        } catch (const qstellar::exception::InvalidSep45ChallengeException&) { threw = true; }
        QVERIFY(threw);
    }

    void testVerifyAcceptsAnyHomeDomainInList()
    {
        QString xdr = validChallenge(QStringLiteral("two.example.com"));
        auto v = Sep45Challenge::verifyChallenge(
            xdr, serverSigner()->getAccountId(), Network::testnetNetwork(), nullptr,
            QStringList{QStringLiteral("one.example.com"), QStringLiteral("two.example.com")},
            QStringLiteral("auth.example.com"));
        QCOMPARE(v.matchedHomeDomain, QString("two.example.com"));
    }

    void testVerifyFailsWhenSimulationReturnsError()
    {
        QString xdr = validChallenge();
        StubSep45SorobanServer srv;
        QJsonObject simJson;
        simJson.insert("latestLedger", 1);
        simJson.insert("error", QStringLiteral("HostError: __check_auth rejected"));
        srv.fixedSim = SimulateTransactionResponse::fromJson(simJson);

        bool threw = false;
        try {
            Sep45Challenge::verifyChallenge(
                xdr, serverSigner()->getAccountId(), Network::testnetNetwork(), &srv,
                QStringList{QStringLiteral("example.com")},
                QStringLiteral("auth.example.com"));
        } catch (const qstellar::exception::InvalidSep45ChallengeException&) { threw = true; }
        QVERIFY(threw);
    }
};

ADD_TEST(Sep45ChallengeTest)
#endif // SEP45CHALLENGETEST_H
