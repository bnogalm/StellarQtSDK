#ifndef SEP45CHALLENGETEST_H
#define SEP45CHALLENGETEST_H

#include <QJsonArray>
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
#include "../src/account.h"
#include "../src/strkey.h"
#include "../src/contract/sorobancredentialssigner.h"
#include <QDateTime>
#include <QScopedPointer>
#include <cstring>
#include <algorithm>
#include <functional>
#include <limits>

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

/** Answers every simulation with `sim` and keeps each envelope it was asked
 *  to simulate, so a test can compare them with py's. */
class CapturingSep45SorobanServer : public SorobanServer
{
public:
    CapturingSep45SorobanServer() : SorobanServer(QUrl("http://127.0.0.1:1/")) {}
    SimulateTransactionResponse sim;
    QStringList requests;
    SimulateTransactionResponse simulateTransactionXdr(const QString& envelope) override {
        requests.append(envelope);
        return sim;
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

    static QString serverId() {
        QScopedPointer<KeyPair> server(serverSigner());
        return server->getAccountId();
    }

    static qint64 now() { return QDateTime::currentMSecsSinceEpoch() / 1000; }

    static QString contractStrKey(char fill) { return StrKey::encodeContract(QByteArray(32, fill)); }

    /** The five web_auth_verify arguments newChallenge writes. */
    static QList<stellar::SCVal> challengeArgs(quint64 expiration,
                                               const QString& homeDomain = QStringLiteral("example.com"),
                                               const QString& webAuthDomain = QStringLiteral("auth.example.com"))
    {
        return QList<stellar::SCVal>() << Scv::toAddress(QString(clientContract()))
                                       << Scv::toString(homeDomain) << Scv::toString(webAuthDomain)
                                       << Scv::toBytes(fixedNonce()) << Scv::toUint64(expiration);
    }

    static stellar::SorobanAuthorizedInvocation invocationOf(const QString& contract, const QByteArray& function,
                                                             const QList<stellar::SCVal>& args)
    {
        stellar::SorobanAuthorizedInvocation inv;
        inv.function.type = stellar::SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CONTRACT_FN;
        inv.function.contractFn.contractAddress = Scv::toAddress(contract).address;
        inv.function.contractFn.functionName = function;
        inv.function.contractFn.args = args;
        return inv;
    }

    /**
     * A challenge built the way newChallenge builds one, signed by the server,
     * but with the host call, the client entry's invocation and the time
     * bounds chosen here: what a server that is not the one expected can send.
     * `withTimeBounds` false leaves the transaction with none at all.
     */
    static QString handMade(const QString& hfContract, const QString& hfFunction,
                            const QList<stellar::SCVal>& hfArgs,
                            const stellar::SorobanAuthorizedInvocation& root,
                            qint64 minTime, qint64 maxTime, bool withTimeBounds = true)
    {
        InvokeHostFunctionOperation* op = InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
            StellarAddress::fromString(hfContract), hfFunction, hfArgs);
        stellar::SorobanAuthorizationEntry entry;
        entry.credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS;
        entry.credentials.address.address.type = stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT;
        std::memcpy(entry.credentials.address.address.contractId,
                    StellarAddress::fromString(clientContract()).getBytes().constData(), 32);
        entry.credentials.address.nonce = 424242;
        entry.rootInvocation = root;
        op->setAuth({entry});

        QScopedPointer<KeyPair> server(serverSigner());
        Account account(new KeyPair(*server), -1);
        TransactionBuilder builder(&account, Network::testnetNetwork());
        builder.addOperation(op).setBaseFee(100);
        if (withTimeBounds)
            builder.addTimeBounds(new TimeBounds(minTime, maxTime));
        else
            builder.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);   // {0, 0}: cleared below
        QScopedPointer<Transaction> tx(builder.build());
        if (!withTimeBounds) {
            // An infinite timeout is time bounds {0, 0}, as in js and java, and
            // no builder call leaves a transaction without any: take them out
            // of the envelope and read it back.
            stellar::TransactionEnvelope envelope = tx->toEnvelopeXdr();
            envelope.v1.tx.cond.type = stellar::PreconditionType::PRECOND_NONE;
            QByteArray bytes;
            QDataStream stream(&bytes, QIODevice::WriteOnly);
            stream << envelope;
            tx.reset(static_cast<Transaction*>(AbstractTransaction::fromEnvelopeXdr(
                QString::fromLatin1(bytes.toBase64()), Network::testnetNetwork())));
        }
        tx->sign(server.data());
        return tx->toEnvelopeXdrBase64();
    }

    /** A well-formed challenge whose expiration and time bounds are chosen here. */
    static QString withTimes(quint64 expiration, qint64 minTime, qint64 maxTime, bool withTimeBounds = true)
    {
        const QList<stellar::SCVal> args = challengeArgs(expiration);
        return handMade(webAuthContract(), QStringLiteral("web_auth_verify"), args,
                        invocationOf(webAuthContract(), "web_auth_verify", args),
                        minTime, maxTime, withTimeBounds);
    }

    /** Why verifyChallenge refused `xdr` with the usual expectations; empty if it accepted it. */
    static QString refusal(const QString& xdr,
                           const QStringList& homeDomains = QStringList{QStringLiteral("example.com")},
                           const QString& webAuthDomain = QStringLiteral("auth.example.com"))
    {
        try {
            Sep45Challenge::verifyChallenge(xdr, serverId(), Network::testnetNetwork(), nullptr,
                                            homeDomains, webAuthDomain);
            return QString();
        } catch (const qstellar::exception::InvalidSep45ChallengeException& e) {
            return QString::fromUtf8(e.what());
        }
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

    /**
     * The client's auth entry is what its key signs. Only its address was
     * checked, so a server-signed challenge whose entry authorized
     * token.transfer(client, attacker, ...), or web_auth_verify with a transfer
     * nested under it, verified — and a wallet gating on this signed a
     * transfer authorization it could not take back. py refuses both.
     */
    void testVerifyRefusesAnEntryThatAuthorizesSomethingElse()
    {
        const qint64 t = now();
        const QString webAuth = webAuthContract();
        const QString token = contractStrKey('\x33');
        const QList<stellar::SCVal> args = challengeArgs(t + 300);
        QScopedPointer<KeyPair> attacker(KeyPair::random());
        const stellar::SorobanAuthorizedInvocation transfer = invocationOf(token, "transfer",
            QList<stellar::SCVal>() << Scv::toAddress(QString(clientContract()))
                                    << Scv::toAddress(attacker->getAccountId())
                                    << Scv::toUint64(quint64(1000000000000ULL)));

        // The genuine shape passes, so what follows is refused for its entry.
        QCOMPARE(refusal(handMade(webAuth, "web_auth_verify", args,
                                  invocationOf(webAuth, "web_auth_verify", args), t, t + 300)),
                 QString());

        QCOMPARE(refusal(handMade(webAuth, "web_auth_verify", args, transfer, t, t + 300)),
                 QString("auth entry must authorize exactly the challenge's own call"));

        stellar::SorobanAuthorizedInvocation nested = invocationOf(webAuth, "web_auth_verify", args);
        nested.subInvocations = QSharedPointer<QList<stellar::SorobanAuthorizedInvocation>>::create();
        nested.subInvocations->append(transfer);
        QCOMPARE(refusal(handMade(webAuth, "web_auth_verify", args, nested, t, t + 300)),
                 QString("auth entry must not have sub-invocations"));

        QList<stellar::SCVal> otherArgs = args;
        otherArgs[1] = Scv::toString(QStringLiteral("evil.com"));
        QCOMPARE(refusal(handMade(webAuth, "web_auth_verify", args,
                                  invocationOf(webAuth, "web_auth_verify", otherArgs), t, t + 300)),
                 QString("auth entry must authorize exactly the challenge's own call"));
    }

    /**
     * Which contract and function a challenge calls was never checked: one
     * calling token.transfer, or drain_wallet on any contract, verified like a
     * login. The function is checked always (web_auth_verify unless named),
     * the contract when the caller names it.
     */
    void testVerifyChecksTheContractAndFunctionCalled()
    {
        const qint64 t = now();
        const QString token = contractStrKey('\x33');
        QScopedPointer<KeyPair> server(serverSigner());

        const QString transferCall = Sep45Challenge::newChallenge(
            server.data(), Network::testnetNetwork(), clientContract(), token,
            QStringLiteral("transfer"), QStringLiteral("example.com"),
            QStringLiteral("auth.example.com"), 300, fixedNonce());
        QCOMPARE(refusal(transferCall), QString("challenge does not call web_auth_verify"));

        const QList<stellar::SCVal> args = challengeArgs(t + 300);
        QCOMPARE(refusal(handMade(token, "drain_wallet", args,
                                  invocationOf(token, "drain_wallet", args), t, t + 300)),
                 QString("challenge does not call web_auth_verify"));

        // Named, the contract is checked, and reported.
        auto v = Sep45Challenge::verifyChallenge(
            validChallenge(), serverId(), Network::testnetNetwork(), nullptr,
            QStringList{QStringLiteral("example.com")}, QStringLiteral("auth.example.com"),
            QString(webAuthContract()));
        QCOMPARE(v.webAuthContract, QString(webAuthContract()));
        QCOMPARE(v.webAuthFunctionName, QString("web_auth_verify"));

        for (const QString& expected : {token, QString(), serverId()}) {
            bool threw = false;
            try {
                Sep45Challenge::verifyChallenge(
                    validChallenge(), serverId(), Network::testnetNetwork(), nullptr,
                    QStringList{QStringLiteral("example.com")}, QStringLiteral("auth.example.com"),
                    expected);
            } catch (const qstellar::exception::InvalidSep45ChallengeException&) { threw = true; }
            QVERIFY2(threw, qPrintable(expected));
        }

        // Another function name is accepted only where it is named.
        const QString custom = Sep45Challenge::newChallenge(
            server.data(), Network::testnetNetwork(), clientContract(), webAuthContract(),
            QStringLiteral("my_auth"), QStringLiteral("example.com"),
            QStringLiteral("auth.example.com"), 300, fixedNonce());
        QVERIFY(!refusal(custom).isEmpty());
        v = Sep45Challenge::verifyChallenge(
            custom, serverId(), Network::testnetNetwork(), nullptr,
            QStringList{QStringLiteral("example.com")}, QStringLiteral("auth.example.com"),
            QString(webAuthContract()), QStringLiteral("my_auth"));
        QCOMPARE(v.webAuthFunctionName, QString("my_auth"));
    }

    /**
     * The grace period was added AFTER expiry, so a challenge that expired
     * up to five minutes ago verified; an expiration argument in the past was
     * accepted while the transaction was still valid.
     */
    void testVerifyRefusesAnExpiredChallenge()
    {
        const qint64 t = now();
        QCOMPARE(refusal(withTimes(quint64(t + 300), t, t + 300)), QString());
        QVERIFY(!refusal(withTimes(quint64(t - 2), t - 300, t - 2)).isEmpty());
        QVERIFY(!refusal(withTimes(quint64(t - 200), t - 500, t - 200)).isEmpty());
        QCOMPARE(refusal(withTimes(quint64(t - 200), t, t + 300)), QString("challenge expired"));
    }

    /** The transaction's time bounds were not read at all: expired an hour
     *  ago, infinite, absent or valid only tomorrow, all verified. They are
     *  checked as SEP-10 checks them, the grace before minTime only. */
    void testVerifyChecksTheTransactionTimeBounds()
    {
        const qint64 t = now();
        const quint64 expiration = quint64(t + 300);
        QVERIFY(!refusal(withTimes(expiration, t - 3600, t - 3000)).isEmpty());
        QCOMPARE(refusal(withTimes(expiration, 0, 0)), QString("challenge requires non-infinite timebounds"));
        QCOMPARE(refusal(withTimes(expiration, 0, 0, false)), QString("challenge requires timebounds"));
        QCOMPARE(refusal(withTimes(expiration, t + 86400, t + 90000)),
                 QString("challenge is not within its timebounds"));
        // A server clock a little ahead is what the grace is for.
        QCOMPARE(refusal(withTimes(quint64(t + 400), t + 100, t + 400)), QString());
    }

    /** expiration + grace was signed arithmetic: near INT64_MAX it wrapped
     *  negative and a far-future challenge came out "expired" (undefined
     *  behaviour). Compared without arithmetic, only the unrepresentable is
     *  refused. */
    void testVerifyExpirationDoesNotOverflow()
    {
        const qint64 t = now();
        const quint64 max = quint64(std::numeric_limits<qint64>::max());
        QCOMPARE(refusal(withTimes(max - 299, t, t + 300)), QString());
        QCOMPARE(refusal(withTimes(max, t, t + 300)), QString());
        QCOMPARE(refusal(withTimes(max + 1, t, t + 300)), QString("challenge expiration is out of range"));
        QCOMPARE(refusal(withTimes(std::numeric_limits<quint64>::max(), t, t + 300)),
                 QString("challenge expiration is out of range"));
    }

    /** A blank entry in the allowed home domains matched a challenge with no
     *  home domain, and a blank expected webAuthDomain one with none. */
    void testVerifyRefusesBlankDomains()
    {
        const qint64 t = now();
        const QList<stellar::SCVal> blankHome = challengeArgs(quint64(t + 300), QString());
        const QString noHome = handMade(webAuthContract(), "web_auth_verify", blankHome,
                                        invocationOf(webAuthContract(), "web_auth_verify", blankHome),
                                        t, t + 300);
        QCOMPARE(refusal(noHome, QStringList{QStringLiteral("example.com"), QString()}),
                 QString("homeDomain not in allowed set"));

        const QList<stellar::SCVal> blankWeb = challengeArgs(quint64(t + 300), QStringLiteral("example.com"), QString());
        const QString noWebAuth = handMade(webAuthContract(), "web_auth_verify", blankWeb,
                                           invocationOf(webAuthContract(), "web_auth_verify", blankWeb),
                                           t, t + 300);
        QCOMPARE(refusal(noWebAuth, QStringList{QStringLiteral("example.com")}, QString()),
                 QString("webAuthDomain is required"));
    }

    /** newChallenge took any timeout (the QML layer caps it at a day) and
     *  signed challenges with a blank home or web-auth domain, which a
     *  verifier can only refuse. */
    void testNewChallengeRefusesWhatCannotVerify()
    {
        QScopedPointer<KeyPair> server(serverSigner());
        auto builds = [&](qint64 timeout, const QString& home, const QString& webAuth) {
            try {
                Sep45Challenge::newChallenge(server.data(), Network::testnetNetwork(),
                                             clientContract(), webAuthContract(),
                                             QStringLiteral("web_auth_verify"), home, webAuth,
                                             timeout, fixedNonce());
                return true;
            } catch (const std::runtime_error&) {
                return false;
            }
        };
        QVERIFY(builds(86400, QStringLiteral("example.com"), QStringLiteral("auth.example.com")));
        QVERIFY(!builds(86401, QStringLiteral("example.com"), QStringLiteral("auth.example.com")));
        QVERIFY(!builds(300, QString(), QStringLiteral("auth.example.com")));
        QVERIFY(!builds(300, QStringLiteral(" "), QStringLiteral("auth.example.com")));
        QVERIFY(!builds(300, QStringLiteral("example.com"), QString()));
    }

    // ─── SEP-45 v0.1.1 ────────────────────────────────────────────────
    //
    // The fixtures below were produced by py-stellar-sdk 14.1.0
    // (sep/stellar_soroban_web_authentication.py, the reference) with fixed
    // keys, nonce and ledger, and a simulation stubbed with the entries an RPC
    // returns (scratch script fixtures.py): the transaction py simulates to
    // build a challenge, the challenge it builds, the challenge signed by
    // py's authorize_entry for the client and the client domain, and the
    // transaction py simulates to verify it. rpcRecorded* is a simulation a
    // real RPC answered, taken from py's own tests, and what py builds from it.

private:
    static QString V_WEB_AUTH() { return QStringLiteral("CCSZMK2C2B7UVP3Q4JRWLS3JY7ZB4YTTRSWQXGWQVS24W3PPZXOUHH4R"); }
    static QString V_CLIENT() { return QStringLiteral("CBUCJMHBZHQ3EXQ2LMSFVZUWCPH7BCTCYGOQ6LIIO2OUVKU3XDDOO2HN"); }
    static QString V_SERVER_SEED() { return QStringLiteral("SC7UDDRCWZQXXCJS6I44TK54WYA4X4Z7FXNMV3D66P4TC2VX7GYVKP36"); }
    static QString V_SERVER() { return QStringLiteral("GCSNRCUM6EDEKSSBQNIOP66ONIM26FVCYP3GHYGD4NR3DK4F635Z32WQ"); }
    static QString V_DOMAIN_SEED() { return QStringLiteral("SAIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRDBNT"); }
    static QString V_DOMAIN_ACCOUNT() { return QStringLiteral("GDIEVMRSOQV3JKZ2CNUL2RQV4TTNAISKW4NAC25PQUQKGMWJO6DTOAE7"); }
    static QString V_CLIENT_SIGNER_SEED() { return QStringLiteral("SARCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEIRCEVGI"); }
    static QString V_HOME() { return QStringLiteral("example.com"); }
    static QString V_WEB() { return QStringLiteral("auth.example.com"); }
    static QString V_CLIENT_DOMAIN() { return QStringLiteral("wallet.example.org"); }
    static QString V_NONCE() { return QStringLiteral("0Q7tFfp5hL2oJv1n9mY3sR8aZc6WkX4bUe0dGiT/+ypoQ1lNwVrE5Ks7jHuMf2Ax"); }
    static constexpr quint32 V_LATEST = 82106;
    static constexpr quint32 V_VALID_UNTIL = 82200;

    static QString pyDChallengeTx() {
        return QStringLiteral(
            "AAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAGQAAAAAAAAAAQAAAAEAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAEAAAAAAAAAGAAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVy"
            "aWZ5AAAAAAEAAAARAAAAAQAAAAcAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3"
            "QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAANY2xpZW50X2RvbWFpbgAAAAAAAA4AAAASd2FsbGV0LmV4YW1w"
            "bGUub3JnAAAAAAAPAAAAFWNsaWVudF9kb21haW5fYWNjb3VudAAAAAAAAA4AAAA4R0RJRVZNUlNPUVYzSktaMkNOVUwyUlFW"
            "NFRUTkFJU0tXNE5BQzI1UFFVUUtHTVdKTzZEVE9BRTcAAAAPAAAAC2hvbWVfZG9tYWluAAAAAA4AAAALZXhhbXBsZS5jb20A"
            "AAAADwAAAAVub25jZQAAAAAAAA4AAABAMFE3dEZmcDVoTDJvSnYxbjltWTNzUjhhWmM2V2tYNGJVZTBkR2lULyt5cG9RMWxO"
            "d1ZyRTVLczdqSHVNZjJBeAAAAA8AAAAPd2ViX2F1dGhfZG9tYWluAAAAAA4AAAAQYXV0aC5leGFtcGxlLmNvbQAAAA8AAAAX"
            "d2ViX2F1dGhfZG9tYWluX2FjY291bnQAAAAADgAAADhHQ1NOUkNVTTZFREVLU1NCUU5JT1A2Nk9OSU0yNkZWQ1lQM0dIWUdE"
            "NE5SM0RLNEY2MzVaMzJXUQAAAAAAAAAAAAAAAA==");
    }

    static QStringList pyDSimulatedAuth() {
        return QStringList{
            QStringLiteral("AAAAAQAAAAFoJLDhyeGyXhpbJFrmlhPP8IpiwZ0PLQh2nUqqm7jG5yM8uN9jhyPHAAAAAAAAAAEAAAAAAAAAAaWWK0LQf0q/"
                           "cOJjZctpx/IeYnOMrQua0Ky1y23vzd1DAAAAD3dlYl9hdXRoX3ZlcmlmeQAAAAABAAAAEQAAAAEAAAAHAAAADwAAAAdhY2Nv"
                           "dW50AAAAAA4AAAA4Q0JVQ0pNSEJaSFEzRVhRMkxNU0ZWWlVXQ1BIN0JDVENZR09RNkxJSU8yT1VWS1UzWERET08ySE4AAAAP"
                           "AAAADWNsaWVudF9kb21haW4AAAAAAAAOAAAAEndhbGxldC5leGFtcGxlLm9yZwAAAAAADwAAABVjbGllbnRfZG9tYWluX2Fj"
                           "Y291bnQAAAAAAAAOAAAAOEdESUVWTVJTT1FWM0pLWjJDTlVMMlJRVjRUVE5BSVNLVzROQUMyNVBRVVFLR01XSk82RFRPQUU3"
                           "AAAADwAAAAtob21lX2RvbWFpbgAAAAAOAAAAC2V4YW1wbGUuY29tAAAAAA8AAAAFbm9uY2UAAAAAAAAOAAAAQDBRN3RGZnA1"
                           "aEwyb0p2MW45bVkzc1I4YVpjNldrWDRiVWUwZEdpVC8reXBvUTFsTndWckU1S3M3akh1TWYyQXgAAAAPAAAAD3dlYl9hdXRo"
                           "X2RvbWFpbgAAAAAOAAAAEGF1dGguZXhhbXBsZS5jb20AAAAPAAAAF3dlYl9hdXRoX2RvbWFpbl9hY2NvdW50AAAAAA4AAAA4"
                           "R0NTTlJDVU02RURFS1NTQlFOSU9QNjZPTklNMjZGVkNZUDNHSFlHRDROUjNESzRGNjM1WjMyV1EAAAAA"),
            QStringLiteral("AAAAAQAAAAAAAAAApNiKjPEGRUpBg1Dn+85qGa8WosP2Y+DD42Oxq4X2+508ErpFoj2T9gAAAAAAAAABAAAAAAAAAAGllitC"
                           "0H9Kv3DiY2XLacfyHmJzjK0LmtCstctt783dQwAAAA93ZWJfYXV0aF92ZXJpZnkAAAAAAQAAABEAAAABAAAABwAAAA8AAAAH"
                           "YWNjb3VudAAAAAAOAAAAOENCVUNKTUhCWkhRM0VYUTJMTVNGVlpVV0NQSDdCQ1RDWUdPUTZMSUlPMk9VVktVM1hERE9PMkhO"
                           "AAAADwAAAA1jbGllbnRfZG9tYWluAAAAAAAADgAAABJ3YWxsZXQuZXhhbXBsZS5vcmcAAAAAAA8AAAAVY2xpZW50X2RvbWFp"
                           "bl9hY2NvdW50AAAAAAAADgAAADhHRElFVk1SU09RVjNKS1oyQ05VTDJSUVY0VFROQUlTS1c0TkFDMjVQUVVRS0dNV0pPNkRU"
                           "T0FFNwAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAPAAAABW5vbmNlAAAAAAAADgAAAEAwUTd0"
                           "RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQvK3lwb1ExbE53VnJFNUtzN2pIdU1mMkF4AAAADwAAAA93ZWJf"
                           "YXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJfYXV0aF9kb21haW5fYWNjb3VudAAAAAAO"
                           "AAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIzREs0RjYzNVozMldRAAAAAA=="),
            QStringLiteral("AAAAAQAAAAAAAAAA0EqyMnQrtKs6E2i9RhXk5tAiSrcaAWuvhSCjMsl3hzci3bLdrq/hmQAAAAAAAAABAAAAAAAAAAGllitC"
                           "0H9Kv3DiY2XLacfyHmJzjK0LmtCstctt783dQwAAAA93ZWJfYXV0aF92ZXJpZnkAAAAAAQAAABEAAAABAAAABwAAAA8AAAAH"
                           "YWNjb3VudAAAAAAOAAAAOENCVUNKTUhCWkhRM0VYUTJMTVNGVlpVV0NQSDdCQ1RDWUdPUTZMSUlPMk9VVktVM1hERE9PMkhO"
                           "AAAADwAAAA1jbGllbnRfZG9tYWluAAAAAAAADgAAABJ3YWxsZXQuZXhhbXBsZS5vcmcAAAAAAA8AAAAVY2xpZW50X2RvbWFp"
                           "bl9hY2NvdW50AAAAAAAADgAAADhHRElFVk1SU09RVjNKS1oyQ05VTDJSUVY0VFROQUlTS1c0TkFDMjVQUVVRS0dNV0pPNkRU"
                           "T0FFNwAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAPAAAABW5vbmNlAAAAAAAADgAAAEAwUTd0"
                           "RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQvK3lwb1ExbE53VnJFNUtzN2pIdU1mMkF4AAAADwAAAA93ZWJf"
                           "YXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJfYXV0aF9kb21haW5fYWNjb3VudAAAAAAO"
                           "AAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIzREs0RjYzNVozMldRAAAAAA=="),
        };
    }

    static QString pyDBuilt() {
        return QStringLiteral(
            "AAAAAwAAAAEAAAABaCSw4cnhsl4aWyRa5pYTz/CKYsGdDy0Idp1Kqpu4xucjPLjfY4cjxwAAAAAAAAABAAAAAAAAAAGllitC"
            "0H9Kv3DiY2XLacfyHmJzjK0LmtCstctt783dQwAAAA93ZWJfYXV0aF92ZXJpZnkAAAAAAQAAABEAAAABAAAABwAAAA8AAAAH"
            "YWNjb3VudAAAAAAOAAAAOENCVUNKTUhCWkhRM0VYUTJMTVNGVlpVV0NQSDdCQ1RDWUdPUTZMSUlPMk9VVktVM1hERE9PMkhO"
            "AAAADwAAAA1jbGllbnRfZG9tYWluAAAAAAAADgAAABJ3YWxsZXQuZXhhbXBsZS5vcmcAAAAAAA8AAAAVY2xpZW50X2RvbWFp"
            "bl9hY2NvdW50AAAAAAAADgAAADhHRElFVk1SU09RVjNKS1oyQ05VTDJSUVY0VFROQUlTS1c0TkFDMjVQUVVRS0dNV0pPNkRU"
            "T0FFNwAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAPAAAABW5vbmNlAAAAAAAADgAAAEAwUTd0"
            "RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQvK3lwb1ExbE53VnJFNUtzN2pIdU1mMkF4AAAADwAAAA93ZWJf"
            "YXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJfYXV0aF9kb21haW5fYWNjb3VudAAAAAAO"
            "AAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIzREs0RjYzNVozMldRAAAAAAAAAAEAAAAA"
            "AAAAAKTYiozxBkVKQYNQ5/vOahmvFqLD9mPgw+NjsauF9vudPBK6RaI9k/YAAUFuAAAAEAAAAAEAAAABAAAAEQAAAAEAAAAC"
            "AAAADwAAAApwdWJsaWNfa2V5AAAAAAANAAAAIKTYiozxBkVKQYNQ5/vOahmvFqLD9mPgw+NjsauF9vudAAAADwAAAAlzaWdu"
            "YXR1cmUAAAAAAAANAAAAQB/7Mvf6yb/GqoPaXmL7TkhUios5tnhq8YjeM+cFz0xLSwImzTRrIzNKmCt06SiRZOReNR399aZN"
            "+kAbKQhbzw8AAAAAAAAAAaWWK0LQf0q/cOJjZctpx/IeYnOMrQua0Ky1y23vzd1DAAAAD3dlYl9hdXRoX3ZlcmlmeQAAAAAB"
            "AAAAEQAAAAEAAAAHAAAADwAAAAdhY2NvdW50AAAAAA4AAAA4Q0JVQ0pNSEJaSFEzRVhRMkxNU0ZWWlVXQ1BIN0JDVENZR09R"
            "NkxJSU8yT1VWS1UzWERET08ySE4AAAAPAAAADWNsaWVudF9kb21haW4AAAAAAAAOAAAAEndhbGxldC5leGFtcGxlLm9yZwAA"
            "AAAADwAAABVjbGllbnRfZG9tYWluX2FjY291bnQAAAAAAAAOAAAAOEdESUVWTVJTT1FWM0pLWjJDTlVMMlJRVjRUVE5BSVNL"
            "VzROQUMyNVBRVVFLR01XSk82RFRPQUU3AAAADwAAAAtob21lX2RvbWFpbgAAAAAOAAAAC2V4YW1wbGUuY29tAAAAAA8AAAAF"
            "bm9uY2UAAAAAAAAOAAAAQDBRN3RGZnA1aEwyb0p2MW45bVkzc1I4YVpjNldrWDRiVWUwZEdpVC8reXBvUTFsTndWckU1S3M3"
            "akh1TWYyQXgAAAAPAAAAD3dlYl9hdXRoX2RvbWFpbgAAAAAOAAAAEGF1dGguZXhhbXBsZS5jb20AAAAPAAAAF3dlYl9hdXRo"
            "X2RvbWFpbl9hY2NvdW50AAAAAA4AAAA4R0NTTlJDVU02RURFS1NTQlFOSU9QNjZPTklNMjZGVkNZUDNHSFlHRDROUjNESzRG"
            "NjM1WjMyV1EAAAAAAAAAAQAAAAAAAAAA0EqyMnQrtKs6E2i9RhXk5tAiSrcaAWuvhSCjMsl3hzci3bLdrq/hmQAAAAAAAAAB"
            "AAAAAAAAAAGllitC0H9Kv3DiY2XLacfyHmJzjK0LmtCstctt783dQwAAAA93ZWJfYXV0aF92ZXJpZnkAAAAAAQAAABEAAAAB"
            "AAAABwAAAA8AAAAHYWNjb3VudAAAAAAOAAAAOENCVUNKTUhCWkhRM0VYUTJMTVNGVlpVV0NQSDdCQ1RDWUdPUTZMSUlPMk9V"
            "VktVM1hERE9PMkhOAAAADwAAAA1jbGllbnRfZG9tYWluAAAAAAAADgAAABJ3YWxsZXQuZXhhbXBsZS5vcmcAAAAAAA8AAAAV"
            "Y2xpZW50X2RvbWFpbl9hY2NvdW50AAAAAAAADgAAADhHRElFVk1SU09RVjNKS1oyQ05VTDJSUVY0VFROQUlTS1c0TkFDMjVQ"
            "UVVRS0dNV0pPNkRUT0FFNwAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAPAAAABW5vbmNlAAAA"
            "AAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQvK3lwb1ExbE53VnJFNUtzN2pIdU1mMkF4"
            "AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJfYXV0aF9kb21haW5f"
            "YWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIzREs0RjYzNVozMldR"
            "AAAAAA==");
    }

    static QString pyDSigned() {
        return QStringLiteral(
            "AAAAAwAAAAEAAAABaCSw4cnhsl4aWyRa5pYTz/CKYsGdDy0Idp1Kqpu4xucjPLjfY4cjxwABQRgAAAAQAAAAAQAAAAEAAAAR"
            "AAAAAQAAAAIAAAAPAAAACnB1YmxpY19rZXkAAAAAAA0AAAAgoJql9HpnWYAv+VX43C0qFKXJnSO+l/hkEn/5ODRVpPAAAAAP"
            "AAAACXNpZ25hdHVyZQAAAAAAAA0AAABATMvPmqFyeyip3da1xo8Vts3K5HALm/2A/i9chn1JRIWhR6h5mlkB2SqXyLUN6dnT"
            "qnHI5HoswLqz/qeeGywvCgAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVy"
            "aWZ5AAAAAAEAAAARAAAAAQAAAAcAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3"
            "QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAANY2xpZW50X2RvbWFpbgAAAAAAAA4AAAASd2FsbGV0LmV4YW1w"
            "bGUub3JnAAAAAAAPAAAAFWNsaWVudF9kb21haW5fYWNjb3VudAAAAAAAAA4AAAA4R0RJRVZNUlNPUVYzSktaMkNOVUwyUlFW"
            "NFRUTkFJU0tXNE5BQzI1UFFVUUtHTVdKTzZEVE9BRTcAAAAPAAAAC2hvbWVfZG9tYWluAAAAAA4AAAALZXhhbXBsZS5jb20A"
            "AAAADwAAAAVub25jZQAAAAAAAA4AAABAMFE3dEZmcDVoTDJvSnYxbjltWTNzUjhhWmM2V2tYNGJVZTBkR2lULyt5cG9RMWxO"
            "d1ZyRTVLczdqSHVNZjJBeAAAAA8AAAAPd2ViX2F1dGhfZG9tYWluAAAAAA4AAAAQYXV0aC5leGFtcGxlLmNvbQAAAA8AAAAX"
            "d2ViX2F1dGhfZG9tYWluX2FjY291bnQAAAAADgAAADhHQ1NOUkNVTTZFREVLU1NCUU5JT1A2Nk9OSU0yNkZWQ1lQM0dIWUdE"
            "NE5SM0RLNEY2MzVaMzJXUQAAAAAAAAABAAAAAAAAAACk2IqM8QZFSkGDUOf7zmoZrxaiw/Zj4MPjY7Grhfb7nTwSukWiPZP2"
            "AAFBbgAAABAAAAABAAAAAQAAABEAAAABAAAAAgAAAA8AAAAKcHVibGljX2tleQAAAAAADQAAACCk2IqM8QZFSkGDUOf7zmoZ"
            "rxaiw/Zj4MPjY7Grhfb7nQAAAA8AAAAJc2lnbmF0dXJlAAAAAAAADQAAAEAf+zL3+sm/xqqD2l5i+05IVIqLObZ4avGI3jPn"
            "Bc9MS0sCJs00ayMzSpgrdOkokWTkXjUd/fWmTfpAGykIW88PAAAAAAAAAAGllitC0H9Kv3DiY2XLacfyHmJzjK0LmtCstctt"
            "783dQwAAAA93ZWJfYXV0aF92ZXJpZnkAAAAAAQAAABEAAAABAAAABwAAAA8AAAAHYWNjb3VudAAAAAAOAAAAOENCVUNKTUhC"
            "WkhRM0VYUTJMTVNGVlpVV0NQSDdCQ1RDWUdPUTZMSUlPMk9VVktVM1hERE9PMkhOAAAADwAAAA1jbGllbnRfZG9tYWluAAAA"
            "AAAADgAAABJ3YWxsZXQuZXhhbXBsZS5vcmcAAAAAAA8AAAAVY2xpZW50X2RvbWFpbl9hY2NvdW50AAAAAAAADgAAADhHRElF"
            "Vk1SU09RVjNKS1oyQ05VTDJSUVY0VFROQUlTS1c0TkFDMjVQUVVRS0dNV0pPNkRUT0FFNwAAAA8AAAALaG9tZV9kb21haW4A"
            "AAAADgAAAAtleGFtcGxlLmNvbQAAAAAPAAAABW5vbmNlAAAAAAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZX"
            "a1g0YlVlMGRHaVQvK3lwb1ExbE53VnJFNUtzN2pIdU1mMkF4AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRo"
            "LmV4YW1wbGUuY29tAAAADwAAABd3ZWJfYXV0aF9kb21haW5fYWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklP"
            "UDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIzREs0RjYzNVozMldRAAAAAAAAAAEAAAAAAAAAANBKsjJ0K7SrOhNovUYV5ObQIkq3"
            "GgFrr4UgozLJd4c3It2y3a6v4ZkAAUEYAAAAEAAAAAEAAAABAAAAEQAAAAEAAAACAAAADwAAAApwdWJsaWNfa2V5AAAAAAAN"
            "AAAAINBKsjJ0K7SrOhNovUYV5ObQIkq3GgFrr4UgozLJd4c3AAAADwAAAAlzaWduYXR1cmUAAAAAAAANAAAAQDP8aVB2jji0"
            "3NRfA2NSMnLUHm9y2mvrjnoDLhVbua86LUM2ooqzSIlWzOtis1Uf8Lcy3wUUBJJIox0Aqp/u+w0AAAAAAAAAAaWWK0LQf0q/"
            "cOJjZctpx/IeYnOMrQua0Ky1y23vzd1DAAAAD3dlYl9hdXRoX3ZlcmlmeQAAAAABAAAAEQAAAAEAAAAHAAAADwAAAAdhY2Nv"
            "dW50AAAAAA4AAAA4Q0JVQ0pNSEJaSFEzRVhRMkxNU0ZWWlVXQ1BIN0JDVENZR09RNkxJSU8yT1VWS1UzWERET08ySE4AAAAP"
            "AAAADWNsaWVudF9kb21haW4AAAAAAAAOAAAAEndhbGxldC5leGFtcGxlLm9yZwAAAAAADwAAABVjbGllbnRfZG9tYWluX2Fj"
            "Y291bnQAAAAAAAAOAAAAOEdESUVWTVJTT1FWM0pLWjJDTlVMMlJRVjRUVE5BSVNLVzROQUMyNVBRVVFLR01XSk82RFRPQUU3"
            "AAAADwAAAAtob21lX2RvbWFpbgAAAAAOAAAAC2V4YW1wbGUuY29tAAAAAA8AAAAFbm9uY2UAAAAAAAAOAAAAQDBRN3RGZnA1"
            "aEwyb0p2MW45bVkzc1I4YVpjNldrWDRiVWUwZEdpVC8reXBvUTFsTndWckU1S3M3akh1TWYyQXgAAAAPAAAAD3dlYl9hdXRo"
            "X2RvbWFpbgAAAAAOAAAAEGF1dGguZXhhbXBsZS5jb20AAAAPAAAAF3dlYl9hdXRoX2RvbWFpbl9hY2NvdW50AAAAAA4AAAA4"
            "R0NTTlJDVU02RURFS1NTQlFOSU9QNjZPTklNMjZGVkNZUDNHSFlHRDROUjNESzRGNjM1WjMyV1EAAAAA");
    }

    static QString pyDClientOnlySigned() {
        return QStringLiteral(
            "AAAAAwAAAAEAAAABaCSw4cnhsl4aWyRa5pYTz/CKYsGdDy0Idp1Kqpu4xucjPLjfY4cjxwABQRgAAAAQAAAAAQAAAAEAAAAR"
            "AAAAAQAAAAIAAAAPAAAACnB1YmxpY19rZXkAAAAAAA0AAAAgoJql9HpnWYAv+VX43C0qFKXJnSO+l/hkEn/5ODRVpPAAAAAP"
            "AAAACXNpZ25hdHVyZQAAAAAAAA0AAABATMvPmqFyeyip3da1xo8Vts3K5HALm/2A/i9chn1JRIWhR6h5mlkB2SqXyLUN6dnT"
            "qnHI5HoswLqz/qeeGywvCgAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVy"
            "aWZ5AAAAAAEAAAARAAAAAQAAAAcAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3"
            "QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAANY2xpZW50X2RvbWFpbgAAAAAAAA4AAAASd2FsbGV0LmV4YW1w"
            "bGUub3JnAAAAAAAPAAAAFWNsaWVudF9kb21haW5fYWNjb3VudAAAAAAAAA4AAAA4R0RJRVZNUlNPUVYzSktaMkNOVUwyUlFW"
            "NFRUTkFJU0tXNE5BQzI1UFFVUUtHTVdKTzZEVE9BRTcAAAAPAAAAC2hvbWVfZG9tYWluAAAAAA4AAAALZXhhbXBsZS5jb20A"
            "AAAADwAAAAVub25jZQAAAAAAAA4AAABAMFE3dEZmcDVoTDJvSnYxbjltWTNzUjhhWmM2V2tYNGJVZTBkR2lULyt5cG9RMWxO"
            "d1ZyRTVLczdqSHVNZjJBeAAAAA8AAAAPd2ViX2F1dGhfZG9tYWluAAAAAA4AAAAQYXV0aC5leGFtcGxlLmNvbQAAAA8AAAAX"
            "d2ViX2F1dGhfZG9tYWluX2FjY291bnQAAAAADgAAADhHQ1NOUkNVTTZFREVLU1NCUU5JT1A2Nk9OSU0yNkZWQ1lQM0dIWUdE"
            "NE5SM0RLNEY2MzVaMzJXUQAAAAAAAAABAAAAAAAAAACk2IqM8QZFSkGDUOf7zmoZrxaiw/Zj4MPjY7Grhfb7nTwSukWiPZP2"
            "AAFBbgAAABAAAAABAAAAAQAAABEAAAABAAAAAgAAAA8AAAAKcHVibGljX2tleQAAAAAADQAAACCk2IqM8QZFSkGDUOf7zmoZ"
            "rxaiw/Zj4MPjY7Grhfb7nQAAAA8AAAAJc2lnbmF0dXJlAAAAAAAADQAAAEAf+zL3+sm/xqqD2l5i+05IVIqLObZ4avGI3jPn"
            "Bc9MS0sCJs00ayMzSpgrdOkokWTkXjUd/fWmTfpAGykIW88PAAAAAAAAAAGllitC0H9Kv3DiY2XLacfyHmJzjK0LmtCstctt"
            "783dQwAAAA93ZWJfYXV0aF92ZXJpZnkAAAAAAQAAABEAAAABAAAABwAAAA8AAAAHYWNjb3VudAAAAAAOAAAAOENCVUNKTUhC"
            "WkhRM0VYUTJMTVNGVlpVV0NQSDdCQ1RDWUdPUTZMSUlPMk9VVktVM1hERE9PMkhOAAAADwAAAA1jbGllbnRfZG9tYWluAAAA"
            "AAAADgAAABJ3YWxsZXQuZXhhbXBsZS5vcmcAAAAAAA8AAAAVY2xpZW50X2RvbWFpbl9hY2NvdW50AAAAAAAADgAAADhHRElF"
            "Vk1SU09RVjNKS1oyQ05VTDJSUVY0VFROQUlTS1c0TkFDMjVQUVVRS0dNV0pPNkRUT0FFNwAAAA8AAAALaG9tZV9kb21haW4A"
            "AAAADgAAAAtleGFtcGxlLmNvbQAAAAAPAAAABW5vbmNlAAAAAAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZX"
            "a1g0YlVlMGRHaVQvK3lwb1ExbE53VnJFNUtzN2pIdU1mMkF4AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRo"
            "LmV4YW1wbGUuY29tAAAADwAAABd3ZWJfYXV0aF9kb21haW5fYWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklP"
            "UDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIzREs0RjYzNVozMldRAAAAAAAAAAEAAAAAAAAAANBKsjJ0K7SrOhNovUYV5ObQIkq3"
            "GgFrr4UgozLJd4c3It2y3a6v4ZkAAAAAAAAAAQAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAP"
            "d2ViX2F1dGhfdmVyaWZ5AAAAAAEAAAARAAAAAQAAAAcAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEy"
            "TE1TRlZaVVdDUEg3QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAANY2xpZW50X2RvbWFpbgAAAAAAAA4AAAAS"
            "d2FsbGV0LmV4YW1wbGUub3JnAAAAAAAPAAAAFWNsaWVudF9kb21haW5fYWNjb3VudAAAAAAAAA4AAAA4R0RJRVZNUlNPUVYz"
            "SktaMkNOVUwyUlFWNFRUTkFJU0tXNE5BQzI1UFFVUUtHTVdKTzZEVE9BRTcAAAAPAAAAC2hvbWVfZG9tYWluAAAAAA4AAAAL"
            "ZXhhbXBsZS5jb20AAAAADwAAAAVub25jZQAAAAAAAA4AAABAMFE3dEZmcDVoTDJvSnYxbjltWTNzUjhhWmM2V2tYNGJVZTBk"
            "R2lULyt5cG9RMWxOd1ZyRTVLczdqSHVNZjJBeAAAAA8AAAAPd2ViX2F1dGhfZG9tYWluAAAAAA4AAAAQYXV0aC5leGFtcGxl"
            "LmNvbQAAAA8AAAAXd2ViX2F1dGhfZG9tYWluX2FjY291bnQAAAAADgAAADhHQ1NOUkNVTTZFREVLU1NCUU5JT1A2Nk9OSU0y"
            "NkZWQ1lQM0dIWUdENE5SM0RLNEY2MzVaMzJXUQAAAAA=");
    }

    static QString pyDVerifyTx() {
        return QStringLiteral(
            "AAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAGQAAAAAAAAAAQAAAAEAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAEAAAAAAAAAGAAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVy"
            "aWZ5AAAAAAEAAAARAAAAAQAAAAcAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3"
            "QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAANY2xpZW50X2RvbWFpbgAAAAAAAA4AAAASd2FsbGV0LmV4YW1w"
            "bGUub3JnAAAAAAAPAAAAFWNsaWVudF9kb21haW5fYWNjb3VudAAAAAAAAA4AAAA4R0RJRVZNUlNPUVYzSktaMkNOVUwyUlFW"
            "NFRUTkFJU0tXNE5BQzI1UFFVUUtHTVdKTzZEVE9BRTcAAAAPAAAAC2hvbWVfZG9tYWluAAAAAA4AAAALZXhhbXBsZS5jb20A"
            "AAAADwAAAAVub25jZQAAAAAAAA4AAABAMFE3dEZmcDVoTDJvSnYxbjltWTNzUjhhWmM2V2tYNGJVZTBkR2lULyt5cG9RMWxO"
            "d1ZyRTVLczdqSHVNZjJBeAAAAA8AAAAPd2ViX2F1dGhfZG9tYWluAAAAAA4AAAAQYXV0aC5leGFtcGxlLmNvbQAAAA8AAAAX"
            "d2ViX2F1dGhfZG9tYWluX2FjY291bnQAAAAADgAAADhHQ1NOUkNVTTZFREVLU1NCUU5JT1A2Nk9OSU0yNkZWQ1lQM0dIWUdE"
            "NE5SM0RLNEY2MzVaMzJXUQAAAAMAAAABAAAAAWgksOHJ4bJeGlskWuaWE8/wimLBnQ8tCHadSqqbuMbnIzy432OHI8cAAUEY"
            "AAAAEAAAAAEAAAABAAAAEQAAAAEAAAACAAAADwAAAApwdWJsaWNfa2V5AAAAAAANAAAAIKCapfR6Z1mAL/lV+NwtKhSlyZ0j"
            "vpf4ZBJ/+Tg0VaTwAAAADwAAAAlzaWduYXR1cmUAAAAAAAANAAAAQEzLz5qhcnsoqd3WtcaPFbbNyuRwC5v9gP4vXIZ9SUSF"
            "oUeoeZpZAdkql8i1DenZ06pxyOR6LMC6s/6nnhssLwoAAAAAAAAAAaWWK0LQf0q/cOJjZctpx/IeYnOMrQua0Ky1y23vzd1D"
            "AAAAD3dlYl9hdXRoX3ZlcmlmeQAAAAABAAAAEQAAAAEAAAAHAAAADwAAAAdhY2NvdW50AAAAAA4AAAA4Q0JVQ0pNSEJaSFEz"
            "RVhRMkxNU0ZWWlVXQ1BIN0JDVENZR09RNkxJSU8yT1VWS1UzWERET08ySE4AAAAPAAAADWNsaWVudF9kb21haW4AAAAAAAAO"
            "AAAAEndhbGxldC5leGFtcGxlLm9yZwAAAAAADwAAABVjbGllbnRfZG9tYWluX2FjY291bnQAAAAAAAAOAAAAOEdESUVWTVJT"
            "T1FWM0pLWjJDTlVMMlJRVjRUVE5BSVNLVzROQUMyNVBRVVFLR01XSk82RFRPQUU3AAAADwAAAAtob21lX2RvbWFpbgAAAAAO"
            "AAAAC2V4YW1wbGUuY29tAAAAAA8AAAAFbm9uY2UAAAAAAAAOAAAAQDBRN3RGZnA1aEwyb0p2MW45bVkzc1I4YVpjNldrWDRi"
            "VWUwZEdpVC8reXBvUTFsTndWckU1S3M3akh1TWYyQXgAAAAPAAAAD3dlYl9hdXRoX2RvbWFpbgAAAAAOAAAAEGF1dGguZXhh"
            "bXBsZS5jb20AAAAPAAAAF3dlYl9hdXRoX2RvbWFpbl9hY2NvdW50AAAAAA4AAAA4R0NTTlJDVU02RURFS1NTQlFOSU9QNjZP"
            "TklNMjZGVkNZUDNHSFlHRDROUjNESzRGNjM1WjMyV1EAAAAAAAAAAQAAAAAAAAAApNiKjPEGRUpBg1Dn+85qGa8WosP2Y+DD"
            "42Oxq4X2+508ErpFoj2T9gABQW4AAAAQAAAAAQAAAAEAAAARAAAAAQAAAAIAAAAPAAAACnB1YmxpY19rZXkAAAAAAA0AAAAg"
            "pNiKjPEGRUpBg1Dn+85qGa8WosP2Y+DD42Oxq4X2+50AAAAPAAAACXNpZ25hdHVyZQAAAAAAAA0AAABAH/sy9/rJv8aqg9pe"
            "YvtOSFSKizm2eGrxiN4z5wXPTEtLAibNNGsjM0qYK3TpKJFk5F41Hf31pk36QBspCFvPDwAAAAAAAAABpZYrQtB/Sr9w4mNl"
            "y2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVyaWZ5AAAAAAEAAAARAAAAAQAAAAcAAAAPAAAAB2FjY291bnQA"
            "AAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAAN"
            "Y2xpZW50X2RvbWFpbgAAAAAAAA4AAAASd2FsbGV0LmV4YW1wbGUub3JnAAAAAAAPAAAAFWNsaWVudF9kb21haW5fYWNjb3Vu"
            "dAAAAAAAAA4AAAA4R0RJRVZNUlNPUVYzSktaMkNOVUwyUlFWNFRUTkFJU0tXNE5BQzI1UFFVUUtHTVdKTzZEVE9BRTcAAAAP"
            "AAAAC2hvbWVfZG9tYWluAAAAAA4AAAALZXhhbXBsZS5jb20AAAAADwAAAAVub25jZQAAAAAAAA4AAABAMFE3dEZmcDVoTDJv"
            "SnYxbjltWTNzUjhhWmM2V2tYNGJVZTBkR2lULyt5cG9RMWxOd1ZyRTVLczdqSHVNZjJBeAAAAA8AAAAPd2ViX2F1dGhfZG9t"
            "YWluAAAAAA4AAAAQYXV0aC5leGFtcGxlLmNvbQAAAA8AAAAXd2ViX2F1dGhfZG9tYWluX2FjY291bnQAAAAADgAAADhHQ1NO"
            "UkNVTTZFREVLU1NCUU5JT1A2Nk9OSU0yNkZWQ1lQM0dIWUdENE5SM0RLNEY2MzVaMzJXUQAAAAAAAAABAAAAAAAAAADQSrIy"
            "dCu0qzoTaL1GFeTm0CJKtxoBa6+FIKMyyXeHNyLdst2ur+GZAAFBGAAAABAAAAABAAAAAQAAABEAAAABAAAAAgAAAA8AAAAK"
            "cHVibGljX2tleQAAAAAADQAAACDQSrIydCu0qzoTaL1GFeTm0CJKtxoBa6+FIKMyyXeHNwAAAA8AAAAJc2lnbmF0dXJlAAAA"
            "AAAADQAAAEAz/GlQdo44tNzUXwNjUjJy1B5vctpr6456Ay4VW7mvOi1DNqKKs0iJVszrYrNVH/C3Mt8FFASSSKMdAKqf7vsN"
            "AAAAAAAAAAGllitC0H9Kv3DiY2XLacfyHmJzjK0LmtCstctt783dQwAAAA93ZWJfYXV0aF92ZXJpZnkAAAAAAQAAABEAAAAB"
            "AAAABwAAAA8AAAAHYWNjb3VudAAAAAAOAAAAOENCVUNKTUhCWkhRM0VYUTJMTVNGVlpVV0NQSDdCQ1RDWUdPUTZMSUlPMk9V"
            "VktVM1hERE9PMkhOAAAADwAAAA1jbGllbnRfZG9tYWluAAAAAAAADgAAABJ3YWxsZXQuZXhhbXBsZS5vcmcAAAAAAA8AAAAV"
            "Y2xpZW50X2RvbWFpbl9hY2NvdW50AAAAAAAADgAAADhHRElFVk1SU09RVjNKS1oyQ05VTDJSUVY0VFROQUlTS1c0TkFDMjVQ"
            "UVVRS0dNV0pPNkRUT0FFNwAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAPAAAABW5vbmNlAAAA"
            "AAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQvK3lwb1ExbE53VnJFNUtzN2pIdU1mMkF4"
            "AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJfYXV0aF9kb21haW5f"
            "YWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIzREs0RjYzNVozMldR"
            "AAAAAAAAAAAAAAAA");
    }

    static QString pyNChallengeTx() {
        return QStringLiteral(
            "AAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAGQAAAAAAAAAAQAAAAEAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAEAAAAAAAAAGAAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVy"
            "aWZ5AAAAAAEAAAARAAAAAQAAAAUAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3"
            "QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAP"
            "AAAABW5vbmNlAAAAAAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQvK3lwb1ExbE53VnJF"
            "NUtzN2pIdU1mMkF4AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJf"
            "YXV0aF9kb21haW5fYWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIz"
            "REs0RjYzNVozMldRAAAAAAAAAAAAAAAA");
    }

    static QStringList pyNSimulatedAuth() {
        return QStringList{
            QStringLiteral("AAAAAQAAAAFoJLDhyeGyXhpbJFrmlhPP8IpiwZ0PLQh2nUqqm7jG5+7d7wuCFn7rAAAAAAAAAAEAAAAAAAAAAaWWK0LQf0q/"
                           "cOJjZctpx/IeYnOMrQua0Ky1y23vzd1DAAAAD3dlYl9hdXRoX3ZlcmlmeQAAAAABAAAAEQAAAAEAAAAFAAAADwAAAAdhY2Nv"
                           "dW50AAAAAA4AAAA4Q0JVQ0pNSEJaSFEzRVhRMkxNU0ZWWlVXQ1BIN0JDVENZR09RNkxJSU8yT1VWS1UzWERET08ySE4AAAAP"
                           "AAAAC2hvbWVfZG9tYWluAAAAAA4AAAALZXhhbXBsZS5jb20AAAAADwAAAAVub25jZQAAAAAAAA4AAABAMFE3dEZmcDVoTDJv"
                           "SnYxbjltWTNzUjhhWmM2V2tYNGJVZTBkR2lULyt5cG9RMWxOd1ZyRTVLczdqSHVNZjJBeAAAAA8AAAAPd2ViX2F1dGhfZG9t"
                           "YWluAAAAAA4AAAAQYXV0aC5leGFtcGxlLmNvbQAAAA8AAAAXd2ViX2F1dGhfZG9tYWluX2FjY291bnQAAAAADgAAADhHQ1NO"
                           "UkNVTTZFREVLU1NCUU5JT1A2Nk9OSU0yNkZWQ1lQM0dIWUdENE5SM0RLNEY2MzVaMzJXUQAAAAA="),
            QStringLiteral("AAAAAQAAAAAAAAAApNiKjPEGRUpBg1Dn+85qGa8WosP2Y+DD42Oxq4X2+50AAAAAOt5osQAAAAAAAAABAAAAAAAAAAGllitC"
                           "0H9Kv3DiY2XLacfyHmJzjK0LmtCstctt783dQwAAAA93ZWJfYXV0aF92ZXJpZnkAAAAAAQAAABEAAAABAAAABQAAAA8AAAAH"
                           "YWNjb3VudAAAAAAOAAAAOENCVUNKTUhCWkhRM0VYUTJMTVNGVlpVV0NQSDdCQ1RDWUdPUTZMSUlPMk9VVktVM1hERE9PMkhO"
                           "AAAADwAAAAtob21lX2RvbWFpbgAAAAAOAAAAC2V4YW1wbGUuY29tAAAAAA8AAAAFbm9uY2UAAAAAAAAOAAAAQDBRN3RGZnA1"
                           "aEwyb0p2MW45bVkzc1I4YVpjNldrWDRiVWUwZEdpVC8reXBvUTFsTndWckU1S3M3akh1TWYyQXgAAAAPAAAAD3dlYl9hdXRo"
                           "X2RvbWFpbgAAAAAOAAAAEGF1dGguZXhhbXBsZS5jb20AAAAPAAAAF3dlYl9hdXRoX2RvbWFpbl9hY2NvdW50AAAAAA4AAAA4"
                           "R0NTTlJDVU02RURFS1NTQlFOSU9QNjZPTklNMjZGVkNZUDNHSFlHRDROUjNESzRGNjM1WjMyV1EAAAAA"),
        };
    }

    static QString pyNBuilt() {
        return QStringLiteral(
            "AAAAAgAAAAEAAAABaCSw4cnhsl4aWyRa5pYTz/CKYsGdDy0Idp1Kqpu4xufu3e8LghZ+6wAAAAAAAAABAAAAAAAAAAGllitC"
            "0H9Kv3DiY2XLacfyHmJzjK0LmtCstctt783dQwAAAA93ZWJfYXV0aF92ZXJpZnkAAAAAAQAAABEAAAABAAAABQAAAA8AAAAH"
            "YWNjb3VudAAAAAAOAAAAOENCVUNKTUhCWkhRM0VYUTJMTVNGVlpVV0NQSDdCQ1RDWUdPUTZMSUlPMk9VVktVM1hERE9PMkhO"
            "AAAADwAAAAtob21lX2RvbWFpbgAAAAAOAAAAC2V4YW1wbGUuY29tAAAAAA8AAAAFbm9uY2UAAAAAAAAOAAAAQDBRN3RGZnA1"
            "aEwyb0p2MW45bVkzc1I4YVpjNldrWDRiVWUwZEdpVC8reXBvUTFsTndWckU1S3M3akh1TWYyQXgAAAAPAAAAD3dlYl9hdXRo"
            "X2RvbWFpbgAAAAAOAAAAEGF1dGguZXhhbXBsZS5jb20AAAAPAAAAF3dlYl9hdXRoX2RvbWFpbl9hY2NvdW50AAAAAA4AAAA4"
            "R0NTTlJDVU02RURFS1NTQlFOSU9QNjZPTklNMjZGVkNZUDNHSFlHRDROUjNESzRGNjM1WjMyV1EAAAAAAAAAAQAAAAAAAAAA"
            "pNiKjPEGRUpBg1Dn+85qGa8WosP2Y+DD42Oxq4X2+50AAAAAOt5osQABQW4AAAAQAAAAAQAAAAEAAAARAAAAAQAAAAIAAAAP"
            "AAAACnB1YmxpY19rZXkAAAAAAA0AAAAgpNiKjPEGRUpBg1Dn+85qGa8WosP2Y+DD42Oxq4X2+50AAAAPAAAACXNpZ25hdHVy"
            "ZQAAAAAAAA0AAABApp9+7UOIFrH7urEsTG/+SoWMQdqnpbLRffMDGHfA/nj6w5En34oDbGoWCPCJUQrYgpaTuYnU0H3mTCi7"
            "mgxzCgAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVyaWZ5AAAAAAEAAAAR"
            "AAAAAQAAAAUAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3QkNUQ1lHT1E2TElJ"
            "TzJPVVZLVTNYRERPTzJITgAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAPAAAABW5vbmNlAAAA"
            "AAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQvK3lwb1ExbE53VnJFNUtzN2pIdU1mMkF4"
            "AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJfYXV0aF9kb21haW5f"
            "YWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIzREs0RjYzNVozMldR"
            "AAAAAA==");
    }

    static QString pyNSigned() {
        return QStringLiteral(
            "AAAAAgAAAAEAAAABaCSw4cnhsl4aWyRa5pYTz/CKYsGdDy0Idp1Kqpu4xufu3e8LghZ+6wABQRgAAAAQAAAAAQAAAAEAAAAR"
            "AAAAAQAAAAIAAAAPAAAACnB1YmxpY19rZXkAAAAAAA0AAAAgoJql9HpnWYAv+VX43C0qFKXJnSO+l/hkEn/5ODRVpPAAAAAP"
            "AAAACXNpZ25hdHVyZQAAAAAAAA0AAABAaH0lmbS7DL5PMT1Snr2KDd4yRDVqvAywAgPaDBrcM6CHHYztxCSWuu5eiNP7Gkb5"
            "xIhDpRMG7Sa+PmgFXtBoDAAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVy"
            "aWZ5AAAAAAEAAAARAAAAAQAAAAUAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3"
            "QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAP"
            "AAAABW5vbmNlAAAAAAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQvK3lwb1ExbE53VnJF"
            "NUtzN2pIdU1mMkF4AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJf"
            "YXV0aF9kb21haW5fYWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIz"
            "REs0RjYzNVozMldRAAAAAAAAAAEAAAAAAAAAAKTYiozxBkVKQYNQ5/vOahmvFqLD9mPgw+NjsauF9vudAAAAADreaLEAAUFu"
            "AAAAEAAAAAEAAAABAAAAEQAAAAEAAAACAAAADwAAAApwdWJsaWNfa2V5AAAAAAANAAAAIKTYiozxBkVKQYNQ5/vOahmvFqLD"
            "9mPgw+NjsauF9vudAAAADwAAAAlzaWduYXR1cmUAAAAAAAANAAAAQKaffu1DiBax+7qxLExv/kqFjEHap6Wy0X3zAxh3wP54"
            "+sORJ9+KA2xqFgjwiVEK2IKWk7mJ1NB95kwou5oMcwoAAAAAAAAAAaWWK0LQf0q/cOJjZctpx/IeYnOMrQua0Ky1y23vzd1D"
            "AAAAD3dlYl9hdXRoX3ZlcmlmeQAAAAABAAAAEQAAAAEAAAAFAAAADwAAAAdhY2NvdW50AAAAAA4AAAA4Q0JVQ0pNSEJaSFEz"
            "RVhRMkxNU0ZWWlVXQ1BIN0JDVENZR09RNkxJSU8yT1VWS1UzWERET08ySE4AAAAPAAAAC2hvbWVfZG9tYWluAAAAAA4AAAAL"
            "ZXhhbXBsZS5jb20AAAAADwAAAAVub25jZQAAAAAAAA4AAABAMFE3dEZmcDVoTDJvSnYxbjltWTNzUjhhWmM2V2tYNGJVZTBk"
            "R2lULyt5cG9RMWxOd1ZyRTVLczdqSHVNZjJBeAAAAA8AAAAPd2ViX2F1dGhfZG9tYWluAAAAAA4AAAAQYXV0aC5leGFtcGxl"
            "LmNvbQAAAA8AAAAXd2ViX2F1dGhfZG9tYWluX2FjY291bnQAAAAADgAAADhHQ1NOUkNVTTZFREVLU1NCUU5JT1A2Nk9OSU0y"
            "NkZWQ1lQM0dIWUdENE5SM0RLNEY2MzVaMzJXUQAAAAA=");
    }

    static QString pyNVerifyTx() {
        return QStringLiteral(
            "AAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAGQAAAAAAAAAAQAAAAEAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAEAAAAAAAAAGAAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVy"
            "aWZ5AAAAAAEAAAARAAAAAQAAAAUAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3"
            "QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAP"
            "AAAABW5vbmNlAAAAAAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQvK3lwb1ExbE53VnJF"
            "NUtzN2pIdU1mMkF4AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJf"
            "YXV0aF9kb21haW5fYWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIz"
            "REs0RjYzNVozMldRAAAAAgAAAAEAAAABaCSw4cnhsl4aWyRa5pYTz/CKYsGdDy0Idp1Kqpu4xufu3e8LghZ+6wABQRgAAAAQ"
            "AAAAAQAAAAEAAAARAAAAAQAAAAIAAAAPAAAACnB1YmxpY19rZXkAAAAAAA0AAAAgoJql9HpnWYAv+VX43C0qFKXJnSO+l/hk"
            "En/5ODRVpPAAAAAPAAAACXNpZ25hdHVyZQAAAAAAAA0AAABAaH0lmbS7DL5PMT1Snr2KDd4yRDVqvAywAgPaDBrcM6CHHYzt"
            "xCSWuu5eiNP7Gkb5xIhDpRMG7Sa+PmgFXtBoDAAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAP"
            "d2ViX2F1dGhfdmVyaWZ5AAAAAAEAAAARAAAAAQAAAAUAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEy"
            "TE1TRlZaVVdDUEg3QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFt"
            "cGxlLmNvbQAAAAAPAAAABW5vbmNlAAAAAAAADgAAAEAwUTd0RmZwNWhMMm9KdjFuOW1ZM3NSOGFaYzZXa1g0YlVlMGRHaVQv"
            "K3lwb1ExbE53VnJFNUtzN2pIdU1mMkF4AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29t"
            "AAAADwAAABd3ZWJfYXV0aF9kb21haW5fYWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZD"
            "WVAzR0hZR0Q0TlIzREs0RjYzNVozMldRAAAAAAAAAAEAAAAAAAAAAKTYiozxBkVKQYNQ5/vOahmvFqLD9mPgw+NjsauF9vud"
            "AAAAADreaLEAAUFuAAAAEAAAAAEAAAABAAAAEQAAAAEAAAACAAAADwAAAApwdWJsaWNfa2V5AAAAAAANAAAAIKTYiozxBkVK"
            "QYNQ5/vOahmvFqLD9mPgw+NjsauF9vudAAAADwAAAAlzaWduYXR1cmUAAAAAAAANAAAAQKaffu1DiBax+7qxLExv/kqFjEHa"
            "p6Wy0X3zAxh3wP54+sORJ9+KA2xqFgjwiVEK2IKWk7mJ1NB95kwou5oMcwoAAAAAAAAAAaWWK0LQf0q/cOJjZctpx/IeYnOM"
            "rQua0Ky1y23vzd1DAAAAD3dlYl9hdXRoX3ZlcmlmeQAAAAABAAAAEQAAAAEAAAAFAAAADwAAAAdhY2NvdW50AAAAAA4AAAA4"
            "Q0JVQ0pNSEJaSFEzRVhRMkxNU0ZWWlVXQ1BIN0JDVENZR09RNkxJSU8yT1VWS1UzWERET08ySE4AAAAPAAAAC2hvbWVfZG9t"
            "YWluAAAAAA4AAAALZXhhbXBsZS5jb20AAAAADwAAAAVub25jZQAAAAAAAA4AAABAMFE3dEZmcDVoTDJvSnYxbjltWTNzUjhh"
            "WmM2V2tYNGJVZTBkR2lULyt5cG9RMWxOd1ZyRTVLczdqSHVNZjJBeAAAAA8AAAAPd2ViX2F1dGhfZG9tYWluAAAAAA4AAAAQ"
            "YXV0aC5leGFtcGxlLmNvbQAAAA8AAAAXd2ViX2F1dGhfZG9tYWluX2FjY291bnQAAAAADgAAADhHQ1NOUkNVTTZFREVLU1NC"
            "UU5JT1A2Nk9OSU0yNkZWQ1lQM0dIWUdENE5SM0RLNEY2MzVaMzJXUQAAAAAAAAAAAAAAAA==");
    }

    static QStringList rpcRecordedAuth() {
        return QStringList{
            QStringLiteral("AAAAAQAAAAFoJLDhyeGyXhpbJFrmlhPP8IpiwZ0PLQh2nUqqm7jG51VZ+TqakiUNAAAAAAAAAAEAAAAAAAAAAaWWK0LQf0q/"
                           "cOJjZctpx/IeYnOMrQua0Ky1y23vzd1DAAAAD3dlYl9hdXRoX3ZlcmlmeQAAAAABAAAAEQAAAAEAAAAFAAAADwAAAAdhY2Nv"
                           "dW50AAAAAA4AAAA4Q0JVQ0pNSEJaSFEzRVhRMkxNU0ZWWlVXQ1BIN0JDVENZR09RNkxJSU8yT1VWS1UzWERET08ySE4AAAAP"
                           "AAAAC2hvbWVfZG9tYWluAAAAAA4AAAALZXhhbXBsZS5jb20AAAAADwAAAAVub25jZQAAAAAAAA4AAABAWkY4R0wzT21kMXdh"
                           "K0IrbTB0aE5SZkNBeVBydzduL1pEQXVTMzE1VHNVeXVoTTJDRzVzUzkxTzluaWFoTTJuWAAAAA8AAAAPd2ViX2F1dGhfZG9t"
                           "YWluAAAAAA4AAAAQYXV0aC5leGFtcGxlLmNvbQAAAA8AAAAXd2ViX2F1dGhfZG9tYWluX2FjY291bnQAAAAADgAAADhHQ1NO"
                           "UkNVTTZFREVLU1NCUU5JT1A2Nk9OSU0yNkZWQ1lQM0dIWUdENE5SM0RLNEY2MzVaMzJXUQAAAAA="),
            QStringLiteral("AAAAAQAAAAAAAAAApNiKjPEGRUpBg1Dn+85qGa8WosP2Y+DD42Oxq4X2+50ve13p7f5W7AAAAAAAAAABAAAAAAAAAAGllitC"
                           "0H9Kv3DiY2XLacfyHmJzjK0LmtCstctt783dQwAAAA93ZWJfYXV0aF92ZXJpZnkAAAAAAQAAABEAAAABAAAABQAAAA8AAAAH"
                           "YWNjb3VudAAAAAAOAAAAOENCVUNKTUhCWkhRM0VYUTJMTVNGVlpVV0NQSDdCQ1RDWUdPUTZMSUlPMk9VVktVM1hERE9PMkhO"
                           "AAAADwAAAAtob21lX2RvbWFpbgAAAAAOAAAAC2V4YW1wbGUuY29tAAAAAA8AAAAFbm9uY2UAAAAAAAAOAAAAQFpGOEdMM09t"
                           "ZDF3YStCK20wdGhOUmZDQXlQcnc3bi9aREF1UzMxNVRzVXl1aE0yQ0c1c1M5MU85bmlhaE0yblgAAAAPAAAAD3dlYl9hdXRo"
                           "X2RvbWFpbgAAAAAOAAAAEGF1dGguZXhhbXBsZS5jb20AAAAPAAAAF3dlYl9hdXRoX2RvbWFpbl9hY2NvdW50AAAAAA4AAAA4"
                           "R0NTTlJDVU02RURFS1NTQlFOSU9QNjZPTklNMjZGVkNZUDNHSFlHRDROUjNESzRGNjM1WjMyV1EAAAAA"),
        };
    }

    static QString rpcRecordedBuilt() {
        return QStringLiteral(
            "AAAAAgAAAAEAAAABaCSw4cnhsl4aWyRa5pYTz/CKYsGdDy0Idp1Kqpu4xudVWfk6mpIlDQAAAAAAAAABAAAAAAAAAAGllitC"
            "0H9Kv3DiY2XLacfyHmJzjK0LmtCstctt783dQwAAAA93ZWJfYXV0aF92ZXJpZnkAAAAAAQAAABEAAAABAAAABQAAAA8AAAAH"
            "YWNjb3VudAAAAAAOAAAAOENCVUNKTUhCWkhRM0VYUTJMTVNGVlpVV0NQSDdCQ1RDWUdPUTZMSUlPMk9VVktVM1hERE9PMkhO"
            "AAAADwAAAAtob21lX2RvbWFpbgAAAAAOAAAAC2V4YW1wbGUuY29tAAAAAA8AAAAFbm9uY2UAAAAAAAAOAAAAQFpGOEdMM09t"
            "ZDF3YStCK20wdGhOUmZDQXlQcnc3bi9aREF1UzMxNVRzVXl1aE0yQ0c1c1M5MU85bmlhaE0yblgAAAAPAAAAD3dlYl9hdXRo"
            "X2RvbWFpbgAAAAAOAAAAEGF1dGguZXhhbXBsZS5jb20AAAAPAAAAF3dlYl9hdXRoX2RvbWFpbl9hY2NvdW50AAAAAA4AAAA4"
            "R0NTTlJDVU02RURFS1NTQlFOSU9QNjZPTklNMjZGVkNZUDNHSFlHRDROUjNESzRGNjM1WjMyV1EAAAAAAAAAAQAAAAAAAAAA"
            "pNiKjPEGRUpBg1Dn+85qGa8WosP2Y+DD42Oxq4X2+50ve13p7f5W7AABQlYAAAAQAAAAAQAAAAEAAAARAAAAAQAAAAIAAAAP"
            "AAAACnB1YmxpY19rZXkAAAAAAA0AAAAgpNiKjPEGRUpBg1Dn+85qGa8WosP2Y+DD42Oxq4X2+50AAAAPAAAACXNpZ25hdHVy"
            "ZQAAAAAAAA0AAABAPpM7WHa4kcqQVLHvpwar5D8Dzm3FwOtJz9U7/tXfGmikNSEHcrZyIYrwqBHayLMo28frgE0HIzCW3F8V"
            "wlJpBAAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVyaWZ5AAAAAAEAAAAR"
            "AAAAAQAAAAUAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3QkNUQ1lHT1E2TElJ"
            "TzJPVVZLVTNYRERPTzJITgAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAPAAAABW5vbmNlAAAA"
            "AAAADgAAAEBaRjhHTDNPbWQxd2ErQittMHRoTlJmQ0F5UHJ3N24vWkRBdVMzMTVUc1V5dWhNMkNHNXNTOTFPOW5pYWhNMm5Y"
            "AAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJfYXV0aF9kb21haW5f"
            "YWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIzREs0RjYzNVozMldR"
            "AAAAAA==");
    }

    static QString rpcRecordedRequest() {
        return QStringLiteral(
            "AAAAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAGQAAAAAAAAAAQAAAAEAAAAAAAAAAAAAAAAAAAAA"
            "AAAAAAAAAAEAAAAAAAAAGAAAAAAAAAABpZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAPd2ViX2F1dGhfdmVy"
            "aWZ5AAAAAAEAAAARAAAAAQAAAAUAAAAPAAAAB2FjY291bnQAAAAADgAAADhDQlVDSk1IQlpIUTNFWFEyTE1TRlZaVVdDUEg3"
            "QkNUQ1lHT1E2TElJTzJPVVZLVTNYRERPTzJITgAAAA8AAAALaG9tZV9kb21haW4AAAAADgAAAAtleGFtcGxlLmNvbQAAAAAP"
            "AAAABW5vbmNlAAAAAAAADgAAAEBaRjhHTDNPbWQxd2ErQittMHRoTlJmQ0F5UHJ3N24vWkRBdVMzMTVUc1V5dWhNMkNHNXNT"
            "OTFPOW5pYWhNMm5YAAAADwAAAA93ZWJfYXV0aF9kb21haW4AAAAADgAAABBhdXRoLmV4YW1wbGUuY29tAAAADwAAABd3ZWJf"
            "YXV0aF9kb21haW5fYWNjb3VudAAAAAAOAAAAOEdDU05SQ1VNNkVERUtTU0JRTklPUDY2T05JTTI2RlZDWVAzR0hZR0Q0TlIz"
            "REs0RjYzNVozMldRAAAAAAAAAAAAAAAA");
    }

    /** A simulateTransaction answer carrying `auth` for the one call. */
    static SimulateTransactionResponse simulationWith(const QStringList& auth, quint32 latestLedger)
    {
        QJsonObject result;
        result.insert("auth", QJsonArray::fromStringList(auth));
        result.insert("xdr", QStringLiteral("AAAAAQ=="));   // SCV_VOID: web_auth_verify's ()
        QJsonObject sim;
        sim.insert("latestLedger", qint64(latestLedger));
        sim.insert("results", QJsonArray{result});
        return SimulateTransactionResponse::fromJson(sim);
    }

    static QList<stellar::SorobanAuthorizationEntry> entriesOf(const QString& base64)
    {
        QByteArray data = QByteArray::fromBase64(base64.toLatin1());
        QDataStream in(&data, QIODevice::ReadOnly);
        qint32 count = 0;
        in >> count;
        QList<stellar::SorobanAuthorizationEntry> entries;
        for (qint32 i = 0; i < count; ++i) {
            stellar::SorobanAuthorizationEntry entry;
            in >> entry;
            entries.append(entry);
        }
        return entries;
    }

    /** The entries of a simulation's auth list, one base64 entry each. */
    static QList<stellar::SorobanAuthorizationEntry> simulatedEntriesOf(const QStringList& auth)
    {
        QList<stellar::SorobanAuthorizationEntry> entries;
        for (const QString& one : auth) {
            QByteArray data = QByteArray::fromBase64(one.toLatin1());
            QDataStream in(&data, QIODevice::ReadOnly);
            stellar::SorobanAuthorizationEntry entry;
            in >> entry;
            entries.append(entry);
        }
        return entries;
    }

    /** Whether `call` throws a std::exception (QVERIFY_THROWS_ANY is Qt 6 only). */
    static bool throwsStd(const std::function<void()>& call)
    {
        try {
            call();
            return false;
        } catch (const std::exception&) {
            return true;
        }
    }

    static QString encoded(const QList<stellar::SorobanAuthorizationEntry>& entries)
    {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out << qint32(entries.size());
        for (const auto& entry : entries)
            out << entry;
        return QString::fromLatin1(data.toBase64());
    }

    static QString addressOf(const stellar::SorobanAuthorizationEntry& entry)
    {
        return Scv::fromAddress(Scv::toAddress(entry.credentials.address.address)).toString();
    }

    /** The server's entry signed again, as a server would, so that a test
     *  refusal comes from what the test changed and not from a stale signature. */
    static QString resigned(QList<stellar::SorobanAuthorizationEntry> entries,
                            const QString& passphrase = Network::testnetNetwork()->getNetworkPassphrase())
    {
        QScopedPointer<KeyPair> server(KeyPair::fromSecretSeed(V_SERVER_SEED()));
        for (auto& entry : entries) {
            if (addressOf(entry) == V_SERVER())
                entry = SorobanCredentialsSigner::authorizeEntry(entry, server.data(),
                                                                 V_LATEST + 180, passphrase);
        }
        return encoded(entries);
    }

    /** web_auth_verify's argument with the given keys, sorted as the host wants it. */
    static stellar::SCVal argMap(const QList<QPair<QString, QString>>& pairs)
    {
        QList<stellar::SCMapEntry> entries;
        for (const auto& pair : pairs) {
            stellar::SCMapEntry entry;
            entry.key = Scv::toSymbol(pair.first);
            entry.val = Scv::toString(pair.second);
            entries.append(entry);
        }
        return Scv::toMap(entries);
    }

    static QList<QPair<QString, QString>> pyArgs()
    {
        return {{"account", V_CLIENT()}, {"client_domain", V_CLIENT_DOMAIN()},
                {"client_domain_account", V_DOMAIN_ACCOUNT()}, {"home_domain", V_HOME()},
                {"nonce", V_NONCE()}, {"web_auth_domain", V_WEB()},
                {"web_auth_domain_account", V_SERVER()}};
    }

    /** py's challenge with every entry authorizing `call` instead, re-signed. */
    static QString withCall(const stellar::InvokeContractArgs& call)
    {
        QList<stellar::SorobanAuthorizationEntry> entries = entriesOf(pyDBuilt());
        for (auto& entry : entries) {
            entry.rootInvocation.function.contractFn = call;
            entry.rootInvocation.subInvocations.reset();
        }
        return resigned(entries);
    }

    static stellar::InvokeContractArgs pyCall()
    {
        return entriesOf(pyDBuilt()).first().rootInvocation.function.contractFn;
    }

    /** Why readChallengeAuthorizationEntries refused `entries` with py's
     *  expectations (or those given); empty if it read them. */
    static QString readRefusal(const QString& entries, const QString& server = V_SERVER(),
                               const QStringList& homes = QStringList{V_HOME()},
                               const QString& web = V_WEB(), const QString& contract = V_WEB_AUTH(),
                               Network* network = Network::testnetNetwork())
    {
        try {
            Sep45Challenge::readChallengeAuthorizationEntries(entries, server, homes, web, contract, network);
            return QString();
        } catch (const qstellar::exception::InvalidSep45ChallengeException& e) {
            return QString::fromUtf8(e.what());
        }
    }

private slots:
    /** What a server simulates to build a challenge is py's transaction, byte
     *  for byte, with and without a client domain. (This SDK had no SEP-45
     *  v0.1.1 at all: its challenges were its own format, read by nothing else.) */
    void testV011ChallengeTransactionIsPys()
    {
        QCOMPARE(Sep45Challenge::challengeTransaction(V_SERVER(), V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(),
                                                      Network::testnetNetwork(), V_NONCE(),
                                                      V_CLIENT_DOMAIN(), V_DOMAIN_ACCOUNT()),
                 pyDChallengeTx());
        QCOMPARE(Sep45Challenge::challengeTransaction(V_SERVER(), V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(),
                                                      Network::testnetNetwork(), V_NONCE()),
                 pyNChallengeTx());
    }

    /** From the same simulation, the server builds the challenge py builds:
     *  the same entries, the server's signed with the same signature, valid
     *  until the same ledger. A real RPC's answer included. */
    void testV011BuildSignsAsPyDoes()
    {
        QScopedPointer<KeyPair> server(KeyPair::fromSecretSeed(V_SERVER_SEED()));
        CapturingSep45SorobanServer rpc;
        rpc.sim = simulationWith(pyDSimulatedAuth(), V_LATEST);
        QCOMPARE(Sep45Challenge::buildChallengeAuthorizationEntries(
                     &rpc, server.data(), V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(),
                     Network::testnetNetwork(), V_NONCE(), 180, V_CLIENT_DOMAIN(), V_DOMAIN_ACCOUNT()),
                 pyDBuilt());
        QCOMPARE(rpc.requests, QStringList{pyDChallengeTx()});

        rpc.requests.clear();
        rpc.sim = simulationWith(pyNSimulatedAuth(), V_LATEST);
        QCOMPARE(Sep45Challenge::buildChallengeAuthorizationEntries(
                     &rpc, server.data(), V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(),
                     Network::testnetNetwork(), V_NONCE()),
                 pyNBuilt());
        QCOMPARE(rpc.requests, QStringList{pyNChallengeTx()});

        rpc.requests.clear();
        rpc.sim = simulationWith(rpcRecordedAuth(), 82338);
        QCOMPARE(Sep45Challenge::buildChallengeAuthorizationEntries(
                     &rpc, server.data(), V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(),
                     Network::testnetNetwork(),
                     QStringLiteral("ZF8GL3Omd1wa+B+m0thNRfCAyPrw7n/ZDAuS315TsUyuhM2CG5sS91O9niahM2nX")),
                 rpcRecordedBuilt());
        QCOMPARE(rpc.requests, QStringList{rpcRecordedRequest()});

        // Left out, the nonce is 48 random bytes in base64, as py's.
        rpc.requests.clear();
        rpc.sim = simulationWith(pyNSimulatedAuth(), V_LATEST);
        try {
            Sep45Challenge::buildChallengeAuthorizationEntries(&rpc, server.data(), V_CLIENT(), V_HOME(),
                                                               V_WEB(), V_WEB_AUTH(), Network::testnetNetwork());
        } catch (const std::runtime_error&) {
            // The canned simulation carries the fixed nonce, not this one.
        }
        QCOMPARE(rpc.requests.size(), 1);
        QVERIFY(rpc.requests.first() != pyNChallengeTx());
    }

    /**
     * py signs whatever entry the simulation puts the server's address on. A
     * compromised or mistaken RPC answering with an entry that authorizes a
     * transfer from the server's account got it signed; so did an extra
     * entry, a missing client, or a call with something nested under it. The
     * server now signs only the challenge it asked for.
     */
    void testV011BuildRefusesASimulationThatIsNotTheChallenge()
    {
        QScopedPointer<KeyPair> server(KeyPair::fromSecretSeed(V_SERVER_SEED()));
        auto issues = [&](const QList<stellar::SorobanAuthorizationEntry>& entries) {
            QStringList auth;
            for (const auto& entry : entries) {
                QByteArray data;
                QDataStream out(&data, QIODevice::WriteOnly);
                out << entry;
                auth.append(QString::fromLatin1(data.toBase64()));
            }
            try {
                Sep45Challenge::signSimulatedChallengeEntries(auth, V_LATEST, server.data(), V_CLIENT(),
                                                              V_HOME(), V_WEB(), V_WEB_AUTH(),
                                                              Network::testnetNetwork(), V_NONCE());
                return true;
            } catch (const std::runtime_error&) {
                return false;
            }
        };
        const QList<stellar::SorobanAuthorizationEntry> simulated = simulatedEntriesOf(pyNSimulatedAuth());
        QVERIFY(issues(simulated));

        // The server's entry authorizing a token transfer instead.
        QList<stellar::SorobanAuthorizationEntry> transfer = simulated;
        QScopedPointer<KeyPair> thief(KeyPair::random());
        transfer[1].rootInvocation.function.contractFn.contractAddress =
            Scv::toAddress(contractStrKey('\x33')).address;
        transfer[1].rootInvocation.function.contractFn.functionName = "transfer";
        transfer[1].rootInvocation.function.contractFn.args =
            QList<stellar::SCVal>() << Scv::toAddress(V_SERVER()) << Scv::toAddress(thief->getAccountId())
                                    << Scv::toInt128(qint64(1000000000));
        QVERIFY(!issues(transfer));

        // Something nested under the call.
        QList<stellar::SorobanAuthorizationEntry> nested = simulated;
        nested[1].rootInvocation.subInvocations =
            QSharedPointer<QList<stellar::SorobanAuthorizedInvocation>>::create();
        nested[1].rootInvocation.subInvocations->append(transfer[1].rootInvocation);
        QVERIFY(!issues(nested));

        // An entry for somebody else, a repeat, a missing client, one signed already.
        QList<stellar::SorobanAuthorizationEntry> stranger = simulated;
        stranger.append(simulated.at(1));
        stranger.last().credentials.address.address = Scv::toAddress(thief->getAccountId()).address;
        QVERIFY(!issues(stranger));
        QList<stellar::SorobanAuthorizationEntry> twice = simulated;
        twice.append(simulated.at(0));
        QVERIFY(!issues(twice));
        QVERIFY(!issues({simulated.at(1)}));
        QList<stellar::SorobanAuthorizationEntry> presigned = simulated;
        presigned[0].credentials.address.signature = Scv::toUint32(7);
        QVERIFY(!issues(presigned));
        QList<stellar::SorobanAuthorizationEntry> source = simulated;
        source[0].credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_SOURCE_ACCOUNT;
        QVERIFY(!issues(source));

        // A ledger past which no signature can last, and a nonce the call
        // did not carry.
        QStringList auth = pyNSimulatedAuth();
        QVERIFY(throwsStd([&] { Sep45Challenge::signSimulatedChallengeEntries(
            auth, 0xFFFFFFF0u, server.data(), V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(),
            Network::testnetNetwork(), V_NONCE()); }));
        QVERIFY(throwsStd([&] { Sep45Challenge::signSimulatedChallengeEntries(
            auth, V_LATEST, server.data(), V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(),
            Network::testnetNetwork(), QStringLiteral("another nonce")); }));
        QVERIFY(throwsStd([&] { Sep45Challenge::signSimulatedChallengeEntries(
            auth, V_LATEST, server.data(), V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(),
            Network::testnetNetwork(), V_NONCE(), 0); }));

        // A failed simulation, or one with nothing in it, builds nothing.
        CapturingSep45SorobanServer rpc;
        QJsonObject failed;
        failed.insert("latestLedger", 1);
        failed.insert("error", QStringLiteral("HostError: contract not found"));
        rpc.sim = SimulateTransactionResponse::fromJson(failed);
        QVERIFY(throwsStd([&] { Sep45Challenge::buildChallengeAuthorizationEntries(
            &rpc, server.data(), V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(),
            Network::testnetNetwork(), V_NONCE()); }));
        rpc.sim = simulationWith(QStringList(), V_LATEST);
        QVERIFY(throwsStd([&] { Sep45Challenge::buildChallengeAuthorizationEntries(
            &rpc, server.data(), V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(),
            Network::testnetNetwork(), V_NONCE()); }));
    }

    /** What a server issues without an RPC is py's challenge but for the
     *  credential nonces a simulation would have drawn: fresh each time. */
    void testV011OfflineBuildIsPysButForTheCredentialNonces()
    {
        QScopedPointer<KeyPair> server(KeyPair::fromSecretSeed(V_SERVER_SEED()));
        const QString offline = Sep45Challenge::buildChallengeAuthorizationEntries(
            server.data(), V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(), Network::testnetNetwork(),
            V_LATEST, V_NONCE(), 180, V_CLIENT_DOMAIN(), V_DOMAIN_ACCOUNT());
        QCOMPARE(readRefusal(offline), QString());
        QList<stellar::SorobanAuthorizationEntry> entries = entriesOf(offline);
        const QList<stellar::SorobanAuthorizationEntry> py = entriesOf(pyDBuilt());
        QCOMPARE(entries.size(), py.size());
        for (int i = 0; i < entries.size(); ++i) {
            QVERIFY(entries.at(i).credentials.address.nonce != py.at(i).credentials.address.nonce);
            entries[i].credentials.address.nonce = py.at(i).credentials.address.nonce;
        }
        QCOMPARE(resigned(entries), pyDBuilt());

        const QString again = Sep45Challenge::buildChallengeAuthorizationEntries(
            server.data(), V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(), Network::testnetNetwork(),
            V_LATEST, V_NONCE());
        QVERIFY(entriesOf(again).first().credentials.address.nonce
                != entriesOf(offline).first().credentials.address.nonce);
        // A random nonce, when none is given, of py's shape.
        const auto read = Sep45Challenge::readChallengeAuthorizationEntries(
            Sep45Challenge::buildChallengeAuthorizationEntries(server.data(), V_CLIENT(), V_HOME(), V_WEB(),
                                                               V_WEB_AUTH(), Network::testnetNetwork(), V_LATEST),
            V_SERVER(), {V_HOME()}, V_WEB(), V_WEB_AUTH(), Network::testnetNetwork());
        QCOMPARE(QByteArray::fromBase64(read.nonce.toLatin1()).size(), 48);
        QCOMPARE(read.serverSignatureExpirationLedger, V_LATEST + 180);
    }

    /** The server refuses to issue what no reader accepts. */
    void testV011BuildRefusesArgumentsNoReaderAccepts()
    {
        QScopedPointer<KeyPair> server(KeyPair::fromSecretSeed(V_SERVER_SEED()));
        auto builds = [&](const QString& client, const QString& home, const QString& web,
                          const QString& contract, const QString& domain, const QString& domainAccount) {
            try {
                Sep45Challenge::buildChallengeAuthorizationEntries(server.data(), client, home, web, contract,
                                                                   Network::testnetNetwork(), V_LATEST,
                                                                   V_NONCE(), 180, domain, domainAccount);
                return true;
            } catch (const std::runtime_error&) {
                return false;
            }
        };
        QVERIFY(builds(V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(), QString(), QString()));
        QVERIFY(!builds(V_SERVER(), V_HOME(), V_WEB(), V_WEB_AUTH(), QString(), QString()));
        QVERIFY(!builds(V_CLIENT(), QStringLiteral(" "), V_WEB(), V_WEB_AUTH(), QString(), QString()));
        QVERIFY(!builds(V_CLIENT(), V_HOME(), QString(), V_WEB_AUTH(), QString(), QString()));
        QVERIFY(!builds(V_CLIENT(), V_HOME(), V_WEB(), V_SERVER(), QString(), QString()));
        QVERIFY(!builds(V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(), V_CLIENT_DOMAIN(), QString()));
        QVERIFY(!builds(V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(), QString(), V_DOMAIN_ACCOUNT()));
        QVERIFY(!builds(V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(), V_CLIENT_DOMAIN(), V_CLIENT()));
        QVERIFY(!builds(V_CLIENT(), V_HOME(), V_WEB(), V_WEB_AUTH(), V_CLIENT_DOMAIN(), V_SERVER()));
    }

    /** A wallet reads py's challenge, with its client domain, and reports it. */
    void testV011WalletReadsWhatPyBuilds()
    {
        const auto read = Sep45Challenge::readChallengeAuthorizationEntries(
            pyDBuilt(), V_SERVER(), {QStringLiteral("other.example"), V_HOME()}, V_WEB(), V_WEB_AUTH(),
            Network::testnetNetwork());
        QCOMPARE(read.clientAccountId, V_CLIENT());
        QCOMPARE(read.matchedHomeDomain, V_HOME());
        QCOMPARE(read.nonce, V_NONCE());
        QCOMPARE(read.webAuthDomain, V_WEB());
        QCOMPARE(read.serverAccountId, V_SERVER());
        QCOMPARE(read.webAuthContract, V_WEB_AUTH());
        QCOMPARE(read.clientDomain, V_CLIENT_DOMAIN());
        QCOMPARE(read.clientDomainAccount, V_DOMAIN_ACCOUNT());
        QCOMPARE(read.serverSignatureExpirationLedger, V_LATEST + 180);
        QCOMPARE(read.authorizationEntries.size(), 3);

        const auto plain = Sep45Challenge::readChallengeAuthorizationEntries(
            pyNBuilt(), V_SERVER(), {V_HOME()}, V_WEB(), V_WEB_AUTH(), Network::testnetNetwork());
        QVERIFY(plain.clientDomain.isEmpty() && plain.clientDomainAccount.isEmpty());
        QCOMPARE(readRefusal(rpcRecordedBuilt()), QString());
        // The entries are not required to be in any order.
        QList<stellar::SorobanAuthorizationEntry> reversed = entriesOf(pyDBuilt());
        std::reverse(reversed.begin(), reversed.end());
        QCOMPARE(readRefusal(encoded(reversed)), QString());
    }

    /**
     * Signed by the wallet, the challenge is byte for byte what py's
     * authorize_entry makes of it: the client's entry, and the client
     * domain's when its key signs too; the others untouched.
     */
    void testV011WalletSignsAsPyDoes()
    {
        QScopedPointer<KeyPair> client(KeyPair::fromSecretSeed(V_CLIENT_SIGNER_SEED()));
        QScopedPointer<KeyPair> domain(KeyPair::fromSecretSeed(V_DOMAIN_SEED()));
        QCOMPARE(Sep45Challenge::signChallengeAuthorizationEntries(
                     pyDBuilt(), V_SERVER(), {V_HOME()}, V_WEB(), V_WEB_AUTH(), V_CLIENT(), client.data(),
                     V_VALID_UNTIL, Network::testnetNetwork(), V_CLIENT_DOMAIN(), V_DOMAIN_ACCOUNT(),
                     domain.data()),
                 pyDSigned());
        QCOMPARE(Sep45Challenge::signChallengeAuthorizationEntries(
                     pyDBuilt(), V_SERVER(), {V_HOME()}, V_WEB(), V_WEB_AUTH(), V_CLIENT(), client.data(),
                     V_VALID_UNTIL, Network::testnetNetwork()),
                 pyDClientOnlySigned());
        QCOMPARE(Sep45Challenge::signChallengeAuthorizationEntries(
                     pyNBuilt(), V_SERVER(), {V_HOME()}, V_WEB(), V_WEB_AUTH(), V_CLIENT(), client.data(),
                     V_VALID_UNTIL, Network::testnetNetwork()),
                 pyNSigned());
        // The client domain's backend signs its own entry alone, and the
        // wallet then adds the client's: py's result either way.
        const QString byDomain = Sep45Challenge::signChallengeAuthorizationEntries(
            pyDBuilt(), V_SERVER(), {V_HOME()}, V_WEB(), V_WEB_AUTH(), V_CLIENT(), nullptr, V_VALID_UNTIL,
            Network::testnetNetwork(), V_CLIENT_DOMAIN(), V_DOMAIN_ACCOUNT(), domain.data());
        QCOMPARE(Sep45Challenge::signChallengeAuthorizationEntries(
                     byDomain, V_SERVER(), {V_HOME()}, V_WEB(), V_WEB_AUTH(), V_CLIENT(), client.data(),
                     V_VALID_UNTIL, Network::testnetNetwork()),
                 pyDSigned());
    }

    /** Signing is refused, and nothing signed, unless the challenge is this
     *  wallet's and says what the wallet asked for. */
    void testV011WalletSignsOnlyItsOwnChallenge()
    {
        QScopedPointer<KeyPair> client(KeyPair::fromSecretSeed(V_CLIENT_SIGNER_SEED()));
        QScopedPointer<KeyPair> domain(KeyPair::fromSecretSeed(V_DOMAIN_SEED()));
        QScopedPointer<KeyPair> other(KeyPair::random());
        auto signs = [&](const QString& entries, const QString& clientAccount, KeyPair* signer, quint32 until,
                         const QString& clientDomain, const QString& domainAccount, KeyPair* domainSigner) {
            try {
                Sep45Challenge::signChallengeAuthorizationEntries(entries, V_SERVER(), {V_HOME()}, V_WEB(),
                                                                  V_WEB_AUTH(), clientAccount, signer, until,
                                                                  Network::testnetNetwork(), clientDomain,
                                                                  domainAccount, domainSigner);
                return true;
            } catch (const qstellar::exception::InvalidSep45ChallengeException&) {
                return false;
            }
        };
        QVERIFY(signs(pyDBuilt(), V_CLIENT(), client.data(), V_VALID_UNTIL, V_CLIENT_DOMAIN(),
                      V_DOMAIN_ACCOUNT(), nullptr));
        // Another contract's challenge.
        QVERIFY(!signs(pyDBuilt(), contractStrKey('\x44'), client.data(), V_VALID_UNTIL, QString(), QString(), nullptr));
        // No signer, no ledger, a G account as the client.
        QVERIFY(!signs(pyDBuilt(), V_CLIENT(), nullptr, V_VALID_UNTIL, QString(), QString(), nullptr));
        QVERIFY(!signs(pyDBuilt(), V_CLIENT(), client.data(), 0, QString(), QString(), nullptr));
        QVERIFY(!signs(pyDBuilt(), V_SERVER(), client.data(), V_VALID_UNTIL, QString(), QString(), nullptr));
        // Another client domain than the one asked for, or none at all.
        QVERIFY(!signs(pyDBuilt(), V_CLIENT(), client.data(), V_VALID_UNTIL, QStringLiteral("evil.example"),
                       V_DOMAIN_ACCOUNT(), nullptr));
        QVERIFY(!signs(pyDBuilt(), V_CLIENT(), client.data(), V_VALID_UNTIL, V_CLIENT_DOMAIN(),
                       other->getAccountId(), nullptr));
        QVERIFY(!signs(pyNBuilt(), V_CLIENT(), client.data(), V_VALID_UNTIL, V_CLIENT_DOMAIN(),
                       V_DOMAIN_ACCOUNT(), nullptr));
        // A client domain key that is not the domain's, or signing for no domain.
        QVERIFY(!signs(pyDBuilt(), V_CLIENT(), client.data(), V_VALID_UNTIL, V_CLIENT_DOMAIN(),
                       V_DOMAIN_ACCOUNT(), other.data()));
        QVERIFY(!signs(pyDBuilt(), V_CLIENT(), client.data(), V_VALID_UNTIL, QString(), QString(), domain.data()));
        QVERIFY(!signs(pyDBuilt(), V_CLIENT(), client.data(), V_VALID_UNTIL, V_CLIENT_DOMAIN(), QString(), nullptr));
        // And anything readChallengeAuthorizationEntries refuses.
        stellar::InvokeContractArgs other_call = pyCall();
        other_call.functionName = "transfer";
        QVERIFY(!signs(withCall(other_call), V_CLIENT(), client.data(), V_VALID_UNTIL, QString(), QString(), nullptr));
    }

    /**
     * A wallet refuses whatever is not a SEP-45 challenge from this server
     * for this call: another contract or function, arguments that differ
     * between entries or that the SEP does not define, something nested under
     * the call, a missing, extra or repeated entry, a server entry not signed
     * by the server on this network, and input that is not whole XDR.
     */
    void testV011ReadRefusesWhatIsNotAChallenge()
    {
        QCOMPARE(readRefusal(pyDBuilt()), QString());
        QCOMPARE(readRefusal(withCall(pyCall())), QString());

        // Another contract, another function, a second argument.
        stellar::InvokeContractArgs call = pyCall();
        call.contractAddress = Scv::toAddress(contractStrKey('\x33')).address;
        QCOMPARE(readRefusal(withCall(call)), QString("the challenge does not call the expected web-auth contract"));
        call = pyCall();
        call.functionName = "transfer";
        QCOMPARE(readRefusal(withCall(call)), QString("the challenge does not call web_auth_verify"));
        call = pyCall();
        call.args.append(Scv::toUint32(1));
        QCOMPARE(readRefusal(withCall(call)), QString("Expected exactly one argument in contract function call."));

        // Arguments: a key the SEP does not define, one missing, a value that
        // is not a string, a repeated key, keys out of the host's order.
        QList<QPair<QString, QString>> args = pyArgs();
        args.append({QStringLiteral("amount"), QStringLiteral("1000")});
        call = pyCall();
        call.args = {argMap(args)};
        QCOMPARE(readRefusal(withCall(call)), QString("the web_auth_verify argument has a key SEP-45 does not define"));
        args = pyArgs();
        args.removeAt(4);   // nonce
        call.args = {argMap(args)};
        QCOMPARE(readRefusal(withCall(call)), QString("Missing 'nonce' in arguments."));
        args = pyArgs();
        args.removeAt(2);   // client_domain_account, its domain left
        call.args = {argMap(args)};
        QVERIFY(!readRefusal(withCall(call)).isEmpty());
        stellar::SCVal map = argMap(pyArgs());
        QList<stellar::SCMapEntry> raw = *map.map;
        raw[4].val = Scv::toBytes(QByteArray("nonce"));
        stellar::SCVal edited;
        edited.type = stellar::SCValType::SCV_MAP;
        edited.map = QSharedPointer<QList<stellar::SCMapEntry>>::create(raw);
        call.args = {edited};
        QCOMPARE(readRefusal(withCall(call)), QString("every value of the web_auth_verify argument must be a string"));
        raw = *map.map;
        raw.swapItemsAt(0, 1);
        edited.map = QSharedPointer<QList<stellar::SCMapEntry>>::create(raw);
        call.args = {edited};
        QCOMPARE(readRefusal(withCall(call)), QString("the keys of the web_auth_verify argument must be sorted and without repeats"));
        raw = *map.map;
        raw.insert(1, raw.at(0));
        edited.map = QSharedPointer<QList<stellar::SCMapEntry>>::create(raw);
        call.args = {edited};
        QCOMPARE(readRefusal(withCall(call)), QString("the keys of the web_auth_verify argument must be sorted and without repeats"));
        // An account that is not a contract.
        args = pyArgs();
        args[0].second = V_DOMAIN_ACCOUNT();
        call.args = {argMap(args)};
        QCOMPARE(readRefusal(withCall(call)), QString("the account being authenticated must be a contract (C...)"));

        // The entries disagree: the client's authorizes another home domain.
        QList<stellar::SorobanAuthorizationEntry> entries = entriesOf(pyDBuilt());
        args = pyArgs();
        args[3].second = QStringLiteral("evil.example");
        entries[0].rootInvocation.function.contractFn.args = {argMap(args)};
        QCOMPARE(readRefusal(resigned(entries)), QString("Inconsistent root_invocation across authorization entries."));

        // Something nested under the call the client signs.
        entries = entriesOf(pyDBuilt());
        stellar::SorobanAuthorizedInvocation transfer;
        transfer.function.contractFn.contractAddress = Scv::toAddress(contractStrKey('\x33')).address;
        transfer.function.contractFn.functionName = "transfer";
        for (auto& entry : entries) {
            entry.rootInvocation.subInvocations =
                QSharedPointer<QList<stellar::SorobanAuthorizedInvocation>>::create();
            entry.rootInvocation.subInvocations->append(transfer);
        }
        QCOMPARE(readRefusal(resigned(entries)), QString("Authorization entry must not have sub-invocations."));

        // Missing, extra, repeated entries.
        const QList<stellar::SorobanAuthorizationEntry> py = entriesOf(pyDBuilt());
        QCOMPARE(readRefusal(encoded({py.at(0), py.at(2)})),
                 QString("Challenge does not contain an authorization entry for the server."));
        QCOMPARE(readRefusal(encoded({py.at(1), py.at(2)})),
                 QString("Challenge does not contain an authorization entry for the client."));
        QCOMPARE(readRefusal(encoded({py.at(0), py.at(1)})),
                 QString("Challenge does not contain an authorization entry for the client domain account."));
        QCOMPARE(readRefusal(encoded({py.at(1)})),
                 QString("Challenge must contain at least two authorization entries."));
        QScopedPointer<KeyPair> stranger(KeyPair::random());
        entries = py;
        entries.append(py.at(2));
        entries.last().credentials.address.address = Scv::toAddress(stranger->getAccountId()).address;
        QCOMPARE(readRefusal(encoded(entries)), QString("the challenge carries an entry for an address that is not a party to it"));
        entries = py;
        entries.append(py.at(0));
        QCOMPARE(readRefusal(encoded(entries)), QString("the challenge carries two entries for the client"));
        entries = py;
        entries.append(py.at(1));
        QCOMPARE(readRefusal(encoded(entries)), QString("the challenge carries two entries for the server"));

        // Credentials other than address ones (py knows no others either).
        entries = py;
        entries[0].credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_SOURCE_ACCOUNT;
        QCOMPARE(readRefusal(encoded(entries)), QString("every entry must carry address credentials"));
        entries = py;
        entries[1].credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2;
        QCOMPARE(readRefusal(resigned(entries)), QString("every entry must carry address credentials"));

        // The server's entry: unsigned, signed by somebody else, altered after
        // signing, signed for another network, signed twice over.
        entries = py;
        entries[1].credentials.address.signature = stellar::SCVal();
        QCOMPARE(readRefusal(encoded(entries)), QString("the server's entry is not signed by the server's signing key"));
        entries = py;
        entries[1] = SorobanCredentialsSigner::authorizeEntry(entries.at(1), stranger.data(), V_LATEST + 180,
                                                              Network::testnetNetwork()->getNetworkPassphrase());
        QCOMPARE(readRefusal(encoded(entries)), QString("the server's entry is not signed by the server's signing key"));
        entries = py;
        entries[1].credentials.address.nonce += 1;
        QVERIFY(readRefusal(encoded(entries)).startsWith("the server's signature on its entry does not verify"));
        entries = py;
        entries[1].credentials.address.signatureExpirationLedger += 1000;
        QVERIFY(readRefusal(encoded(entries)).startsWith("the server's signature on its entry does not verify"));
        QVERIFY(readRefusal(resigned(py, Network::publicNetwork()->getNetworkPassphrase()))
                    .startsWith("the server's signature on its entry does not verify"));
        QVERIFY(readRefusal(pyDBuilt(), V_SERVER(), {V_HOME()}, V_WEB(), V_WEB_AUTH(), Network::publicNetwork())
                    .startsWith("the server's signature on its entry does not verify"));
        entries = py;
        {
            const stellar::SCVal one = entries.at(1).credentials.address.signature.vec->first();
            entries[1].credentials.address.signature = Scv::toVec({one, one});
        }
        QCOMPARE(readRefusal(encoded(entries)), QString("the server's entry is not signed by the server's signing key"));

        // Expectations: another server, home, web-auth domain or contract.
        QCOMPARE(readRefusal(pyDBuilt(), stranger->getAccountId()),
                 QString("web_auth_domain_account is not the server's signing key"));
        QCOMPARE(readRefusal(pyDBuilt(), V_SERVER(), {QStringLiteral("example.org")}),
                 QString("home_domain is not one of the expected home domains"));
        QCOMPARE(readRefusal(pyDBuilt(), V_SERVER(), {V_HOME()}, QStringLiteral("Auth.example.com")),
                 QString("web_auth_domain is not the expected web-auth domain"));
        QCOMPARE(readRefusal(pyDBuilt(), V_SERVER(), {V_HOME()}, V_WEB(), contractStrKey('\x33')),
                 QString("the challenge does not call the expected web-auth contract"));
        QVERIFY(!readRefusal(pyDBuilt(), V_SERVER(), {V_HOME()}, QString()).isEmpty());
        QVERIFY(!readRefusal(pyDBuilt(), V_SERVER(), {V_HOME()}, V_WEB(), QString()).isEmpty());
        QVERIFY(!readRefusal(pyDBuilt(), V_SERVER(), {V_HOME()}, V_WEB(), V_WEB_AUTH(), nullptr).isEmpty());

        // Not whole XDR: trailing bytes, cut short, not base64, empty.
        QByteArray bytes = QByteArray::fromBase64(pyDBuilt().toLatin1());
        QVERIFY(!readRefusal(QString::fromLatin1((bytes + QByteArray(4, '\0')).toBase64())).isEmpty());
        QVERIFY(!readRefusal(QString::fromLatin1(bytes.left(bytes.size() - 8).toBase64())).isEmpty());
        QVERIFY(!readRefusal(pyDBuilt() + QStringLiteral("*")).isEmpty());
        QVERIFY(!readRefusal(QString()).isEmpty());
        // Wrapped over lines is still the challenge.
        QString wrapped = pyDBuilt();
        wrapped.insert(76, QLatin1Char('\n'));
        QCOMPARE(readRefusal(wrapped), QString());
    }

    /**
     * What every party signs is the payload js and py sign: for each entry of
     * py's signed challenges, the SHA-256 of its HashIdPreimage at the
     * entry's own expiration ledger, on testnet and on the public network, is
     * the hash js-stellar-sdk 17.1.0 (buildAuthorizationEntryPreimage) and
     * py-stellar-sdk 14.1.0 (auth.build_authorization_preimage,
     * authorization_payload_hash) compute, and the signature py put on the
     * entry is its signer's over that hash.
     */
    void testV011SignaturePayloadIsJsAndPys()
    {
        struct Payload {
            QString entries;
            int index;
            const char* signer;
            const char* testnet;
            const char* pubnet;
        };
        const QList<Payload> payloads = {
            {pyDSigned(), 0, "GCQJVJPUPJTVTABP7FK7RXBNFIKKLSM5EO7JP6DECJ77SOBUKWSPB64N",
             "c2a639d184fb814c562e0ed34bc3b6d325da936a9ee99c6ac41dc6cd040ec0f9",
             "948d4a64da641c5f6afc858ac60b6144b9fbf9d043d26995bac24fcd707645f0"},
            {pyDSigned(), 1, "GCSNRCUM6EDEKSSBQNIOP66ONIM26FVCYP3GHYGD4NR3DK4F635Z32WQ",
             "184b312c3d3e79d7b27fc051bced1f6d467b14febe38c32129a3cea6905afd27",
             "8317c5346354a0fd250aa8d76f371dca9c43dc1ec2b6c6ef8acc29ca34408dab"},
            {pyDSigned(), 2, "GDIEVMRSOQV3JKZ2CNUL2RQV4TTNAISKW4NAC25PQUQKGMWJO6DTOAE7",
             "17fa5a590c48ae3265bc580570de8f13a83739d8331845085951f507d2c28a15",
             "d6311a2d5b5d7ab60b3a1892c216f1e8d958858bff41ef1c9645f9750780953c"},
            {pyNSigned(), 0, "GCQJVJPUPJTVTABP7FK7RXBNFIKKLSM5EO7JP6DECJ77SOBUKWSPB64N",
             "09929e87f9ead250be9edcecdbcc4ea7c9a8dc863632c7a7b029973be0dac295",
             "c34f95bea47bb2be6121210142326787352c6156cfa711c1a3908f92972d4767"},
            {pyNSigned(), 1, "GCSNRCUM6EDEKSSBQNIOP66ONIM26FVCYP3GHYGD4NR3DK4F635Z32WQ",
             "c42f5da493f9546f48353f52c1414cc4e50d7bc822680671408098e39ebd01c4",
             "8faac618f6d9cc9d6cd85ae7282b310c3eb917c5560b6e6e02cfebef5d53438f"},
        };
        for (const Payload& p : payloads) {
            const QList<stellar::SorobanAuthorizationEntry> entries = entriesOf(p.entries);
            QCOMPARE(entries.size() > p.index, true);
            const stellar::SorobanAuthorizationEntry& entry = entries.at(p.index);
            const quint32 until = entry.credentials.address.signatureExpirationLedger;
            const QByteArray payload = SorobanCredentialsSigner::hashedSignaturePayload(
                entry, until, Network::testnetNetwork()->getNetworkPassphrase());
            QCOMPARE(payload.toHex(), QByteArray(p.testnet));
            QCOMPARE(SorobanCredentialsSigner::hashedSignaturePayload(
                         entry, until, Network::publicNetwork()->getNetworkPassphrase()).toHex(),
                     QByteArray(p.pubnet));

            const stellar::SCVal& shape = entry.credentials.address.signature;
            QVERIFY(shape.type == stellar::SCValType::SCV_VEC && shape.vec && shape.vec->size() == 1);
            const stellar::SCVal& map = shape.vec->first();
            QVERIFY(map.type == stellar::SCValType::SCV_MAP && map.map && map.map->size() == 2);
            QScopedPointer<KeyPair> signer(KeyPair::fromAccountId(QString::fromLatin1(p.signer)));
            QCOMPARE(map.map->at(0).val.bytes, signer->getPublicKey());
            QVERIFY(signer->verify(payload, map.map->at(1).val.bytes));
        }
    }

    /**
     * What py-stellar-sdk 14.1.0's read_challenge_authorization_entries
     * refuses, this SDK refuses, with py's reason where it gives one: a
     * required argument missing, a client domain without its account or the
     * reverse, an empty value (py: "Missing 'nonce'"), an argument that is not
     * a map (py: "Failed to parse"), a home domain differing by case or a
     * trailing dot, a function that is not a contract call, no entries, an
     * entry for a client domain the challenge does not name. What py accepts,
     * another listed home domain, is read, and the server's signature on it,
     * re-signed as py's authorize_entry signs, is py's byte for byte. Verdicts
     * and signature from py 14.1.0 on the same challenges.
     */
    void testV011ReadVerdictsArePys()
    {
        auto withArg = [](const stellar::SCVal& arg) {
            stellar::InvokeContractArgs call = pyCall();
            call.args = {arg};
            return withCall(call);
        };
        auto without = [](int i) {
            QList<QPair<QString, QString>> args = pyArgs();
            args.removeAt(i);
            return argMap(args);
        };
        auto changed = [](int i, const QString& value) {
            QList<QPair<QString, QString>> args = pyArgs();
            args[i].second = value;
            return argMap(args);
        };
        QCOMPARE(readRefusal(withArg(without(0))), QString("Missing 'account' in arguments."));
        QCOMPARE(readRefusal(withArg(without(3))), QString("Missing 'home_domain' in arguments."));
        QCOMPARE(readRefusal(withArg(without(5))), QString("Missing 'web_auth_domain' in arguments."));
        QCOMPARE(readRefusal(withArg(without(6))), QString("Missing 'web_auth_domain_account' in arguments."));
        const QString both = QStringLiteral("'client_domain' and 'client_domain_account' must both "
                                            "be provided or both be absent.");
        QCOMPARE(readRefusal(withArg(without(1))), both);
        QCOMPARE(readRefusal(withArg(without(2))), both);

        QVERIFY(!readRefusal(withArg(changed(4, QString()))).isEmpty());
        QVERIFY(!readRefusal(withArg(changed(3, QString()))).isEmpty());
        QVERIFY(!readRefusal(withArg(Scv::toString(V_HOME()))).isEmpty());
        QVERIFY(!readRefusal(withArg(changed(3, QStringLiteral("EXAMPLE.COM")))).isEmpty());
        QVERIFY(!readRefusal(withArg(changed(3, QStringLiteral("example.com.")))).isEmpty());

        const QString other = withArg(changed(3, QStringLiteral("other.example")));
        QVERIFY(!readRefusal(other).isEmpty());
        QCOMPARE(readRefusal(other, V_SERVER(), {QStringLiteral("other.example"), V_HOME()}), QString());
        const stellar::SorobanAuthorizationEntry server = entriesOf(other).at(1);
        QCOMPARE(addressOf(server), V_SERVER());
        QCOMPARE(server.credentials.address.signature.vec->first().map->at(1).val.bytes.toHex(),
                 QByteArray("720e819d5d2042fb0341879ec674eef64967cb9dfb5c8e8c71dd5969130d0653"
                            "2d5e82d35df880be6a304e9b2024c7ccd646a43d7bd6730a15f249a809946e03"));
        QCOMPARE(readRefusal(pyNBuilt(), V_SERVER(), {QStringLiteral("a.example"), V_HOME()}), QString());

        QList<stellar::SorobanAuthorizationEntry> create = entriesOf(pyDBuilt());
        for (auto& entry : create) {
            entry.rootInvocation.function.type =
                stellar::SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CREATE_CONTRACT_HOST_FN;
            entry.rootInvocation.function.createContractHostFn.contractIDPreimage.fromAddressAddr =
                Scv::toAddress(V_SERVER()).address;
        }
        QCOMPARE(readRefusal(encoded(create)), QString("Authorization entry must invoke a contract function."));
        QCOMPARE(readRefusal(encoded(QList<stellar::SorobanAuthorizationEntry>())),
                 QString("Challenge must contain at least two authorization entries."));

        QList<stellar::SorobanAuthorizationEntry> plain = entriesOf(pyNBuilt());
        stellar::SorobanAuthorizationEntry stray = entriesOf(pyDBuilt()).at(2);
        stray.rootInvocation = plain.first().rootInvocation;
        plain.append(stray);
        QVERIFY(!readRefusal(encoded(plain)).isEmpty());
    }

    /**
     * The server's check: what it simulates is py's verification transaction
     * for the signed challenge, byte for byte, and only a simulation that ran
     * the call and succeeded verifies it. Without a server there is no
     * verification: the client's signature is contract code's to judge.
     */
    void testV011ServerVerifiesAsPyDoes()
    {
        CapturingSep45SorobanServer rpc;
        rpc.sim = simulationWith(QStringList(), V_LATEST);
        const auto verified = Sep45Challenge::verifyChallengeAuthorizationEntries(
            &rpc, pyDSigned(), V_SERVER(), {V_HOME()}, V_WEB(), V_WEB_AUTH(), Network::testnetNetwork());
        QCOMPARE(rpc.requests, QStringList{pyDVerifyTx()});
        QCOMPARE(verified.clientAccountId, V_CLIENT());
        QCOMPARE(verified.nonce, V_NONCE());
        QCOMPARE(verified.clientDomain, V_CLIENT_DOMAIN());

        rpc.requests.clear();
        Sep45Challenge::verifyChallengeAuthorizationEntries(
            &rpc, pyNSigned(), V_SERVER(), {V_HOME()}, V_WEB(), V_WEB_AUTH(), Network::testnetNetwork());
        QCOMPARE(rpc.requests, QStringList{pyNVerifyTx()});
        const auto read = Sep45Challenge::readChallengeAuthorizationEntries(
            pyNSigned(), V_SERVER(), {V_HOME()}, V_WEB(), V_WEB_AUTH(), Network::testnetNetwork());
        QCOMPARE(Sep45Challenge::verificationTransaction(read, Network::testnetNetwork()), pyNVerifyTx());

        auto verifies = [&](SorobanServer* server, const QString& entries) {
            try {
                Sep45Challenge::verifyChallengeAuthorizationEntries(server, entries, V_SERVER(), {V_HOME()},
                                                                    V_WEB(), V_WEB_AUTH(),
                                                                    Network::testnetNetwork());
                return true;
            } catch (const qstellar::exception::InvalidSep45ChallengeException&) {
                return false;
            }
        };
        QJsonObject failed;
        failed.insert("latestLedger", 1);
        failed.insert("error", QStringLiteral("HostError: Error(Auth, InvalidAction)"));
        rpc.sim = SimulateTransactionResponse::fromJson(failed);
        QVERIFY(!verifies(&rpc, pyDSigned()));
        QJsonObject empty;
        empty.insert("latestLedger", 1);
        rpc.sim = SimulateTransactionResponse::fromJson(empty);
        QVERIFY(!verifies(&rpc, pyDSigned()));
        QVERIFY(!verifies(nullptr, pyDSigned()));
        // Anything the wallet would refuse, the server refuses before simulating.
        rpc.sim = simulationWith(QStringList(), V_LATEST);
        rpc.requests.clear();
        stellar::InvokeContractArgs call = pyCall();
        call.functionName = "transfer";
        QVERIFY(!verifies(&rpc, withCall(call)));
        QVERIFY(rpc.requests.isEmpty());
        QVERIFY(verifies(&rpc, pyDSigned()));
    }

    /** The earlier format still works as it did (the 1.0 API is frozen), and
     *  the two formats do not read each other. */
    void testV011AndTheEarlierFormatDoNotMix()
    {
        const QString legacy = validChallenge();
        QCOMPARE(refusal(legacy), QString());
        QVERIFY(!readRefusal(legacy).isEmpty());
        QVERIFY(throwsStd([] {
            Sep45Challenge::verifyChallenge(pyDBuilt(), V_SERVER(), Network::testnetNetwork(), nullptr,
                                            QStringList{V_HOME()}, V_WEB(), V_WEB_AUTH());
        }));
    }

    /**
     * The SEP asks a wallet to simulate what it signed before posting it, and
     * to refuse a call that writes anything but the challenge's own nonces;
     * py does not, and neither did this SDK. The footprint a real RPC returned
     * (py's tests) passes; a token balance, the client contract's own storage,
     * an account, a nonce the entries do not use or somebody else's is a side
     * effect, refused. The web-auth contract's instance, restored when
     * archived, is allowed (v0.1.1).
     */
    void testV011WalletChecksTheSimulationHasNoSideEffect()
    {
        const QString recorded = QStringLiteral(
            "AAAAAAAAAAYAAAAAAAAAAAsFgywxsz4qQ0GCRAuou6p/ImgqqQ7mY7P2yFlmqm0zAAAAAAAAAACk2IqM8QZFSkGDUOf7zmoZ"
            "rxaiw/Zj4MPjY7Grhfb7nQAAAAYAAAABaCSw4cnhsl4aWyRa5pYTz/CKYsGdDy0Idp1Kqpu4xucAAAAUAAAAAQAAAAYAAAAB"
            "pZYrQtB/Sr9w4mNly2nH8h5ic4ytC5rQrLXLbe/N3UMAAAAUAAAAAQAAAAcj000tCjHCGMjC/mKFK3Lht6v48nJut+fTPuaJ"
            "MaiVaQAAAAeaeGC7WJkEo55dPPnhmm7ZJWc9cFqYV+zyj15sRkesdgAAAAMAAAAGAAAAAAAAAAALBYMsMbM+KkNBgkQLqLuq"
            "fyJoKqkO5mOz9shZZqptMwAAABU3nBT/87UYoQAAAAAAAAAGAAAAAAAAAACk2IqM8QZFSkGDUOf7zmoZrxaiw/Zj4MPjY7Gr"
            "hfb7nQAAABVa8Dqi/xldiQAAAAAAAAAGAAAAAWgksOHJ4bJeGlskWuaWE8/wimLBnQ8tCHadSqqbuMbnAAAAFTA2vAnNXt3S"
            "AAAAAAAchrQAAAEgAAAA4AAAAAAAB619");
        const QString domainAccount = QStringLiteral("GAFQLAZMGGZT4KSDIGBEIC5IXOVH6ITIFKUQ5ZTDWP3MQWLGVJWTH3TX");
        auto party = [](const QString& address, qint64 nonce) {
            stellar::SorobanAuthorizationEntry entry;
            entry.credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS;
            entry.credentials.address.address = Scv::toAddress(address).address;
            entry.credentials.address.nonce = nonce;
            return entry;
        };
        Sep45Challenge::ChallengeAuthorizationEntries challenge;
        challenge.clientAccountId = V_CLIENT();
        challenge.serverAccountId = V_SERVER();
        challenge.clientDomain = QStringLiteral("client.example.com");
        challenge.clientDomainAccount = domainAccount;
        challenge.webAuthContract = V_WEB_AUTH();
        challenge.authorizationEntries = {party(V_CLIENT(), 3474170912849190354LL),
                                          party(V_SERVER(), 6552801929563037065LL),
                                          party(domainAccount, 4007100857990846625LL)};
        auto passes = [](const Sep45Challenge::ChallengeAuthorizationEntries& c, const QString& data) {
            try {
                Sep45Challenge::checkSimulationFootprint(c, data);
                return true;
            } catch (const qstellar::exception::InvalidSep45ChallengeException&) {
                return false;
            }
        };
        auto decoded = [](const QString& base64) {
            stellar::SorobanTransactionData data;
            QByteArray raw = QByteArray::fromBase64(base64.toLatin1());
            QDataStream in(&raw, QIODevice::ReadOnly);
            in >> data;
            return data;
        };
        auto encodedData = [](const stellar::SorobanTransactionData& data) {
            QByteArray raw;
            QDataStream out(&raw, QIODevice::WriteOnly);
            out << data;
            return QString::fromLatin1(raw.toBase64());
        };
        auto writing = [&](const stellar::SorobanLedgerKey& key) {
            stellar::SorobanTransactionData data = decoded(recorded);
            data.resources.footprint.readWrite.append(key);
            return encodedData(data);
        };
        QVERIFY(passes(challenge, recorded));
        QCOMPARE(encodedData(decoded(recorded)), recorded);

        Sep45Challenge::ChallengeAuthorizationEntries otherNonce = challenge;
        otherNonce.authorizationEntries[0].credentials.address.nonce += 1;
        QVERIFY(!passes(otherNonce, recorded));
        Sep45Challenge::ChallengeAuthorizationEntries noDomain = challenge;
        noDomain.clientDomain.clear();
        noDomain.clientDomainAccount.clear();
        noDomain.authorizationEntries.removeLast();
        QVERIFY(!passes(noDomain, recorded));

        stellar::SorobanLedgerKey balance;
        balance.type = stellar::LedgerEntryType::CONTRACT_DATA;
        balance.contractData.contract = Scv::toAddress(contractStrKey('\x33')).address;
        balance.contractData.key = Scv::toVec({Scv::toSymbol(QStringLiteral("Balance")), Scv::toAddress(V_CLIENT())});
        balance.contractData.durability = stellar::ContractDataDurability::PERSISTENT;
        QVERIFY(!passes(challenge, writing(balance)));
        stellar::SorobanLedgerKey own = balance;
        own.contractData.contract = Scv::toAddress(V_CLIENT()).address;
        own.contractData.key = Scv::toSymbol(QStringLiteral("spent"));
        QVERIFY(!passes(challenge, writing(own)));
        stellar::SorobanLedgerKey account;
        account.type = stellar::LedgerEntryType::ACCOUNT;
        account.accountID = StrKey::encodeToXDRAccountId(V_SERVER());
        QVERIFY(!passes(challenge, writing(account)));
        stellar::SorobanLedgerKey strangerNonce = balance;
        strangerNonce.contractData.contract = Scv::toAddress(contractStrKey('\x44')).address;
        strangerNonce.contractData.key = Scv::toLedgerKeyNonce(qint64(1));
        strangerNonce.contractData.durability = stellar::ContractDataDurability::TEMPORARY;
        QVERIFY(!passes(challenge, writing(strangerNonce)));

        stellar::SorobanLedgerKey instance = balance;
        instance.contractData.contract = Scv::toAddress(V_WEB_AUTH()).address;
        instance.contractData.key = Scv::toLedgerKeyContractInstance();
        QVERIFY(passes(challenge, writing(instance)));
        instance.contractData.contract = Scv::toAddress(contractStrKey('\x33')).address;
        QVERIFY(!passes(challenge, writing(instance)));

        QVERIFY(!passes(challenge, QString()));
        QVERIFY(!passes(challenge, recorded + QStringLiteral("AAAA")));
    }

    /**
     * A real login against testanchor.stellar.org (testnet, 2026-09-30) while
     * its web-auth contract was archived: the simulation restores the
     * instance AND its wasm, both listed in archivedSorobanEntries [2, 3].
     * The wasm (CONTRACT_CODE) was refused as "not contract data", so every
     * login failed the wallet's check while the server issued a token. An
     * immutable wasm is restored, not written; unlisted, it stays refused.
     */
    void testV011WalletAcceptsTheRestoredArchivedWasm()
    {
        const QString real = QStringLiteral(
            "AAAAAQAAAAIAAAACAAAAAwAAAAUAAAAAAAAAAI6zjC5RtJsxMAzXJfbm813ySujUwQVm4r2uHtkav62tAAAABgAAAAFTqYAV"
            "Fna/IQ6pOZSzsbQxJMXafNWX6FpDA2PuuT91OgAAABAAAAABAAAAAgAAAA8AAAAHRWQyNTUxOQAAAAANAAAAIMHnGB6oSzbM"
            "s4P/DQfp6hCgmPzx3jn2f3s90P1lqiB/AAAAAAAAAAYAAAABU6mAFRZ2vyEOqTmUs7G0MSTF2nzVl+haQwNj7rk/dToAAAAQ"
            "AAAAAQAAAAIAAAAPAAAAB0VkMjU1MTkAAAAADQAAACDB5xgeqEs2zLOD/w0H6eoQoJj88d459n97PdD9ZaogfwAAAAEAAAAG"
            "AAAAAVOpgBUWdr8hDqk5lLOxtDEkxdp81ZfoWkMDY+65P3U6AAAAFAAAAAEAAAAH/e+tZLloNxR+HDM+UfU3tpbquSXp8Ufm"
            "PVl8BOPJA/AAAAAEAAAABgAAAAAAAAAAjrOMLlG0mzEwDNcl9ubzXfJK6NTBBWbiva4e2Rq/ra0AAAAVZZlmliqw0SUAAAAA"
            "AAAABgAAAAFTqYAVFna/IQ6pOZSzsbQxJMXafNWX6FpDA2PuuT91OgAAABUIm57n42+/8gAAAAAAAAAGAAAAAfaweiovR6K3"
            "UdL79lrrDGRLWI0n+YCQuZSM1MUmQIpoAAAAFAAAAAEAAAAH7doZWfgqJopRjXonTaZJncde+t1AM94wi4ClpMiqAyMAKt/M"
            "AAAIWAAACFwAAAAAAKJVFw==");
        auto party = [](const QString& address, qint64 nonce) {
            stellar::SorobanAuthorizationEntry entry;
            entry.credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS;
            entry.credentials.address.address = Scv::toAddress(address).address;
            entry.credentials.address.nonce = nonce;
            return entry;
        };
        Sep45Challenge::ChallengeAuthorizationEntries challenge;
        challenge.clientAccountId = QStringLiteral("CBJ2TAAVCZ3L6IIOVE4ZJM5RWQYSJRO2PTKZP2C2IMBWH3VZH52TUEKG");
        challenge.serverAccountId = QStringLiteral("GCHLHDBOKG2JWMJQBTLSL5XG6NO7ESXI2TAQKZXCXWXB5WI2X6W233PR");
        challenge.webAuthContract = QStringLiteral("CD3LA6RKF5D2FN2R2L57MWXLBRSEWWENE74YBEFZSSGNJRJGICFGQXMX");
        challenge.authorizationEntries = {party(challenge.serverAccountId, 7320995464414810405LL),
                                          party(challenge.clientAccountId, 620264092483960818LL)};
        auto passes = [](const Sep45Challenge::ChallengeAuthorizationEntries& c, const QString& data) {
            try {
                Sep45Challenge::checkSimulationFootprint(c, data);
                return true;
            } catch (const qstellar::exception::InvalidSep45ChallengeException&) {
                return false;
            }
        };
        QVERIFY(passes(challenge, real));

        stellar::SorobanTransactionData data;
        QByteArray raw = QByteArray::fromBase64(real.toLatin1());
        {
            QDataStream in(&raw, QIODevice::ReadOnly);
            in >> data;
            QCOMPARE(in.status(), QDataStream::Ok);
        }
        QCOMPARE(data.ext.v, 1);
        QCOMPARE(data.ext.archivedSorobanEntries, QVector<quint32>({2, 3}));
        QCOMPARE(data.resources.footprint.readWrite.at(3).type, stellar::LedgerEntryType::CONTRACT_CODE);
        auto encoded = [](const stellar::SorobanTransactionData& d) {
            QByteArray bytes;
            QDataStream out(&bytes, QIODevice::WriteOnly);
            out << d;
            return QString::fromLatin1(bytes.toBase64());
        };
        QCOMPARE(encoded(data), real);
        // The same wasm, not listed as archived: a write, refused.
        stellar::SorobanTransactionData unlisted = data;
        unlisted.ext.archivedSorobanEntries = {2};
        QVERIFY(!passes(challenge, encoded(unlisted)));
        unlisted.ext.v = 0;
        unlisted.ext.archivedSorobanEntries.clear();
        QVERIFY(!passes(challenge, encoded(unlisted)));
        // Listing an entry does not let anything else through: an account.
        stellar::SorobanTransactionData account = data;
        account.resources.footprint.readWrite[3].type = stellar::LedgerEntryType::ACCOUNT;
        account.resources.footprint.readWrite[3].accountID =
            StrKey::encodeToXDRAccountId(challenge.serverAccountId);
        QVERIFY(!passes(challenge, encoded(account)));
        // Nor a contract's storage that is listed as archived.
        stellar::SorobanTransactionData storage = data;
        stellar::SorobanLedgerKey balance;
        balance.type = stellar::LedgerEntryType::CONTRACT_DATA;
        balance.contractData.contract = Scv::toAddress(challenge.clientAccountId).address;
        balance.contractData.key = Scv::toSymbol(QStringLiteral("spent"));
        balance.contractData.durability = stellar::ContractDataDurability::PERSISTENT;
        storage.resources.footprint.readWrite[3] = balance;
        QVERIFY(!passes(challenge, encoded(storage)));
    }

    /** The wallet's check end to end: what it simulates is the verification
     *  transaction py builds for what it signed, and a simulation that writes
     *  more than the nonces, fails or ran nothing makes it stop. */
    void testV011WalletSimulatesWhatItSigned()
    {
        const QString nonces = QStringLiteral(
            "AAAAAAAAAAAAAAACAAAABgAAAAFoJLDhyeGyXhpbJFrmlhPP8IpiwZ0PLQh2nUqqm7jG5wAAABXu3e8LghZ+6wAAAAAAAAAG"
            "AAAAAAAAAACk2IqM8QZFSkGDUOf7zmoZrxaiw/Zj4MPjY7Grhfb7nQAAABUAAAAAOt5osQAAAAAAD0JAAAAD6AAAAGQAAAAA"
            "AADDUA==");
        const QString withBalance = QStringLiteral(
            "AAAAAAAAAAAAAAADAAAABgAAAAFoJLDhyeGyXhpbJFrmlhPP8IpiwZ0PLQh2nUqqm7jG5wAAABXu3e8LghZ+6wAAAAAAAAAG"
            "AAAAAAAAAACk2IqM8QZFSkGDUOf7zmoZrxaiw/Zj4MPjY7Grhfb7nQAAABUAAAAAOt5osQAAAAAAAAAGAAAAASW0/NhZrsL6"
            "Y0hDjEibPDwQyYttIb5P08swy2iVPvl3AAAAEAAAAAEAAAACAAAADwAAAAdCYWxhbmNlAAAAABIAAAABaCSw4cnhsl4aWyRa"
            "5pYTz/CKYsGdDy0Idp1Kqpu4xucAAAABAA9CQAAAA+gAAABkAAAAAAAAw1A=");
        auto answer = [](const QString& transactionData, const QString& error = QString()) {
            QJsonObject result;
            result.insert("auth", QJsonArray());
            result.insert("xdr", QStringLiteral("AAAAAQ=="));
            QJsonObject sim;
            sim.insert("latestLedger", 82110);
            sim.insert("transactionData", transactionData);
            if (error.isEmpty())
                sim.insert("results", QJsonArray{result});
            else
                sim.insert("error", error);
            return SimulateTransactionResponse::fromJson(sim);
        };
        auto simulates = [](SorobanServer* server) {
            try {
                Sep45Challenge::simulateSignedChallenge(server, pyNSigned(), V_SERVER(), {V_HOME()}, V_WEB(),
                                                        V_WEB_AUTH(), Network::testnetNetwork());
                return true;
            } catch (const qstellar::exception::InvalidSep45ChallengeException&) {
                return false;
            }
        };
        CapturingSep45SorobanServer rpc;
        rpc.sim = answer(nonces);
        QVERIFY(simulates(&rpc));
        QCOMPARE(rpc.requests, QStringList{pyNVerifyTx()});
        rpc.sim = answer(withBalance);
        QVERIFY(!simulates(&rpc));
        rpc.sim = answer(nonces, QStringLiteral("HostError: Error(Auth, InvalidAction)"));
        QVERIFY(!simulates(&rpc));
        QJsonObject empty;
        empty.insert("latestLedger", 1);
        empty.insert("transactionData", nonces);
        rpc.sim = SimulateTransactionResponse::fromJson(empty);
        QVERIFY(!simulates(&rpc));
        QVERIFY(!simulates(nullptr));
    }
};

ADD_TEST(Sep45ChallengeTest)
#endif // SEP45CHALLENGETEST_H
