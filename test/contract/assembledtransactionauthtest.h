#ifndef ASSEMBLEDTRANSACTIONAUTHTEST_H
#define ASSEMBLEDTRANSACTIONAUTHTEST_H

#include <QJsonArray>
#include <QJsonDocument>
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
#include "../../src/exception/sendfailedexception.h"
#include "../../src/exception/simulationfailedexception.h"
#include "../../src/exception/sorobanrpcexception.h"
#include <QElapsedTimer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

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
    /** How the send fails, if it does: as SorobanServer's own do. */
    enum SendFailure { NoFailure, NeverConnected, TimedOut, Cancelled, LostMidway,
                       RpcError, Unparsable };
    SendFailure sendFailure = NoFailure;
    /** Takes the account's next number during the send, as a build made in
     *  its nested event loop would. */
    Account* buildsMeanwhile = nullptr;

    SimulateTransactionResponse simulateTransactionXdr(const QString&) override {
        return simResponses.at(qMin(simIdx++, int(simResponses.size() - 1)));
    }
    SendTransactionResponse sendTransactionXdr(const QString& env) override {
        capturedSendEnvelopes.append(env);
        if (buildsMeanwhile) {
            buildsMeanwhile->incrementSequenceNumber();
            buildsMeanwhile = nullptr;
        }
        using qstellar::exception::SorobanTransportException;
        switch (sendFailure) {
        case NoFailure:
            break;
        case NeverConnected:
            throw SorobanTransportException(QStringLiteral(
                "Soroban RPC HTTP error: Connection refused (sendTransaction); the request was not sent"), false);
        case TimedOut:
            throw SorobanTransportException(QStringLiteral(
                "Soroban RPC HTTP error: no answer within 30000 ms (sendTransaction); the server "
                "may still have received it"), true);
        case Cancelled:
            throw SorobanTransportException(QStringLiteral(
                "Soroban RPC request cancelled before its answer arrived (sendTransaction); the "
                "server may still have received it"), true);
        case LostMidway:
            throw SorobanTransportException(QStringLiteral(
                "Soroban RPC HTTP error: Connection closed (sendTransaction); the server may have "
                "received it"), true);
        case RpcError:
            throw qstellar::exception::SorobanRpcException(
                -32603, QStringLiteral("could not submit transaction to stellar-core"));
        case Unparsable: {
            QJsonObject garbled;
            garbled.insert("status", QStringLiteral("PENDNG"));
            return SendTransactionResponse::fromJson(garbled);     // throws
        }
        }
        return sendResponses.at(qMin(sendIdx++, int(sendResponses.size() - 1)));
    }
protected:
    GetTransactionResponse fetchTransaction(const QString&) override {
        return fetchResponses.at(qMin(fetchIdx++, int(fetchResponses.size() - 1)));
    }
};

/** A SorobanServer that simulates from a script and sends for real, over
 *  HTTP to `endpoint`. */
class HttpSendingSorobanServer : public SorobanServer
{
public:
    explicit HttpSendingSorobanServer(const QUrl& endpoint) : SorobanServer(endpoint) {}
    SimulateTransactionResponse sim;
    SimulateTransactionResponse simulateTransactionXdr(const QString&) override { return sim; }
};

/** A SorobanServer that simulates from a script, answers the send with
 *  `sent` (recording the envelope), and polls for real, over HTTP to
 *  `endpoint`. */
class PollingOverHttpSorobanServer : public SorobanServer
{
public:
    explicit PollingOverHttpSorobanServer(const QUrl& endpoint) : SorobanServer(endpoint) {}
    SimulateTransactionResponse sim;
    SendTransactionResponse sent;
    QStringList envelopes;
    SimulateTransactionResponse simulateTransactionXdr(const QString&) override { return sim; }
    SendTransactionResponse sendTransactionXdr(const QString& envelope) override
    {
        envelopes.append(envelope);
        return sent;
    }
};

/**
 * Answers the first request of a connection and keeps the connection alive,
 * reads the second in full, then stops listening and drops the connection
 * without an answer: the whole second request reached it, and a connection
 * made to send it again is refused.
 *
 * It records which connection carried each request, because the second one
 * does not always travel over the kept-alive connection: now and then Qt
 * closes that connection itself as the first answer arrives. When the
 * answer is already there as Qt finishes writing the request, Qt queues a
 * read of it; if the socket's own readyRead reads it first, the queued read
 * finds no reply left and closes the connection, and the second request
 * goes out over a new one. Seen about once in 35 runs under load.
 */
class KeptAliveThenDroppedEndpoint
{
public:
    KeptAliveThenDroppedEndpoint()
    {
        QObject::connect(&m_server, &QTcpServer::newConnection, [this]() {
            while (m_server.hasPendingConnections()) {
                QTcpSocket* socket = m_server.nextPendingConnection();
                const int connection = ++m_connections;
                QObject::connect(socket, &QTcpSocket::readyRead, socket,
                                 [this, socket, connection]() { read(socket, connection); });
            }
        });
        m_server.listen(QHostAddress::LocalHost, 0);
        m_port = m_server.serverPort();         // 0 once it stops listening
    }
    QUrl url() const { return QUrl(QStringLiteral("http://127.0.0.1:%1/").arg(m_port)); }
    int connections() const { return m_connections; }
    int requests() const { return m_requests; }
    QByteArray lastRequest() const { return m_lastRequest; }
    /** The connection (1 for the first one taken) each whole request came over, in order. */
    QList<int> carriers() const { return m_carriers; }

private:
    void read(QTcpSocket* socket, int connection)
    {
        QByteArray& buffer = m_buffers[socket];
        buffer += socket->readAll();
        for (;;) {
            const int end = int(buffer.indexOf("\r\n\r\n"));
            if (end < 0)
                return;
            int length = 0;
            for (const QByteArray& line : buffer.left(end).split('\n')) {
                if (line.toLower().startsWith("content-length:"))
                    length = line.mid(15).trimmed().toInt();
            }
            if (buffer.size() < end + 4 + length)
                return;
            m_lastRequest = buffer.left(end + 4 + length);
            buffer.remove(0, end + 4 + length);
            m_carriers.append(connection);
            if (++m_requests == 1) {
                const QByteArray body =
                    "{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"result\":{\"status\":\"healthy\","
                    "\"latestLedger\":4917043,\"oldestLedger\":4796084,\"ledgerRetentionWindow\":120960}}";
                socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                              "Connection: keep-alive\r\nContent-Length: "
                              + QByteArray::number(body.size()) + "\r\n\r\n" + body);
            } else {
                m_server.close();       // a new connection is refused from now on
                socket->abort();        // and this one ends without an answer
                return;
            }
        }
    }

    QTcpServer m_server;
    quint16 m_port = 0;
    QHash<QTcpSocket*, QByteArray> m_buffers;
    int m_connections = 0;
    int m_requests = 0;
    QByteArray m_lastRequest;
    QList<int> m_carriers;
};

