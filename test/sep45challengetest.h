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

    /** Rewrite an issued challenge's single auth entry, the way a client does
     *  when it completes the challenge, and return the new envelope. */
    static QString withCompletedAuthEntry(const QString& challengeXdr,
                                          quint32 expirationLedger,
                                          qint64 nonceDelta = 0)
    {
        QScopedPointer<AbstractTransaction> atx(
            AbstractTransaction::fromEnvelopeXdr(challengeXdr, Network::testnetNetwork()));
        auto* tx = dynamic_cast<Transaction*>(atx.data());
        auto* op = dynamic_cast<InvokeHostFunctionOperation*>(tx->getOperations().at(0));
        QList<stellar::SorobanAuthorizationEntry> auth = op->getAuth();
        auth[0].credentials.address.signatureExpirationLedger = expirationLedger;
        auth[0].credentials.address.signature = Scv::toUint32(7);  // stand-in client signature
        auth[0].credentials.address.nonce += nonceDelta;
        op->setAuth(auth);
        return tx->toEnvelopeXdrBase64();
    }

    /** The real SEP-45 flow: the server signs the envelope BEFORE the client
     *  fills in the auth entry, and those fields sit inside the operation body,
     *  so the completed envelope hashes differently. Verification must check
     *  the server signature against the as-issued form or it rejects every
     *  genuine client response. */
    void testVerifyAcceptsClientCompletedChallenge()
    {
        QString completed = withCompletedAuthEntry(validChallenge(), 123456);
        auto v = Sep45Challenge::verifyChallenge(
            completed, serverSigner()->getAccountId(), Network::testnetNetwork(),
            nullptr,
            QStringList{QStringLiteral("example.com")},
            QStringLiteral("auth.example.com"));
        QCOMPARE(v.clientAccountId, QString(clientContract()));
    }

    /** ...but resetting the client-owned fields must not blind the check to
     *  anything else in the entry: the nonce is still covered. */
    void testVerifyRejectsTamperedAuthNonce()
    {
        QString tampered = withCompletedAuthEntry(validChallenge(), 123456, /*nonceDelta*/ 1);
        bool threw = false;
        try {
            Sep45Challenge::verifyChallenge(
                tampered, serverSigner()->getAccountId(), Network::testnetNetwork(),
                nullptr,
                QStringList{QStringLiteral("example.com")},
                QStringLiteral("auth.example.com"));
        } catch (const qstellar::exception::InvalidSep45ChallengeException&) { threw = true; }
        QVERIFY(threw);
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

    void testVerifyRejectsForgedServerSignature()
    {
        // A challenge whose source account IS the server but whose envelope
        // signature comes from a DIFFERENT key must be rejected. The earlier
        // checks (source account, args, domains, auth entry) all pass — and
        // simulate validates only the CLIENT's auth chain — so this is the
        // case the local server-signature check exists to catch. Without it,
        // an attacker who sets the server's G-address as the source and signs
        // with their own key would slip a forged challenge through.
        QString xdr = validChallenge();
        QScopedPointer<Transaction> tx(dynamic_cast<Transaction*>(
            AbstractTransaction::fromEnvelopeXdr(xdr, Network::testnetNetwork())));
        QVERIFY(tx);

        // Swap the genuine server signature for one from an unrelated key,
        // signed over the same tx hash (structurally valid, wrong signer).
        QScopedPointer<KeyPair> attacker(KeyPair::random());
        stellar::TransactionEnvelope env = tx->toEnvelopeXdr();
        env.v1.signatures.clear();
        env.v1.signatures.append(attacker->signDecorated(tx->hash()));

        QScopedPointer<Transaction> forged(dynamic_cast<Transaction*>(
            AbstractTransaction::fromEnvelopeXdr(env, Network::testnetNetwork())));
        QVERIFY(forged);
        QString forgedXdr = forged->toEnvelopeXdrBase64();

        bool threw = false;
        try {
            Sep45Challenge::verifyChallenge(
                forgedXdr, serverSigner()->getAccountId(), Network::testnetNetwork(),
                /*sorobanServer*/ nullptr,
                QStringList{QStringLiteral("example.com")},
                QStringLiteral("auth.example.com"));
        } catch (const qstellar::exception::InvalidSep45ChallengeException&) { threw = true; }
        QVERIFY(threw);
    }
};

ADD_TEST(Sep45ChallengeTest)
#endif // SEP45CHALLENGETEST_H