/**
 * A Soroban RPC that reads every request whole. It drops the connection of
 * the first sendTransaction without an answer (read whole, never answered)
 * and answers every later one with `status`, under the hash of the envelope
 * posted on testnet (with txBAD_SEQ for ERROR); getTransaction it answers
 * SUCCESS for the hash asked. Qt 5 sends a request again, by itself, on a new
 * connection, when the one it was written on closes before an answer: the
 * sendTransaction dropped is read again, and that later send is answered. It
 * keeps the envelope of every sendTransaction read whole, in order.
 */
class FirstSendDroppedEndpoint
{
public:
    explicit FirstSendDroppedEndpoint(const QString& status) : m_status(status)
    {
        QObject::connect(&m_server, &QTcpServer::newConnection, [this]() {
            while (m_server.hasPendingConnections()) {
                QTcpSocket* socket = m_server.nextPendingConnection();
                QObject::connect(socket, &QTcpSocket::readyRead, socket, [this, socket]() { read(socket); });
            }
        });
        m_server.listen(QHostAddress::LocalHost, 0);
    }
    QUrl url() const { return QUrl(QStringLiteral("http://127.0.0.1:%1/").arg(m_server.serverPort())); }
    /** The envelope of every sendTransaction read whole, in order. */
    QStringList envelopes() const { return m_envelopes; }

private:
    void read(QTcpSocket* socket)
    {
        QByteArray& buffer = m_buffers[socket];
        buffer += socket->readAll();
        for (;;) {
            const int end = int(buffer.indexOf("\r\n\r\n"));
            if (end < 0)
                return;
            int length = 0;
            for (const QByteArray& line : buffer.left(end).split('\n')) {
                if (line.toLower().startsWith("content-length:"))
                    length = line.mid(15).trimmed().toInt();
            }
            if (buffer.size() < end + 4 + length)
                return;
            const QJsonObject call = QJsonDocument::fromJson(buffer.mid(end + 4, length)).object();
            buffer.remove(0, end + 4 + length);
            const QJsonObject params = call.value(QStringLiteral("params")).toObject();
            QJsonObject result;
            if (call.value(QStringLiteral("method")).toString() == QLatin1String("sendTransaction")) {
                const QString envelope = params.value(QStringLiteral("transaction")).toString();
                m_envelopes.append(envelope);
                if (m_envelopes.size() == 1) {
                    buffer.clear();
                    socket->abort();    // read whole, never answered
                    return;
                }
                QString hash = QString(64, QLatin1Char('0'));
                try {
                    QScopedPointer<AbstractTransaction> tx(
                        AbstractTransaction::fromEnvelopeXdr(envelope, Network::testnetNetwork()));
                    hash = tx->hashHex();
                } catch (const std::exception&) {
                }
                result.insert("status", m_status);
                result.insert("hash", hash);
                result.insert("latestLedger", 4919962);
                result.insert("latestLedgerCloseTime", QStringLiteral("1790623397"));
                if (m_status == QLatin1String("ERROR"))
                    result.insert("errorResultXdr", QStringLiteral("AAAAAAAAAGT////7AAAAAA=="));
            } else {
                result.insert("status", QStringLiteral("SUCCESS"));
                result.insert("txHash", params.value(QStringLiteral("hash")).toString());
                result.insert("latestLedger", 4919964);
                result.insert("latestLedgerCloseTime", QStringLiteral("1790623407"));
                result.insert("oldestLedger", 4799005);
                result.insert("oldestLedgerCloseTime", QStringLiteral("1790018612"));
                result.insert("applicationOrder", 2);
                result.insert("feeBump", false);
                result.insert("ledger", 4919963);
                result.insert("createdAt", QStringLiteral("1790623402"));
                result.insert("envelopeXdr", QStringLiteral("AAAAAg=="));
                result.insert("resultXdr", QStringLiteral("AAAAAA=="));
                result.insert("resultMetaXdr", QStringLiteral("AAAABA=="));
            }
            QJsonObject answer;
            answer.insert("jsonrpc", QStringLiteral("2.0"));
            answer.insert("id", QStringLiteral("1"));
            answer.insert("result", result);
            const QByteArray body = QJsonDocument(answer).toJson(QJsonDocument::Compact);
            socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                          "Connection: keep-alive\r\nContent-Length: "
                          + QByteArray::number(body.size()) + "\r\n\r\n" + body);
        }
    }

    QTcpServer m_server;
    QHash<QTcpSocket*, QByteArray> m_buffers;
    QString m_status;
    QStringList m_envelopes;
};

/** Takes every connection and never answers; hangs up after 10 s so a test
 *  without a timeout fails instead of hanging. */
class NeverAnsweringEndpoint
{
public:
    NeverAnsweringEndpoint()
    {
        QObject::connect(&m_server, &QTcpServer::newConnection, [this]() {
            while (m_server.hasPendingConnections())
                m_server.nextPendingConnection();       // a child of m_server; never read
        });
        m_server.listen(QHostAddress::LocalHost, 0);
        m_hangUp.setSingleShot(true);
        QObject::connect(&m_hangUp, &QTimer::timeout, [this]() {
            for (QTcpSocket* socket : m_server.findChildren<QTcpSocket*>())
                socket->abort();
        });
        m_hangUp.start(10000);
    }
    QUrl url() const { return QUrl(QStringLiteral("http://127.0.0.1:%1/").arg(m_server.serverPort())); }

private:
    QTcpServer m_server;
    QTimer m_hangUp;
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

    /** One entry the simulated call writes: without it, a call that asks for
     *  no authorization is a read call, which sign() refuses. */
    static stellar::SorobanLedgerKey writtenKey()
    {
        stellar::SorobanLedgerKey k;
        k.type = stellar::LedgerEntryType::CONTRACT_CODE;
        for (int i = 0; i < 32; ++i) k.contractCode.hash[i] = quint8(0x11);
        return k;
    }

    static SimulateTransactionResponse makeSimResponse(qint64 resourceFee, quint32 latestLedger = 1000)
    {
        stellar::SorobanTransactionData td = SorobanDataBuilder()
            .setReadWrite({writtenKey()}).setResources(123456u, 4096u, 2048u).setResourceFee(resourceFee).build();
        QJsonObject simJson;
        simJson.insert("latestLedger", int(latestLedger));
        simJson.insert("transactionData", xdrToBase64(td));
        simJson.insert("minResourceFee", QString::number(resourceFee));
        QJsonObject result0; result0.insert("xdr", xdrToBase64(Scv::toVoid()));
        QJsonArray results; results.append(result0);
        simJson.insert("results", results);
        return SimulateTransactionResponse::fromJson(simJson);
    }

    /** A simulation that asks for a restore, with the preamble's own footprint. */
    static SimulateTransactionResponse makeRestoreSimResponse()
    {
        QJsonObject simJson;
        simJson.insert("latestLedger", 1000);
        QJsonObject result0; result0.insert("xdr", xdrToBase64(Scv::toVoid()));
        simJson.insert("results", QJsonArray{result0});
        QJsonObject rp;
        rp.insert("transactionData", xdrToBase64(
            SorobanDataBuilder().setResources(100u, 100u, 100u).setResourceFee(qint64(555)).build()));
        rp.insert("minResourceFee", QStringLiteral("555"));
        simJson.insert("restorePreamble", rp);
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

    /** The sequence number of the envelope the stub was sent. */
    static qint64 sentSequence(const QString& envelope)
    {
        QScopedPointer<AbstractTransaction> tx(
            AbstractTransaction::fromEnvelopeXdr(envelope, Network::testnetNetwork()));
        return dynamic_cast<Transaction*>(tx.data())->getSequenceNumber();
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

    /**
     * simulate() and signAuthEntries() change the signed body (the soroban
     * data and the fee, the auth entries): after sign() they are refused, as
     * py's prepare() and sign_auth_entries() refuse them ("Prepare must happen
     * before transaction signing", "Authorization entries must be signed
     * before transaction signing"), and
     * the transaction is left as signed. They used to go ahead and keep the
     * stale signature, and signAndSend added a second one (txBAD_AUTH_EXTRA).
     */
    void testSimulateAndSignAuthEntriesAreRefusedOnceSigned()
    {
        StubAuthSorobanServer srv;
        srv.simResponses = { makeSimResponse(10) };
        QList<stellar::SorobanAuthorizationEntry> noAuth;
        AssembledTransaction at(buildTxWithAuth(noAuth), &srv, Network::testnetNetwork());
        at.simulate();
        QScopedPointer<KeyPair> signer(sourceKeyPair());
        at.sign(signer.data());
        const QString signedEnvelope = at.getTransaction()->toEnvelopeXdrBase64();

        QVERIFY_EXCEPTION_THROWN(at.simulate(), qstellar::exception::AssembledTransactionException);
        QCOMPARE(srv.simIdx, 1);                    // not simulated again
        QVERIFY_EXCEPTION_THROWN(at.signAuthEntries(signer.data()),
                                 qstellar::exception::AssembledTransactionException);
        QCOMPARE(at.getTransaction()->toEnvelopeXdrBase64(), signedEnvelope);
        QCOMPARE(at.getTransaction()->getSignatures().size(), 1);
    }

    /**
     * A simulation that asks for a restore (restorePreamble) is not signed or
     * sent: the call needs archived entries, and on chain it fails with the
     * fee paid. js and py refuse it (ExpiredState). It used to be signed and
     * sent. Simulated again once the entries are restored, it goes.
     */
    void testSignIsRefusedWhileTheSimulationAsksForARestore()
    {
        StubAuthSorobanServer srv;
        srv.simResponses = { makeRestoreSimResponse(), makeSimResponse(10) };
        srv.sendResponses = { sendStatus(QStringLiteral("PENDING")) };
        srv.fetchResponses = { txStatus(QStringLiteral("SUCCESS")) };
        QList<stellar::SorobanAuthorizationEntry> noAuth;
        AssembledTransaction at(buildTxWithAuth(noAuth), &srv, Network::testnetNetwork());
        at.simulate();
        QScopedPointer<KeyPair> signer(sourceKeyPair());
        QVERIFY_EXCEPTION_THROWN(at.sign(signer.data()), qstellar::exception::ExpiredStateException);
        QVERIFY_EXCEPTION_THROWN(at.sign(signer.data(), true),             // force does not skip it
                                 qstellar::exception::ExpiredStateException);
        QVERIFY_EXCEPTION_THROWN(at.signAndSend(signer.data(), 5000, 1),
                                 qstellar::exception::ExpiredStateException);
        QVERIFY(at.getTransaction()->getSignatures().isEmpty());
        QVERIFY(srv.capturedSendEnvelopes.isEmpty());

        at.simulate();                              // the entries restored meanwhile
        QCOMPARE(at.signAndSend(signer.data(), 5000, 1).getStatus(),
                 GetTransactionResponse::Status::SUCCESS);
        QCOMPARE(srv.capturedSendEnvelopes.size(), 1);
    }

    /**
     * After a simulation that failed, sign() and signAndSend() refuse
     * (SimulationFailedException), `force` too, as js does (SimulationFailed,
     * forced or not; py refuses unless forced): the transaction still carries
     * the earlier, successful simulation's soroban data, fee and auth. It
     * used to be signed and sent. Simulated again with success, it goes.
     */
    void testSignIsRefusedAfterASimulationThatFailed()
    {
        QJsonObject failed;
        failed.insert("latestLedger", 1001);
        failed.insert("error", QStringLiteral("HostError: Error(Contract, #1)"));
        StubAuthSorobanServer srv;
        srv.simResponses = { makeSimResponse(10), SimulateTransactionResponse::fromJson(failed),
                             makeSimResponse(10) };
        srv.sendResponses = { sendStatus(QStringLiteral("PENDING")) };
        srv.fetchResponses = { txStatus(QStringLiteral("SUCCESS")) };
        QList<stellar::SorobanAuthorizationEntry> noAuth;
        AssembledTransaction at(buildTxWithAuth(noAuth), &srv, Network::testnetNetwork());
        at.simulate();
        QVERIFY_EXCEPTION_THROWN(at.simulate(), qstellar::exception::SimulationFailedException);
        QVERIFY(!at.isSimulated());

        QScopedPointer<KeyPair> signer(sourceKeyPair());
        QVERIFY_EXCEPTION_THROWN(at.sign(signer.data()),
                                 qstellar::exception::SimulationFailedException);
        QVERIFY_EXCEPTION_THROWN(at.sign(signer.data(), true),             // force does not skip it
                                 qstellar::exception::SimulationFailedException);
        QVERIFY_EXCEPTION_THROWN(at.signAndSend(signer.data(), 5000, 1, true),
                                 qstellar::exception::SimulationFailedException);
        QVERIFY(at.getTransaction()->getSignatures().isEmpty());
        QVERIFY(srv.capturedSendEnvelopes.isEmpty());

        at.simulate();
        QVERIFY(at.isSimulated());
        QCOMPARE(at.signAndSend(signer.data(), 5000, 1).getStatus(),
                 GetTransactionResponse::Status::SUCCESS);
        QCOMPARE(srv.capturedSendEnvelopes.size(), 1);
    }

    /**
     * While an auth entry of a G... address is unsigned, sign() and
     * signAndSend() refuse with NeedsMoreSignatures, as js and py do, with
     * `force` too: sent, the host refuses the authorization when the
     * transaction is applied, and the fee is paid. They used to sign and send
     * it. Once every entry is signed, it goes.
     */
    void testSignIsRefusedWhileAuthEntriesAreUnsigned()
    {
        QScopedPointer<KeyPair> invoker(sourceKeyPair());
        QScopedPointer<KeyPair> other(KeyPair::random());
        QList<stellar::SorobanAuthorizationEntry> auth;
        auth.append(unsignedAddressEntry(other->getPublicKey()));
        StubAuthSorobanServer srv;
        srv.simResponses = { makeSimResponse(10) };
        srv.sendResponses = { sendStatus(QStringLiteral("PENDING")) };
        srv.fetchResponses = { txStatus(QStringLiteral("SUCCESS")) };
        AssembledTransaction at(buildTxWithAuth(auth), &srv, Network::testnetNetwork());
        at.simulate();

        QStringList addresses;
        try {
            at.signAndSend(invoker.data(), 5000, 1);
        } catch (const qstellar::exception::NeedsMoreSignaturesException& e) {
            addresses = e.getAddresses();
        }
        QCOMPARE(addresses, QStringList{other->getAccountId()});
        QVERIFY_EXCEPTION_THROWN(at.sign(invoker.data(), true),
                                 qstellar::exception::NeedsMoreSignaturesException);
        QVERIFY_EXCEPTION_THROWN(at.signAndSend(invoker.data(), 5000, 1, true),
                                 qstellar::exception::NeedsMoreSignaturesException);
        QVERIFY(at.getTransaction()->getSignatures().isEmpty());
        QVERIFY(srv.capturedSendEnvelopes.isEmpty());

        at.signAuthEntries(other.data());
        QCOMPARE(at.signAndSend(invoker.data(), 5000, 1).getStatus(),
                 GetTransactionResponse::Status::SUCCESS);
        QCOMPARE(srv.capturedSendEnvelopes.size(), 1);
    }

    /**
     * An ADDRESS_WITH_DELEGATES entry of a G... address whose top-level
     * signature is SCV_VOID is listed by needsNonInvokerSigningBy(), and
     * sign() refuses it (NeedsMoreSignatures), `force` too, as js 17.1.0 does
     * (its getAddressCredentials reads the top-level credentials). It was
     * skipped, and the call was signed and sent for the host to refuse. As in
     * js, only the top-level signature counts: with it present, an unsigned
     * delegate leaves the entry off the list, and sign() goes ahead.
     */
    void testAnEntryWithDelegatesCountsItsTopLevelSignature()
    {
        stellar::SorobanDelegateSignature delegate;          // signature SCV_VOID
        delegate.address.type = stellar::SCAddressType::SC_ADDRESS_TYPE_ACCOUNT;
        std::memset(delegate.address.accountId, 0x43, 32);
        stellar::SorobanAuthorizationEntry entry;
        entry.credentials.type =
            stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_WITH_DELEGATES;
        stellar::SorobanAddressCredentials& top = entry.credentials.addressWithDelegates.addressCredentials;
        top.address.type = stellar::SCAddressType::SC_ADDRESS_TYPE_ACCOUNT;
        std::memset(top.address.accountId, 0x42, 32);
        top.nonce = qint64(0xAABB);
        entry.credentials.addressWithDelegates.delegates.append(delegate);
        entry.rootInvocation.function.type =
            stellar::SorobanAuthorizedFunctionType::SOROBAN_AUTHORIZED_FUNCTION_TYPE_CONTRACT_FN;
        // js 17.1.0, for such an entry: needsNonInvokerSigningBy() lists this
        // address, and sign() and sign({force: true}) throw NeedsMoreSignatures.
        const QString other = QStringLiteral("GBBEEQSCIJBEEQSCIJBEEQSCIJBEEQSCIJBEEQSCIJBEEQSCIJBEFZSP");
        QScopedPointer<KeyPair> invoker(sourceKeyPair());
        {
            StubAuthSorobanServer srv;
            srv.simResponses = { makeSimResponse(10) };
            AssembledTransaction at(buildTxWithAuth(QList<stellar::SorobanAuthorizationEntry>{entry}),
                                    &srv, Network::testnetNetwork());
            at.simulate();
            QCOMPARE(at.needsNonInvokerSigningBy(), QStringList{other});
            QVERIFY_EXCEPTION_THROWN(at.sign(invoker.data()),
                                     qstellar::exception::NeedsMoreSignaturesException);
            QVERIFY_EXCEPTION_THROWN(at.sign(invoker.data(), true),
                                     qstellar::exception::NeedsMoreSignaturesException);
            QVERIFY(at.getTransaction()->getSignatures().isEmpty());
        }
        // The top level signed, the delegate not: js lists nothing and signs.
        top.signature = Scv::toVec(QList<stellar::SCVal>{Scv::toUint32(quint32(1))});
        {
            StubAuthSorobanServer srv;
            srv.simResponses = { makeSimResponse(10) };
            AssembledTransaction at(buildTxWithAuth(QList<stellar::SorobanAuthorizationEntry>{entry}),
                                    &srv, Network::testnetNetwork());
            at.simulate();
            QVERIFY(at.needsNonInvokerSigningBy().isEmpty());
            at.sign(invoker.data());
            QCOMPARE(at.getTransaction()->getSignatures().size(), 1);
        }
    }

    /**
     * signAuthEntries() with a key that has no unsigned entry (another
     * account's, or one already signed) changes nothing and does not throw.
     * py's sign_auth_entries does not throw either, but signs an entry
     * already signed again; js throws (NoSignatureNeeded,
     * NoUnsignedNonInvokerAuthEntries). sign() is what refuses while an entry
     * is left unsigned.
     */
    void testSignAuthEntriesWithNothingToSignChangesNothing()
    {
        QScopedPointer<KeyPair> other(KeyPair::random());
        QScopedPointer<KeyPair> stranger(KeyPair::random());
        QList<stellar::SorobanAuthorizationEntry> auth;
        auth.append(unsignedAddressEntry(other->getPublicKey()));
        StubAuthSorobanServer srv;
        srv.simResponses = { makeSimResponse(10) };
        AssembledTransaction at(buildTxWithAuth(auth), &srv, Network::testnetNetwork());
        at.simulate();
        const QString before = at.getTransaction()->toEnvelopeXdrBase64();
        at.signAuthEntries(stranger.data(), 5000);
        QCOMPARE(at.getTransaction()->toEnvelopeXdrBase64(), before);
        QCOMPARE(at.needsNonInvokerSigningBy(), QStringList{other->getAccountId()});

        at.signAuthEntries(other.data(), 5000);
        const QString signedOnce = at.getTransaction()->toEnvelopeXdrBase64();
        QVERIFY(signedOnce != before);
        at.signAuthEntries(other.data(), 6000);         // already signed: left as it was
        QCOMPARE(at.getTransaction()->toEnvelopeXdrBase64(), signedOnce);
        QVERIFY(at.needsNonInvokerSigningBy().isEmpty());
    }

    // ── restoreFootprint ─────────────────────────────────────────────

    void testRestoreFootprintBuildsSendsAndPolls()
    {
        // Simulation reports needsRestore.
        QJsonObject simJson;
        simJson.insert("latestLedger", 1000);
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
        srv.sendResponses = { sendStatus(QStringLiteral("PENDING")) };
        srv.fetchResponses = { txStatus(QStringLiteral("SUCCESS")) };

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

    /**
     * A restore the RPC never took in hands back the sequence number it took,
     * as ContractClient::invoke does: refused (ERROR), turned away
     * (TRY_AGAIN_LATER), a send whose connection never opened, or a signer
     * that could not sign. The account used to stay advanced, and the next
     * transaction built on it was refused with tx_bad_seq; TRY_AGAIN_LATER was
     * polled as if pending. (The send that throws here used to be any
     * runtime_error; it is now the one SorobanServer throws for a connection
     * that never opened. A send that may have reached the RPC keeps the
     * number: see the next test.)
     */
    void testRestoreFootprintHandsBackANumberTheRpcNeverTookIn()
    {
        enum Outcome { Error, TryAgainLater, SendThrows, CannotSign };
        for (const Outcome outcome : { Error, TryAgainLater, SendThrows, CannotSign }) {
            StubAuthSorobanServer srv;
            srv.simResponses = { makeRestoreSimResponse() };
            srv.sendResponses = { sendStatus(outcome == Error ? QStringLiteral("ERROR")
                                                              : QStringLiteral("TRY_AGAIN_LATER")) };
            srv.sendFailure = outcome == SendThrows ? StubAuthSorobanServer::NeverConnected
                                                    : StubAuthSorobanServer::NoFailure;
            srv.fetchResponses = { txStatus(QStringLiteral("SUCCESS")) };

            QList<stellar::SorobanAuthorizationEntry> noAuth;
            AssembledTransaction at(buildTxWithAuth(noAuth), &srv, Network::testnetNetwork());
            at.simulate();

            Account account(sourceKeyPair(), 99);
            KeyPair* signer = outcome == CannotSign
                ? KeyPair::fromAccountId(sourceKeyPair()->getAccountId())
                : sourceKeyPair();
            bool threw = false;
            try {
                at.restoreFootprint(&account, signer, 100, 5000, 1);
            } catch (const std::exception&) {
                threw = true;
            }
            QVERIFY2(threw, qPrintable(QString::number(int(outcome))));
            QCOMPARE(account.getSequenceNumber(), qint64(99));
            QCOMPARE(srv.fetchIdx, 0);                               // nothing to poll for
            QCOMPARE(int(srv.capturedSendEnvelopes.size()), outcome == CannotSign ? 0 : 1);
            if (outcome != CannotSign)
                QCOMPARE(sentSequence(srv.capturedSendEnvelopes.first()), qint64(100));

            // The retry is signed with the number the failed one gave back.
            srv.sendFailure = StubAuthSorobanServer::NoFailure;
            srv.sendResponses = { sendStatus(QStringLiteral("PENDING")) };
            srv.sendIdx = 0;
            const GetTransactionResponse r =
                at.restoreFootprint(&account, sourceKeyPair(), 100, 5000, 1);
            QCOMPARE(r.getStatus(), GetTransactionResponse::Status::SUCCESS);
            QCOMPARE(sentSequence(srv.capturedSendEnvelopes.last()), qint64(100));
            QCOMPARE(account.getSequenceNumber(), qint64(100));
        }
    }

    /**
     * A send that may have reached the RPC keeps the restore's sequence
     * number, and says so with the hash to look up: a timeout, a cancel, a
     * connection lost midway, an RPC error, an answer that does not parse.
     * Every one of these handed the number back, so the next transaction
     * built on the account could take the number of a restore that then
     * landed, and was refused with tx_bad_seq.
     */
    void testRestoreFootprintKeepsTheNumberWhenTheSendMayHaveReachedTheRpc()
    {
        using Failure = StubAuthSorobanServer::SendFailure;
        const QList<Failure> failures = { StubAuthSorobanServer::TimedOut, StubAuthSorobanServer::Cancelled,
                                          StubAuthSorobanServer::LostMidway, StubAuthSorobanServer::RpcError,
                                          StubAuthSorobanServer::Unparsable };
        for (const Failure failure : failures) {
            StubAuthSorobanServer srv;
            srv.simResponses = { makeRestoreSimResponse() };
            srv.sendResponses = { sendStatus(QStringLiteral("PENDING")) };
            srv.sendFailure = failure;
            srv.fetchResponses = { txStatus(QStringLiteral("SUCCESS")) };

            QList<stellar::SorobanAuthorizationEntry> noAuth;
            AssembledTransaction at(buildTxWithAuth(noAuth), &srv, Network::testnetNetwork());
            at.simulate();

            Account account(sourceKeyPair(), 99);
            bool typed = false;
            QString message;
            QString hash;
            try {
                at.restoreFootprint(&account, sourceKeyPair(), 100, 5000, 1);
                message = QStringLiteral("no exception");
            } catch (const qstellar::exception::SendOutcomeUnknownException& e) {
                typed = true;
                message = QString::fromUtf8(e.what());
                hash = e.getTransactionHash();
            } catch (const std::exception& e) {
                message = QString::fromUtf8(e.what());
            }
            const QByteArray label = QByteArray::number(int(failure)) + ": " + message.toUtf8();
            QVERIFY2(typed, label.constData());
            QCOMPARE(account.getSequenceNumber(), qint64(100));        // kept
            QCOMPARE(srv.fetchIdx, 0);
            QCOMPARE(int(srv.capturedSendEnvelopes.size()), 1);
            QScopedPointer<AbstractTransaction> sent(AbstractTransaction::fromEnvelopeXdr(
                srv.capturedSendEnvelopes.first(), Network::testnetNetwork()));
            QCOMPARE(hash, sent->hashHex());
            QVERIFY2(message.contains(hash) && message.contains(QStringLiteral("look transaction")),
                     label.constData());
        }
    }

    /**
     * Over a real connection SorobanServer says which failures never left: a
     * connection that never opens hands the number back, and a send that
     * timed out or was cancelled keeps it. All three used to hand it back,
     * with the same plain runtime_error.
     */
    void testRestoreFootprintOverHttpKeepsTheNumberUnlessTheConnectionNeverOpened()
    {
        QList<stellar::SorobanAuthorizationEntry> noAuth;
        {
            QTcpServer probe;
            probe.listen(QHostAddress::LocalHost, 0);
            const quint16 port = probe.serverPort();
            probe.close();                          // nothing listens there now
            HttpSendingSorobanServer srv(QUrl(QStringLiteral("http://127.0.0.1:%1/").arg(port)));
            srv.sim = makeRestoreSimResponse();
            AssembledTransaction at(buildTxWithAuth(noAuth), &srv, Network::testnetNetwork());
            at.simulate();

            Account account(sourceKeyPair(), 99);
            bool neverSent = false;
            QString message;
            try {
                at.restoreFootprint(&account, sourceKeyPair(), 100, 5000, 1);
            } catch (const qstellar::exception::SorobanTransportException& e) {
                neverSent = !e.mayHaveReachedServer();
                message = QString::fromUtf8(e.what());
            } catch (const std::exception& e) {
                message = QString::fromUtf8(e.what());
            }
            QVERIFY2(neverSent, qPrintable(message));
            QCOMPARE(account.getSequenceNumber(), qint64(99));         // handed back
        }
        for (const bool cancel : { false, true }) {
            NeverAnsweringEndpoint endpoint;
            HttpSendingSorobanServer srv(endpoint.url());
            srv.setRequestTimeout(cancel ? 10000 : 300);
            srv.sim = makeRestoreSimResponse();
            AssembledTransaction at(buildTxWithAuth(noAuth), &srv, Network::testnetNetwork());
            at.simulate();

            Account account(sourceKeyPair(), 99);
            if (cancel)
                QTimer::singleShot(200, &srv, &SorobanServer::cancelRequests);
            QElapsedTimer clock;
            clock.start();
            bool typed = false;
            QString message;
            try {
                at.restoreFootprint(&account, sourceKeyPair(), 100, 5000, 1);
            } catch (const qstellar::exception::SendOutcomeUnknownException& e) {
                typed = true;
                message = QString::fromUtf8(e.what());
            } catch (const std::exception& e) {
                message = QString::fromUtf8(e.what());
            }
            QVERIFY2(typed, qPrintable(message));
            QVERIFY2(message.contains(cancel ? QStringLiteral("cancelled")
                                             : QStringLiteral("no answer within 300 ms")),
                     qPrintable(message));
            QCOMPARE(account.getSequenceNumber(), qint64(100));        // kept
            QVERIFY2(clock.elapsed() < 5000, qPrintable(QString::number(clock.elapsed())));
        }
    }

    /**
     * A restore sent on a kept-alive connection that then drops keeps its
     * number, whatever error ends the send. Qt 5.15 sends it again on a new
     * connection, and when that is refused the reply ends with
     * ConnectionRefusedError — a connection that never opened — although the
     * RPC had read the whole sendTransaction on the first: taken for one that
     * never left, the number was handed back while the restore could land.
     *
     * A run in which Qt closed the kept-alive connection itself before the
     * send (see KeptAliveThenDroppedEndpoint) drops no kept-alive connection,
     * so it proves nothing here and is run again, up to 10 times; the number
     * must still be kept. It used to be judged on that run, and failed about
     * once in 35 runs under load.
     */
    void testRestoreFootprintKeepsTheNumberWhenAKeptAliveConnectionDrops()
    {
        QList<stellar::SorobanAuthorizationEntry> noAuth;
        const int runs = 10;
        for (int run = 1; run <= runs; ++run) {
            KeptAliveThenDroppedEndpoint endpoint;
            HttpSendingSorobanServer srv(endpoint.url());
            srv.setRequestTimeout(15000);
            srv.sim = makeRestoreSimResponse();
            AssembledTransaction at(buildTxWithAuth(noAuth), &srv, Network::testnetNetwork());
            at.simulate();
            srv.sendRequest(QStringLiteral("getHealth"));   // answered; the connection stays open

            Account account(sourceKeyPair(), 99);
            bool typed = false;
            QString message;
            QString hash;
            try {
                at.restoreFootprint(&account, sourceKeyPair(), 100, 5000, 1);
                message = QStringLiteral("no exception");
            } catch (const qstellar::exception::SendOutcomeUnknownException& e) {
                typed = true;
                message = QString::fromUtf8(e.what());
                hash = e.getTransactionHash();
            } catch (const std::exception& e) {
                message = QString::fromUtf8(e.what());
            }
            // Over whichever connection, the RPC read the whole send.
            QVERIFY2(typed, qPrintable(message));
            QCOMPARE(account.getSequenceNumber(), qint64(100));        // kept
            QCOMPARE(hash.size(), 64);
            QCOMPARE(endpoint.requests(), 2);
            QVERIFY(endpoint.lastRequest().contains("\"method\":\"sendTransaction\""));
            if (endpoint.carriers().at(1) != endpoint.carriers().at(0)) {
                // Sent over a new connection: no kept-alive drop to judge.
                qInfo("run %d: Qt closed the kept-alive connection before the restore; running again", run);
                continue;
            }
            // Both requests over the one kept-alive connection.
            QCOMPARE(endpoint.connections(), 1);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            // What the guard is for: Qt 5.15 sent it again, and that connection was refused.
            QVERIFY2(message.contains(QStringLiteral("Connection refused")), qPrintable(message));
#endif
            return;
        }
        QFAIL(qPrintable(QStringLiteral("in %1 runs the restore never travelled over the kept-alive "
                                        "connection: Qt closed it every time").arg(runs)));
    }

    /**
     * The RPC read the send whole and its connection closed with no answer,
     * so the RPC may have taken the transaction in; Qt 5 sends it again, by
     * itself, and the RPC answers that later send. An ERROR (here txBAD_SEQ,
     * what the RPC answers a transaction sent again once the first has been
     * applied) or a TRY_AGAIN_LATER answers the later send only:
     * restoreFootprint keeps the restore's sequence number and throws
     * SendOutcomeUnknownException with its hash, and signAndSend throws
     * SendOutcomeUnknownException with the transaction's hash. Both took the
     * answer as the send's, "not taken in" (SendFailedException), and
     * restoreFootprint handed the number back while the restore could land.
     * PENDING says the RPC has it: polled, as ever. Qt 6 does not send it
     * again: the send fails as a connection lost after it was written, and
     * the outcome is not known either (restoreFootprint keeps the number).
     */
    void testAnAnswerToASendQtSentAgainIsNotFinal_data()
    {
        QTest::addColumn<QString>("status");
        QTest::addColumn<bool>("restore");
        for (const char* status : { "ERROR", "TRY_AGAIN_LATER", "PENDING" }) {
            QTest::newRow((QByteArray(status) + ", restoreFootprint").constData())
                << QString::fromLatin1(status) << true;
            QTest::newRow((QByteArray(status) + ", signAndSend").constData())
                << QString::fromLatin1(status) << false;
        }
    }
    void testAnAnswerToASendQtSentAgainIsNotFinal()
    {
        QFETCH(QString, status);
        QFETCH(bool, restore);
        FirstSendDroppedEndpoint endpoint(status);
        HttpSendingSorobanServer srv(endpoint.url());
        srv.setRequestTimeout(15000);
        srv.sim = restore ? makeRestoreSimResponse() : makeSimResponse(10);
        QList<stellar::SorobanAuthorizationEntry> noAuth;
        AssembledTransaction at(buildTxWithAuth(noAuth), &srv, Network::testnetNetwork());
        at.simulate();

        Account account(sourceKeyPair(), 99);
        bool returned = false, unknown = false, reached = false;
        QString message;
        QString hash;
        GetTransactionResponse outcome;
        try {
            outcome = restore ? at.restoreFootprint(&account, sourceKeyPair(), 100, 5000, 1)
                              : at.signAndSend(sourceKeyPair(), 5000, 1);
            returned = true;
            message = QStringLiteral("no exception");
        } catch (const qstellar::exception::SendOutcomeUnknownException& e) {
            unknown = true;
            message = QString::fromUtf8(e.what());
            hash = e.getTransactionHash();
        } catch (const qstellar::exception::SorobanTransportException& e) {
            reached = e.mayHaveReachedServer();
            message = QString::fromUtf8(e.what());
        } catch (const std::exception& e) {
            message = QString::fromUtf8(e.what());
        }

        // The one transaction, read whole each time it was sent.
        const QStringList sent = endpoint.envelopes();
        QVERIFY(!sent.isEmpty());
        for (const QString& envelope : sent)
            QCOMPARE(envelope, sent.first());
        QScopedPointer<AbstractTransaction> tx(
            AbstractTransaction::fromEnvelopeXdr(sent.first(), Network::testnetNetwork()));
        const QString txHash = tx->hashHex();
        if (restore)
            QCOMPARE(sentSequence(sent.first()), qint64(100));
        else
            QCOMPARE(txHash, at.getTransaction()->hashHex());
        const int sends = int(sent.size());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QVERIFY2(sends == 2, qPrintable(message));      // Qt 5 sends it again, and it is answered
#endif
        if (sends == 1) {
            // Not sent again: lost after it was written, it may land.
            qInfo("sent once, not again: %s", qPrintable(message));
            if (restore) {
                QVERIFY2(unknown, qPrintable(message));
                QCOMPARE(hash, txHash);
                QCOMPARE(account.getSequenceNumber(), qint64(100));    // kept
            } else {
                QVERIFY2(reached, qPrintable(message));
            }
            return;
        }
        QCOMPARE(sends, 2);
        if (status == QLatin1String("PENDING")) {
            QVERIFY2(returned, qPrintable(message));
            QCOMPARE(outcome.getStatus(), GetTransactionResponse::Status::SUCCESS);
            if (restore)
                QCOMPARE(account.getSequenceNumber(), qint64(100));
            return;
        }
        QVERIFY2(unknown, qPrintable(message));
        QCOMPARE(hash, txHash);
        QVERIFY2(message.contains(txHash), qPrintable(message));
        QVERIFY2(message.contains(QStringLiteral("with ") + status), qPrintable(message));
        if (restore)
            QCOMPARE(account.getSequenceNumber(), qint64(100));        // kept, not handed back
    }

    /**
     * Once the RPC has taken the restore in (PENDING), a polling failure
     * throws SendOutcomeUnknownException with the restore's hash, and the
     * number stays taken. A poll whose own connection never opened came
     * through as SorobanTransportException "the request was not sent" —
     * what a handed-back number is told by — and one that timed out as a
     * plain error; neither carried the hash, which the caller has no other
     * way to learn.
     */
    void testRestoreFootprintSaysTheOutcomeIsUnknownWhenPollingFails()
    {
        QList<stellar::SorobanAuthorizationEntry> noAuth;
        QTcpServer probe;
        probe.listen(QHostAddress::LocalHost, 0);
        const quint16 port = probe.serverPort();
        probe.close();                              // nothing listens there now
        for (const bool timesOut : { false, true }) {
            // Polls over HTTP to a closed port, or answers NOT_FOUND until the timeout.
            PollingOverHttpSorobanServer http(QUrl(QStringLiteral("http://127.0.0.1:%1/").arg(port)));
            http.sim = makeRestoreSimResponse();
            http.sent = sendStatus(QStringLiteral("PENDING"));
            StubAuthSorobanServer stub;
            stub.simResponses = { makeRestoreSimResponse() };
            stub.sendResponses = { sendStatus(QStringLiteral("PENDING")) };
            stub.fetchResponses = { txStatus(QStringLiteral("NOT_FOUND")) };
            SorobanServer* srv = timesOut ? static_cast<SorobanServer*>(&stub) : &http;

            AssembledTransaction at(buildTxWithAuth(noAuth), srv, Network::testnetNetwork());
            at.simulate();
            Account account(sourceKeyPair(), 99);
            bool typed = false;
            QString message;
            QString hash;
            try {
                at.restoreFootprint(&account, sourceKeyPair(), 100, timesOut ? 20 : 10000, 1);
                message = QStringLiteral("no exception");
            } catch (const qstellar::exception::SendOutcomeUnknownException& e) {
                typed = true;
                message = QString::fromUtf8(e.what());
                hash = e.getTransactionHash();
            } catch (const std::exception& e) {
                message = QString::fromUtf8(e.what());
            }
            QVERIFY2(typed, qPrintable(message));
            QCOMPARE(account.getSequenceNumber(), qint64(100));    // kept
            const QStringList sent = timesOut ? stub.capturedSendEnvelopes : http.envelopes;
            QCOMPARE(int(sent.size()), 1);
            QScopedPointer<AbstractTransaction> tx(
                AbstractTransaction::fromEnvelopeXdr(sent.first(), Network::testnetNetwork()));
            QCOMPARE(hash, tx->hashHex());
            QVERIFY2(message.contains(hash) && message.contains(QStringLiteral("polling")),
                     qPrintable(message));
        }
    }

    /**
     * Once the RPC has answered PENDING or DUPLICATE the restore may land
     * whatever the polling says, so its number stays taken: a failed
     * transaction used it up, and one still unconfirmed at the timeout may.
     */
    void testRestoreFootprintKeepsANumberTheRpcTookIn()
    {
        struct Case { const char* send; const char* poll; bool throws; };
        const QList<Case> cases = {
            { "PENDING", "FAILED", false },
            { "PENDING", "NOT_FOUND", true },        // polling times out
            { "DUPLICATE", "SUCCESS", false },
        };
        for (const Case& c : cases) {
            StubAuthSorobanServer srv;
            srv.simResponses = { makeRestoreSimResponse() };
            srv.sendResponses = { sendStatus(QString::fromLatin1(c.send)) };
            srv.fetchResponses = { txStatus(QString::fromLatin1(c.poll)) };

            QList<stellar::SorobanAuthorizationEntry> noAuth;
            AssembledTransaction at(buildTxWithAuth(noAuth), &srv, Network::testnetNetwork());
            at.simulate();

            Account account(sourceKeyPair(), 99);
            bool threw = false;
            try {
                at.restoreFootprint(&account, sourceKeyPair(), 100, /*timeoutMs*/ 20, 1);
            } catch (const std::exception&) {
                threw = true;
            }
            QVERIFY2(threw == c.throws, c.send);
            QCOMPARE(account.getSequenceNumber(), qint64(100));
        }
    }

    /**
     * The send waits in a nested event loop, where another transaction can
     * be built on the same Account. A refused restore then cannot hand its
     * number back: the account holds a later one, which it keeps.
     */
    void testRestoreFootprintLeavesANumberTakenMeanwhile()
    {
        StubAuthSorobanServer srv;
        srv.simResponses = { makeRestoreSimResponse() };
        srv.sendResponses = { sendStatus(QStringLiteral("ERROR")) };

        QList<stellar::SorobanAuthorizationEntry> noAuth;
        AssembledTransaction at(buildTxWithAuth(noAuth), &srv, Network::testnetNetwork());
        at.simulate();

        Account account(sourceKeyPair(), 99);
        srv.buildsMeanwhile = &account;
        bool threw = false;
        try {
            at.restoreFootprint(&account, sourceKeyPair(), 100, 5000, 1);
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
        QCOMPARE(account.getSequenceNumber(), qint64(101));   // 100 restore, 101 the other build
    }
};

ADD_TEST(AssembledTransactionAuthTest)
#endif // ASSEMBLEDTRANSACTIONAUTHTEST_H
