#ifndef SOROBANSERVERHELPERSTEST_H
#define SOROBANSERVERHELPERSTEST_H

#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QtTest>
#include "../../../src/restorefootprintoperation.h"
#include "../../../src/bumpsequenceoperation.h"
#include <cstring>
#include <functional>
#include <limits>
#include <stdexcept>
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
    /** Cancels once while a poll's request is answered, as a cancel queued
     *  from another thread lands in the batch of events that ends it. */
    bool cancelInFetch = false;
protected:
    GetTransactionResponse fetchTransaction(const QString& /*hash*/) override {
        if (cancelInFetch) {
            cancelInFetch = false;
            cancelRequests();
        }
        QJsonObject o;
        const auto s = script.at(qMin(idx, int(script.size() - 1)));
        switch (s) {
        case GetTransactionResponse::Status::NOT_FOUND: o.insert("status", QStringLiteral("NOT_FOUND")); break;
        case GetTransactionResponse::Status::SUCCESS:   o.insert("status", QStringLiteral("SUCCESS")); break;
        case GetTransactionResponse::Status::FAILED:    o.insert("status", QStringLiteral("FAILED")); break;
        }
        // The rest of the answer, whole as the RPC sends it: the ledger range,
        // and the transaction's own fields once it is found (its XDR
        // placeholders, which nothing here decodes).
        const bool found = s != GetTransactionResponse::Status::NOT_FOUND;
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
        ++idx;
        return GetTransactionResponse::fromJson(o);
    }
};

/** SorobanServer whose simulateTransaction answers with `sim`, for driving
 *  prepareTransaction without an RPC endpoint. */
class PreparingSorobanServer : public SorobanServer
{
public:
    PreparingSorobanServer() : SorobanServer(QUrl("http://127.0.0.1:1/")) {}
    SimulateTransactionResponse sim;
    int simulations = 0;
    SimulateTransactionResponse simulateTransactionXdr(const QString&) override {
        ++simulations;
        return sim;
    }
};

/** An endpoint that takes every connection and never answers, and hangs up
 *  after `hangUpMs` so a test without a timeout fails instead of hanging. */
class SilentRpcEndpoint
{
public:
    explicit SilentRpcEndpoint(int hangUpMs)
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
        m_hangUp.start(hangUpMs);
    }
    QUrl url() const { return QUrl(QStringLiteral("http://127.0.0.1:%1/").arg(m_server.serverPort())); }

private:
    QTcpServer m_server;
    QTimer m_hangUp;
};

/** An endpoint that reads what it is sent and hangs up without answering:
 *  a connection lost after the request was written. */
class HangingUpRpcEndpoint
{
public:
    HangingUpRpcEndpoint()
    {
        QObject::connect(&m_server, &QTcpServer::newConnection, [this]() {
            while (m_server.hasPendingConnections()) {
                QTcpSocket* socket = m_server.nextPendingConnection();
                QObject::connect(socket, &QTcpSocket::readyRead, socket, [this, socket]() {
                    m_received += socket->readAll().size();
                    QTimer::singleShot(50, socket, [socket]() { socket->abort(); });
                });
            }
        });
        m_server.listen(QHostAddress::LocalHost, 0);
    }
    QUrl url() const { return QUrl(QStringLiteral("http://127.0.0.1:%1/").arg(m_server.serverPort())); }
    qint64 received() const { return m_received; }

private:
    QTcpServer m_server;
    qint64 m_received = 0;
};

/**
 * An endpoint that answers the first request of a connection and keeps the
 * connection alive, reads the second request in full, then stops listening
 * and drops the connection without an answer: the whole second request
 * reached it, and a connection made to send it again is refused.
 *
 * It records which connection carried each request, because the second one
 * does not always travel over the kept-alive connection: now and then Qt
 * closes that connection itself as the first answer arrives. When the
 * answer is already there as Qt finishes writing the request, Qt queues a
 * read of it; if the socket's own readyRead reads it first, the queued read
 * finds no reply left and closes the connection, and the second request
 * goes out over a new one. Seen about once in 35 runs under load.
 */
class KeepAliveThenDropRpcEndpoint
{
public:
    KeepAliveThenDropRpcEndpoint()
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
 * An endpoint that reads every request whole, drops the connection of the
 * first `drops` of them without an answer (read whole, never answered), and
 * answers every one after with `answer`, keeping the connection alive. Qt 5
 * sends a request again, by itself, on a new connection, when the one it was
 * written on closes before an answer: the request dropped is read again, and
 * that later send is answered. It keeps the body of every request read
 * whole, in order.
 */
class DropsThenAnswersRpcEndpoint
{
public:
    DropsThenAnswersRpcEndpoint(int drops, const QByteArray& answer) : m_drops(drops), m_answer(answer)
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
    /** The body of every request read whole, in order. */
    QList<QByteArray> bodies() const { return m_bodies; }

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
            m_bodies.append(buffer.mid(end + 4, length));
            buffer.remove(0, end + 4 + length);
            if (m_bodies.size() <= m_drops) {
                buffer.clear();
                socket->abort();        // read whole, never answered
                return;
            }
            socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                          "Connection: keep-alive\r\nContent-Length: "
                          + QByteArray::number(m_answer.size()) + "\r\n\r\n" + m_answer);
        }
    }

    QTcpServer m_server;
    QHash<QTcpSocket*, QByteArray> m_buffers;
    int m_drops;
    QByteArray m_answer;
    QList<QByteArray> m_bodies;
};

/**
 * An endpoint that reads each request whole and drops the first, so that Qt 5
 * sends it again by itself, then ends the second as `mode` says: "500" answers
 * it with a 500, "partial" writes half a status line and closes, "hold" never
 * answers (the request times out), and "once-error" answers the first request
 * with an ERROR and drops nothing (a single send). Qt 6 does not send again.
 */
class ResendEndingEndpoint
{
public:
    explicit ResendEndingEndpoint(const QString& mode) : m_mode(mode)
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
    int count = 0;
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
            buffer.remove(0, end + 4 + length);
            ++count;
            const QByteArray err = "{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"result\":{\"status\":\"ERROR\","
                "\"hash\":\"1c2eb969d67a1846070bbbb799fcfc2c58c5272f9dc597f67c0e469726dfea96\","
                "\"latestLedger\":4919962,\"latestLedgerCloseTime\":\"1790623397\","
                "\"errorResultXdr\":\"AAAAAAAAAGT////7AAAAAA==\"}}";
            if (m_mode == QLatin1String("once-error")) {
                socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: keep-alive\r\nContent-Length: "
                              + QByteArray::number(err.size()) + "\r\n\r\n" + err);
                continue;
            }
            if (count == 1) {
                if (m_mode == QLatin1String("partial")) {
                    socket->write("HTTP/1.1 2");
                    socket->flush();
                    socket->disconnectFromHost();
                    return;
                }
                buffer.clear();
                socket->abort();
                return;
            }
            if (m_mode == QLatin1String("hold"))
                return;
            if (m_mode == QLatin1String("500")) {
                socket->write("HTTP/1.1 500 Internal Server Error\r\nContent-Type: application/json\r\nConnection: keep-alive\r\nContent-Length: "
                              + QByteArray::number(err.size()) + "\r\n\r\n" + err);
                continue;
            }
            socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: keep-alive\r\nContent-Length: "
                          + QByteArray::number(err.size()) + "\r\n\r\n" + err);
        }
    }
    QTcpServer m_server;
    QHash<QTcpSocket*, QByteArray> m_buffers;
    QString m_mode;
};

/** SorobanServer whose JSON-RPC answer is `answer`, parsed as a real one is,
 *  recording what was asked. */
class AnsweringSorobanServer : public SorobanServer
{
public:
    AnsweringSorobanServer() : SorobanServer(QUrl("http://127.0.0.1:1/")) {}
    QByteArray answer;
    QStringList methods;
    QJsonValue lastParams;
    QJsonValue sendRequest(const QString& method, const QJsonValue& params) override {
        methods.append(method);
        lastParams = params;
        return parseRpcResult(answer);
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

    /** A local port nothing listens on: a connection to it never opens. */
    static QUrl closedPortUrl()
    {
        QTcpServer probe;
        probe.listen(QHostAddress::LocalHost, 0);
        const quint16 port = probe.serverPort();
        probe.close();
        return QUrl(QStringLiteral("http://127.0.0.1:%1/").arg(port));
    }

    /** What a request's failure said about where it got to. */
    struct TransportOutcome
    {
        bool typed = false;             // a SorobanTransportException
        bool mayHaveReachedServer = false;
        QString message;
    };
    static TransportOutcome sendAndCatch(SorobanServer& server)
    {
        TransportOutcome out;
        try {
            server.sendTransactionXdr(QStringLiteral("AAAAAgAAAAA="));
            out.message = QStringLiteral("no exception");
        } catch (const qstellar::exception::SorobanTransportException& e) {
            out.typed = true;
            out.mayHaveReachedServer = e.mayHaveReachedServer();
            out.message = QString::fromUtf8(e.what());
        } catch (const std::exception& e) {
            out.message = QString::fromUtf8(e.what());
        }
        return out;
    }

    // ── SAC balance fixtures ──────────────────────────────────────────

    /** The holder every fixture below is about: a testnet contract. */
    static QString sacHolder() { return QStringLiteral("CDAM4O427TUPATTJXWYYMFX5UOLPTQXOLWGOB6DAH6I3EDS4RPUGR3JL"); }

    /**
     * A real getLedgerEntries answer from https://soroban-testnet.stellar.org
     * (ledger 4916136) for sacHolder()'s XLM balance: 741892200000 stroops,
     * authorized, no clawback. js-stellar-sdk 17.1.0 reads it the same.
     */
    static QByteArray realSacBalanceAnswer()
    {
        return QByteArray(
            "{\"jsonrpc\":\"2.0\",\"id\":8675309,\"result\":{\"entries\":[{\"key\":\"AAAABgAAAAHXkotywnA8z+r365/0701QSlWouXn8m0UOoshCtNHOYQAAABAAAAABAAAAAgAAAA8AAAAHQmFsYW5jZQAAAAASAAAAAcDOO5r86PBOab2xhhb9o5b5wu5djOD4YD+Rsg5ci+hoAAAAAQ==\","
            "\"xdr\":\"AAAABgAAAAAAAAAB15KLcsJwPM/q9+uf9O9NUEpVqLl5/JtFDqLIQrTRzmEAAAAQAAAAAQAAAAIAAAAPAAAAB0JhbGFuY2UAAAAAEgAAAAHAzjua/OjwTmm9sYYW/aOW+cLuXYzg+GA/kbIOXIvoaAAAAAEAAAARAAAAAQAAAAMAAAAPAAAABmFtb3VudAAAAAAACgAAAAAAAAAAAAAArLw4lkAAAAAPAAAACmF1dGhvcml6ZWQAAAAAAAAAAAABAAAADwAAAAhjbGF3YmFjawAAAAAAAAAA\","
            "\"lastModifiedLedgerSeq\":4916033,\"liveUntilLedgerSeq\":5425357,\"extXdr\":\"AAAAAA==\"}],\"latestLedger\":4916136}}");
    }
    /** Its key: sacHolder()'s XLM balance on testnet, as js-stellar-sdk builds it. */
    static QString nativeBalanceKey()
    {
        return QStringLiteral("AAAABgAAAAHXkotywnA8z+r365/0701QSlWouXn8m0UOoshCtNHOYQAAABAAAAABAAAAAgAAAA8AAAAHQmFsYW5jZQAAAAASAAAAAcDOO5r86PBOab2xhhb9o5b5wu5djOD4YD+Rsg5ci+hoAAAAAQ==");
    }
    /**
     * A LedgerEntryData built with js-stellar-sdk 17.1.0 (xdr.LedgerEntryData
     * .contractData): sacHolder()'s balance of the SAC of USDC issued by
     * usdcIssuer() on testnet, amount the largest i128, not authorized,
     * clawback enabled.
     */
    static QString jsUsdcBalanceEntry()
    {
        return QStringLiteral("AAAABgAAAAAAAAABDVzotvgukFBdGA4r3VmdT3tsGzYIQ82sxT0TX+afmWcAAAAQAAAAAQAAAAIAAAAPAAAAB0JhbGFuY2UAAAAAEgAAAAHAzjua/OjwTmm9sYYW/aOW+cLuXYzg+GA/kbIOXIvoaAAAAAEAAAARAAAAAQAAAAMAAAAPAAAABmFtb3VudAAAAAAACn////////////////////8AAAAPAAAACmF1dGhvcml6ZWQAAAAAAAAAAAAAAAAADwAAAAhjbGF3YmFjawAAAAAAAAAB");
    }
    static QString usdcIssuer() { return QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"); }
    /** Its SAC, as js-stellar-sdk computes Asset.contractId. */
    static QString usdcSac() { return QStringLiteral("CAGVZ2FW7AXJAUC5DAHCXXKZTVHXW3A3GYEEHTNMYU6RGX7GT6MWOIIH"); }
    /**
     * A LedgerEntryData built with py-stellar-sdk 14.1.0: sacHolder()'s XLM
     * balance on testnet (nativeBalanceKey()), amount 2^64 + 5, authorized,
     * clawback enabled.
     */
    static QString pyNativeBalanceEntry()
    {
        return QStringLiteral("AAAABgAAAAAAAAAB15KLcsJwPM/q9+uf9O9NUEpVqLl5/JtFDqLIQrTRzmEAAAAQAAAAAQAAAAIAAAAPAAAAB0JhbGFuY2UAAAAAEgAAAAHAzjua/OjwTmm9sYYW/aOW+cLuXYzg+GA/kbIOXIvoaAAAAAEAAAARAAAAAQAAAAMAAAAPAAAABmFtb3VudAAAAAAACgAAAAAAAAABAAAAAAAAAAUAAAAPAAAACmF1dGhvcml6ZWQAAAAAAAAAAAABAAAADwAAAAhjbGF3YmFjawAAAAAAAAAB");
    }

    /** A LedgerEntryData of contract data, laid out as stellar-xdr does. */
    static QString contractDataEntry(const QString& contract, const stellar::SCVal& key,
                                     qint32 durability, const stellar::SCVal& val)
    {
        QByteArray buf;
        QDataStream s(&buf, QIODevice::WriteOnly);
        s << static_cast<qint32>(stellar::LedgerEntryType::CONTRACT_DATA)
          << qint32(0)                                                   // ExtensionPoint
          << Scv::toAddress(contract).address << key << durability << val;
        return QString::fromLatin1(buf.toBase64(XDR_BASE64ENCODING));
    }
    static stellar::SCVal balanceKey(const QString& holder)
    {
        return Scv::toVec({ Scv::toSymbol(QStringLiteral("Balance")), Scv::toAddress(holder) });
    }
    /** A map in the order given, sorted or not. */
    static stellar::SCVal rawMap(const QList<QPair<QString, stellar::SCVal>>& pairs)
    {
        QList<stellar::SCMapEntry> entries;
        for (const auto& p : pairs) {
            stellar::SCMapEntry e;
            e.key = Scv::toSymbol(p.first);
            e.val = p.second;
            entries.append(e);
        }
        stellar::SCVal v;
        v.type = stellar::SCValType::SCV_MAP;
        v.map = QSharedPointer<QList<stellar::SCMapEntry>>::create(entries);
        return v;
    }
    /** A getLedgerEntries answer with these (key, xdr) entries. */
    static QByteArray ledgerEntriesAnswer(const QList<QPair<QString, QString>>& entries)
    {
        QJsonArray list;
        for (const auto& e : entries) {
            QJsonObject o;
            o.insert("key", e.first);
            o.insert("xdr", e.second);
            o.insert("lastModifiedLedgerSeq", 4916033);
            list.append(o);
        }
        QJsonObject result;
        result.insert("entries", list);
        result.insert("latestLedger", 4916136);
        QJsonObject envelope;
        envelope.insert("jsonrpc", QStringLiteral("2.0"));
        envelope.insert("id", QStringLiteral("1"));
        envelope.insert("result", result);
        return QJsonDocument(envelope).toJson(QJsonDocument::Compact);
    }
    /** An i128 in its 16 big-endian bytes, from hex. */
    static QByteArray i128(const char* hex) { return QByteArray::fromHex(hex); }

    // ── Asset balance fixtures ────────────────────────────────────────

    /** Testnet accounts funded by friendbot for these fixtures. */
    static QString holderA() { return QStringLiteral("GD7CN66PP3TZU4QEBW3TOZV72RPJUX7DX75J53I3YQ5FWMIIQ27UV5PQ"); }
    static QString holderB() { return QStringLiteral("GB4SHUV7QBY3QC6R6X653SZ2BVW5JD7SJERV6Q5KNG6324IJ7XOYDPQ4"); }
    static QString testIssuer() { return QStringLiteral("GAIUC2GRDLHDFUGKMSUUYWYEFCNDGJHHHCJDEEVEMCPOOKZTL4ZZBDNK"); }
    /** Circle's USDC on testnet. */
    static QString circleUsdcIssuer() { return QStringLiteral("GBBD47IF6LWK7P7MDEVSCWR7DPUWV3NY3DTQEVFL4NAT4AQH3ZLLFLA5"); }

    static stellar::Asset issued(const QString& code, const QString& issuer)
    {
        QScopedPointer<Asset> asset(Asset::createNonNativeAsset(code, issuer));
        return asset->toXdr();
    }
    static stellar::Asset nativeAsset()
    {
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        return native;
    }

    /**
     * Real getLedgerEntries answers from https://soroban-testnet.stellar.org
     * (ledger 4917043), saved as they came. holderA() trusts Circle's USDC
     * (bought on the DEX), and TESTN and TN4 of testIssuer(), which enabled
     * clawback: its TESTN line has a sell offer and a pool deposit, so it
     * carries ext v1 liabilities and ext v2, and it holds INT64_MAX TN4.
     * holderB() holds TESTN authorized to maintain liabilities only. Both
     * accounts carry the ext chain v1, v2, v3 Core gives them. The issuer
     * holds no TESTN line. js-stellar-sdk 17.1.0's getAssetBalance and
     * Horizon read them as the tests below expect.
     */
    static QByteArray realAssetAnswer(const QByteArray& which)
    {
        static const QList<QPair<QByteArray, QByteArray>> answers = {
            { "aUsdc", "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"entries\":[{\"key\":\"AAAAAQAAAAD+JvvPfueacgQNtzdmv9Reml/jv/qe7RvEOlsxCIa/SgAAAAFVU0RDAAAAAEI+fQXy7K+/7BkrIVo/G+lq7bjY5wJUq+NBPgIH3lay\",\"xdr\":\"AAAAAQAAAAD+JvvPfueacgQNtzdmv9Reml/jv/qe7RvEOlsxCIa/SgAAAAFVU0RDAAAAAEI+fQXy7K+/7BkrIVo/G+lq7bjY5wJUq+NBPgIH3layAAAAAACtCOB//////////wAAAAEAAAAA\",\"lastModifiedLedgerSeq\":4917032,\"extXdr\":\"AAAAAA==\"}],\"latestLedger\":4917043}}" },
            { "aTestn", "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"entries\":[{\"key\":\"AAAAAQAAAAD+JvvPfueacgQNtzdmv9Reml/jv/qe7RvEOlsxCIa/SgAAAAJURVNUTgAAAAAAAAAAAAAAEUFo0RrOMtDKZKlMWwQomjMk5ziSMhKkYJ7nKzNfM5A=\",\"xdr\":\"AAAAAQAAAAD+JvvPfueacgQNtzdmv9Reml/jv/qe7RvEOlsxCIa/SgAAAAJURVNUTgAAAAAAAAAAAAAAEUFo0RrOMtDKZKlMWwQomjMk5ziSMhKkYJ7nKzNfM5AAAAAC0/BaNX//////////AAAABQAAAAEAAAAAAAAAAAAAAAA7msoDAAAAAgAAAAEAAAAA\",\"lastModifiedLedgerSeq\":4917033,\"extXdr\":\"AAAAAA==\"}],\"latestLedger\":4917043}}" },
            { "aTn4", "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"entries\":[{\"key\":\"AAAAAQAAAAD+JvvPfueacgQNtzdmv9Reml/jv/qe7RvEOlsxCIa/SgAAAAFUTjQAAAAAABFBaNEazjLQymSpTFsEKJozJOc4kjISpGCe5yszXzOQ\",\"xdr\":\"AAAAAQAAAAD+JvvPfueacgQNtzdmv9Reml/jv/qe7RvEOlsxCIa/SgAAAAFUTjQAAAAAABFBaNEazjLQymSpTFsEKJozJOc4kjISpGCe5yszXzOQf/////////9//////////wAAAAUAAAAA\",\"lastModifiedLedgerSeq\":4917031,\"extXdr\":\"AAAAAA==\"}],\"latestLedger\":4917043}}" },
            { "bTestn", "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"entries\":[{\"key\":\"AAAAAQAAAAB5I9K/gHG4C9H1/d3LOg1t1I/ySSNfQ6ppvb1xCf3dgQAAAAJURVNUTgAAAAAAAAAAAAAAEUFo0RrOMtDKZKlMWwQomjMk5ziSMhKkYJ7nKzNfM5A=\",\"xdr\":\"AAAAAQAAAAB5I9K/gHG4C9H1/d3LOg1t1I/ySSNfQ6ppvb1xCf3dgQAAAAJURVNUTgAAAAAAAAAAAAAAEUFo0RrOMtDKZKlMWwQomjMk5ziSMhKkYJ7nKzNfM5AAAAAAGQixAQAAAAukO3QAAAAABgAAAAA=\",\"lastModifiedLedgerSeq\":4917034,\"extXdr\":\"AAAAAA==\"}],\"latestLedger\":4917043}}" },
            { "issuerTestn", "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"entries\":[],\"latestLedger\":4917043}}" },
            { "aXlm", "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"entries\":[{\"key\":\"AAAAAAAAAAD+JvvPfueacgQNtzdmv9Reml/jv/qe7RvEOlsxCIa/Sg==\",\"xdr\":\"AAAAAAAAAAD+JvvPfueacgQNtzdmv9Reml/jv/qe7RvEOlsxCIa/SgAAABc2SPcEAEsHIwAAAAMAAAAGAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAABAAAAAaE7hhUAAAAAAAAAAAAAAAIAAAAAAAAAAAAAAAAAAAADAAAAAABLBykAAAAAarqFcA==\",\"lastModifiedLedgerSeq\":4917033,\"extXdr\":\"AAAAAA==\"}],\"latestLedger\":4917043}}" },
            { "bXlm", "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"entries\":[{\"key\":\"AAAAAAAAAAB5I9K/gHG4C9H1/d3LOg1t1I/ySSNfQ6ppvb1xCf3dgQ==\",\"xdr\":\"AAAAAAAAAAB5I9K/gHG4C9H1/d3LOg1t1I/ySSNfQ6ppvb1xCf3dgQAAABdIduecAEsHIwAAAAEAAAABAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAAIAAAAAAAAAAAAAAAAAAAADAAAAAABLByYAAAAAarqFYQ==\",\"lastModifiedLedgerSeq\":4917030,\"extXdr\":\"AAAAAA==\"}],\"latestLedger\":4917043}}" },
        };
        for (const auto& a : answers) {
            if (a.first == which)
                return a.second;
        }
        return QByteArray();
    }
    /** The first entry's key and xdr of a getLedgerEntries answer. */
    static QString answeredField(const QByteArray& answer, const char* field)
    {
        return QJsonDocument::fromJson(answer).object().value("result").toObject()
            .value("entries").toArray().first().toObject().value(field).toString();
    }
    /** The key js-stellar-sdk 17.1.0 builds for testIssuer()'s own TESTN line. */
    static QString issuerTestnKey()
    {
        return QStringLiteral("AAAAAQAAAAARQWjRGs4y0MpkqUxbBCiaMyTnOJIyEqRgnucrM18zkAAAAAJURVNUTgAAAAAAAAAAAAAAEUFo0RrOMtDKZKlMWwQomjMk5ziSMhKkYJ7nKzNfM5A=");
    }
    /**
     * LedgerEntryData built with js-stellar-sdk 17.1.0 (xdr.LedgerEntryData):
     * holderA()'s TESTN line with balance and limit INT64_MAX, authorized to
     * maintain liabilities, clawback enabled, ext v1 liabilities buying 123
     * and selling INT64_MAX, ext v2 pool use count 3; and holderA()'s account
     * with INT64_MAX XLM and ext 0 (an account untouched since liabilities).
     */
    static QString jsTestnLineEntry()
    {
        return QStringLiteral("AAAAAQAAAAD+JvvPfueacgQNtzdmv9Reml/jv/qe7RvEOlsxCIa/SgAAAAJURVNUTgAAAAAAAAAAAAAAEUFo0RrOMtDKZKlMWwQomjMk5ziSMhKkYJ7nKzNfM5B//////////3//////////AAAABgAAAAEAAAAAAAAAe3//////////AAAAAgAAAAMAAAAA");
    }
    static QString jsAccountExt0Entry()
    {
        return QStringLiteral("AAAAAAAAAAD+JvvPfueacgQNtzdmv9Reml/jv/qe7RvEOlsxCIa/Sn//////////AEsHIwAAAAMAAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAAAAAAA");
    }
    /**
     * LedgerEntryData built with py-stellar-sdk 14.1.0: holderA()'s account
     * with INT64_MAX XLM, an inflation destination, all four account flags, a
     * home domain, an ed25519 signer and a signed-payload one, and the whole
     * ext chain — v1 liabilities buying 7000000021 and selling INT64_MAX, v2
     * sponsorships (one signer sponsored), v3 sequence ledger and time; and
     * holderA()'s USDC line, 11340000 authorized, with ext v1 liabilities
     * (buying INT64_MAX - 11340000, selling 11340000) and no v2.
     */
    static QString pyAccountFullEntry()
    {
        return QStringLiteral("AAAAAAAAAAD+JvvPfueacgQNtzdmv9Reml/jv/qe7RvEOlsxCIa/Sn//////////AEsHIwAAAAMAAAAGAAAAAQAAAAARQWjRGs4y0MpkqUxbBCiaMyTnOJIyEqRgnucrM18zkAAAAA8AAAALZXhhbXBsZS5jb20AAQIDBAAAAAIAAAAAEUFo0RrOMtDKZKlMWwQomjMk5ziSMhKkYJ7nKzNfM5AAAAABAAAAAxFBaNEazjLQymSpTFsEKJozJOc4kjISpGCe5yszXzOQAAAABQECAwQFAAAAAAAAAgAAAAEAAAABoTuGFX//////////AAAAAgAAAAEAAAACAAAAAgAAAAAAAAABAAAAABFBaNEazjLQymSpTFsEKJozJOc4kjISpGCe5yszXzOQAAAAAwAAAAAASwcpAAAAAGq6hXA=");
    }
    static QString pyUsdcLineEntry()
    {
        return QStringLiteral("AAAAAQAAAAD+JvvPfueacgQNtzdmv9Reml/jv/qe7RvEOlsxCIa/SgAAAAFVU0RDAAAAAEI+fQXy7K+/7BkrIVo/G+lq7bjY5wJUq+NBPgIH3layAAAAAACtCOB//////////wAAAAEAAAABf/////9S9x8AAAAAAK0I4AAAAAA=");
    }
    /** A LedgerEntryData of this trustline or account, as base64. */
    static QString trustLineData(const stellar::TrustLineEntry& line)
    {
        QByteArray buf;
        QDataStream s(&buf, QIODevice::WriteOnly);
        s << static_cast<qint32>(stellar::LedgerEntryType::TRUSTLINE) << line;
        return QString::fromLatin1(buf.toBase64(XDR_BASE64ENCODING));
    }
    static QString accountData(const stellar::AccountEntry& account)
    {
        QByteArray buf;
        QDataStream s(&buf, QIODevice::WriteOnly);
        s << static_cast<qint32>(stellar::LedgerEntryType::ACCOUNT) << account;
        return QString::fromLatin1(buf.toBase64(XDR_BASE64ENCODING));
    }
    /** Base64 with its last four bytes (an ext discriminant) replaced. */
    static QString withLastWord(const QString& base64, quint32 word)
    {
        QByteArray raw = QByteArray::fromBase64(base64.toLatin1());
        QByteArray tail;
        QDataStream(&tail, QIODevice::WriteOnly) << word;
        raw.replace(raw.size() - 4, 4, tail);
        return QString::fromLatin1(raw.toBase64());
    }
    /** What getAssetBalance said: the balance, or the message it threw. */
    static QString assetBalanceOutcome(SorobanServer& server, const QString& address,
                                       const stellar::Asset& asset, Network* network)
    {
        try {
            const AssetBalance b = server.getAssetBalance(address, asset, network);
            return QStringLiteral("read as ") + (b.exists ? QString() : QStringLiteral("no entry, "))
                + Scv::decimalText(Scv::toInt128(b.amount));
        } catch (const std::exception& e) {
            return QString::fromUtf8(e.what());
        }
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    // ── applyTransactionSimulation ─────────────────────────────────────

    /**
     * Soroban data was applied to any transaction: a payment, or two
     * operations. js assembleTransaction refuses them (one invokeHostFunction,
     * extendFootprintTtl or restoreFootprint operation), and core refuses the
     * transaction. It is refused now, and the transaction is left as it was.
     */
    void testApplySimulationNeedsOneSorobanOperation()
    {
        stellar::SorobanTransactionData td =
            SorobanDataBuilder().setResources(1000u, 100u, 100u).setResourceFee(77).build();
        QJsonObject simJson;
        simJson.insert("latestLedger", 1);
        simJson.insert("transactionData", xdrToBase64(td));
        simJson.insert("minResourceFee", QStringLiteral("77"));
        const SimulateTransactionResponse sim = SimulateTransactionResponse::fromJson(simJson);
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        auto build = [&](const QList<Operation*>& ops) {
            Account* acc = new Account(sourceKeyPair(), 41);
            TransactionBuilder b(acc, Network::testnetNetwork());
            for (Operation* op : ops)
                b.addOperation(op);
            b.setBaseFee(100);
            b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
            Transaction* tx = b.build();
            delete acc;
            return tx;
        };
        QScopedPointer<Transaction> classic(build({BumpSequenceOperation::create(qint64(5))}));
        QVERIFY_EXCEPTION_THROWN(SorobanServer::applyTransactionSimulation(classic.data(), sim), std::runtime_error);
        QCOMPARE(classic->getFee(), qint64(100));
        QVERIFY(classic->getSorobanData().isNull());
        QScopedPointer<Transaction> two(build({InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(native),
                                               BumpSequenceOperation::create(qint64(5))}));
        QVERIFY_EXCEPTION_THROWN(SorobanServer::applyTransactionSimulation(two.data(), sim), std::runtime_error);
        QScopedPointer<Transaction> one(build({new RestoreFootprintOperation()}));
        SorobanServer::applyTransactionSimulation(one.data(), sim);
        QCOMPARE(one->getFee(), qint64(177));
    }

    /**
     * The resource fee charged is the one the footprint carries into the
     * envelope (777), as js-stellar-sdk 17.1.0's assembleTransaction charges
     * it — not minResourceFee (999), which this used to add, expecting 1099.
     */
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

        QCOMPARE(tx->getFee(), originalFee + 777);
        QVERIFY(!tx->getSorobanData().isNull());
        QCOMPARE(tx->getSorobanData()->resources.instructions, quint32(123456));
        QCOMPARE(tx->getSorobanData()->resourceFee, qint64(777));

        delete acc;
    }

    /**
     * The resource fee is a number from a remote server, added to the fee the
     * user signs. One that is negative, or that pushes the total past the
     * envelope's uint32 fee field, is refused before the transaction is
     * touched — toV1Xdr used to truncate such a total and sign it. (The fee
     * charged is the footprint's own, so that is where the large values go;
     * they used to be sent as minResourceFee, which is no longer charged but
     * must still be a non-negative whole number.)
     */
    void testApplySimulationRefusesAFeeTheEnvelopeCannotHold()
    {
        Account* acc = new Account(sourceKeyPair(), 43);
        TransactionBuilder b(acc, Network::testnetNetwork());
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        b.addOperation(
            InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(native));
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        QScopedPointer<Transaction> tx(b.build());
        const qint64 originalFee = tx->getFee();

        const QList<QPair<qint64, QString>> refused = {
            { qint64(4294967196LL), QString() },                    // 100 + this > uint32
            { qint64(9223372036854775807LL), QString() },
            { qint64(-1), QString() },                              // a negative footprint fee
            { qint64(1), QStringLiteral("-1") },                    // a negative minResourceFee
        };
        for (const auto& c : refused) {
            stellar::SorobanTransactionData td =
                SorobanDataBuilder().setResources(1u, 1u, 1u).setResourceFee(c.first).build();
            QJsonObject simJson;
            simJson.insert("latestLedger", 1);
            simJson.insert("transactionData", xdrToBase64(td));
            simJson.insert("minResourceFee", c.second.isEmpty() ? QString::number(c.first) : c.second);

            // (A negative minResourceFee is refused as the answer is read.)
            bool threw = false;
            try {
                SorobanServer::applyTransactionSimulation(
                    tx.data(), SimulateTransactionResponse::fromJson(simJson));
            }
            catch (const std::exception&) { threw = true; }
            QVERIFY2(threw, qPrintable(QString::number(c.first) + QLatin1Char('/') + c.second));
            QCOMPARE(tx->getFee(), originalFee);            // untouched
            QVERIFY(tx->getSorobanData().isNull());         // untouched
        }

        // The largest resource fee that still fits goes through.
        const stellar::SorobanTransactionData td =
            SorobanDataBuilder().setResources(1u, 1u, 1u).setResourceFee(4294967195LL).build();
        QJsonObject simJson;
        simJson.insert("latestLedger", 1);
        simJson.insert("transactionData", xdrToBase64(td));
        simJson.insert("minResourceFee", QStringLiteral("4294967195"));
        SorobanServer::applyTransactionSimulation(
            tx.data(), SimulateTransactionResponse::fromJson(simJson));
        QCOMPARE(tx->getFee(), qint64(4294967295LL));
        QCOMPARE(tx->toV1Xdr().fee, quint32(4294967295u));

        delete acc;
    }

    /**
     * Simulating again replaces the resource fee instead of stacking another
     * on top (js-stellar-sdk subtracts the old one too). It used to grow the
     * fee by the whole resource fee on every call — AssembledTransaction's
     * simulate() is public and meant to be called again after a restore.
     * And a response whose minResourceFee is not a number is malformed, not
     * a zero; it now leaves the transaction exactly as it was.
     */
    void testApplySimulationTwiceChargesTheResourceFeeOnce()
    {
        Account* acc = new Account(sourceKeyPair(), 44);
        TransactionBuilder b(acc, Network::testnetNetwork());
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        b.addOperation(
            InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(native));
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        QScopedPointer<Transaction> tx(b.build());

        stellar::SorobanTransactionData td =
            SorobanDataBuilder().setResources(1u, 1u, 1u).setResourceFee(50000).build();
        QJsonObject simJson;
        simJson.insert("latestLedger", 1);
        simJson.insert("transactionData", xdrToBase64(td));
        simJson.insert("minResourceFee", QStringLiteral("50000"));
        const SimulateTransactionResponse sim = SimulateTransactionResponse::fromJson(simJson);

        SorobanServer::applyTransactionSimulation(tx.data(), sim);
        QCOMPARE(tx->getFee(), qint64(50100));
        SorobanServer::applyTransactionSimulation(tx.data(), sim);
        QCOMPARE(tx->getFee(), qint64(50100));

        QJsonObject garbled = simJson;
        garbled.insert("minResourceFee", QStringLiteral("abc"));
        garbled.insert("transactionData",
                       xdrToBase64(SorobanDataBuilder().setResources(9u, 9u, 9u)
                                       .setResourceFee(9).build()));
        bool threw = false;
        try {
            SorobanServer::applyTransactionSimulation(
                tx.data(), SimulateTransactionResponse::fromJson(garbled));
        } catch (const std::exception&) {
            threw = true;
        }
        QVERIFY(threw);
        QCOMPARE(tx->getFee(), qint64(50100));                             // untouched
        QCOMPARE(tx->getSorobanData()->resourceFee, qint64(50000));         // untouched

        delete acc;
    }

    /**
     * Re-simulating recovers the inclusion fee the caller bid exactly, whatever
     * minResourceFee the RPC sends: the fee is always the bid plus the resource
     * fee the envelope's footprint carries. The expected fees are what
     * js-stellar-sdk 17.1.0's rpc.assembleTransaction gives for the same
     * sequence (877, 600, 600, 50100, 50100). This charged minResourceFee and
     * took out the footprint's resourceFee, so an RPC that sent them unequal
     * moved the bid on every call: 1099, then 1122.
     */
    void testReSimulationRecoversTheBidWhateverMinResourceFeeSays()
    {
        Account acc(sourceKeyPair(), 46);
        TransactionBuilder b(&acc, Network::testnetNetwork());
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        b.addOperation(
            InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(native));
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        QScopedPointer<Transaction> tx(b.build());

        struct Step { qint64 resourceFee; qint64 minResourceFee; qint64 fee; };
        const QList<Step> steps = {
            { 777, 999, 877 },          // minResourceFee above the footprint's
            { 500, 800, 600 },          // and again, different both times
            { 500, 500, 600 },          // equal
            { 50000, 50000, 50100 },
            { 50000, 50000, 50100 },    // the same simulation twice
            { 900, 100, 1000 },         // minResourceFee below the footprint's
        };
        for (const Step& step : steps) {
            QJsonObject simJson;
            simJson.insert("latestLedger", 1);
            simJson.insert("transactionData", xdrToBase64(
                SorobanDataBuilder().setResources(1u, 1u, 1u).setResourceFee(step.resourceFee).build()));
            simJson.insert("minResourceFee", QString::number(step.minResourceFee));
            SorobanServer::applyTransactionSimulation(
                tx.data(), SimulateTransactionResponse::fromJson(simJson));
            QCOMPARE(tx->getFee(), step.fee);
            QCOMPARE(tx->getSorobanData()->resourceFee, step.resourceFee);
            QCOMPARE(qint64(tx->toV1Xdr().fee), step.fee);
        }
    }

    /**
     * The bid is the fee less the resource fee of the footprint the
     * transaction already carries. A decoded envelope whose fee is below that
     * resource fee has no bid to recover: it is refused, where js-stellar-sdk
     * (and this, before) took the whole fee for the bid. A fee equal to it
     * is a bid of zero, and stays zero.
     */
    void testApplySimulationRefusesAFeeBelowItsOwnResourceFee()
    {
        Account acc(sourceKeyPair(), 47);
        TransactionBuilder b(&acc, Network::testnetNetwork());
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        b.addOperation(
            InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(native));
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        b.setSorobanData(SorobanDataBuilder().setResources(1u, 1u, 1u).setResourceFee(1000).build());
        QScopedPointer<Transaction> built(b.build());
        QCOMPARE(built->getFee(), qint64(1100));

        QJsonObject simJson;
        simJson.insert("latestLedger", 1);
        simJson.insert("transactionData", xdrToBase64(
            SorobanDataBuilder().setResources(2u, 2u, 2u).setResourceFee(300).build()));
        simJson.insert("minResourceFee", QStringLiteral("300"));
        const SimulateTransactionResponse sim = SimulateTransactionResponse::fromJson(simJson);

        auto withFee = [&](quint32 fee) {
            stellar::TransactionEnvelope envelope = built->toEnvelopeXdr();
            envelope.v1.tx.fee = fee;
            return dynamic_cast<Transaction*>(
                AbstractTransaction::fromEnvelopeXdr(envelope, Network::testnetNetwork()));
        };

        QScopedPointer<Transaction> below(withFee(999));
        QVERIFY(below);
        bool threw = false;
        try { SorobanServer::applyTransactionSimulation(below.data(), sim); }
        catch (const std::exception&) { threw = true; }
        QVERIFY(threw);
        QCOMPARE(below->getFee(), qint64(999));                          // untouched
        QCOMPARE(below->getSorobanData()->resourceFee, qint64(1000));

        QScopedPointer<Transaction> zeroBid(withFee(1000));
        SorobanServer::applyTransactionSimulation(zeroBid.data(), sim);
        QCOMPARE(zeroBid->getFee(), qint64(300));

        SorobanServer::applyTransactionSimulation(built.data(), sim);
        QCOMPARE(built->getFee(), qint64(400));
    }

    /**
     * What a simulation hands back is decoded in full or refused, and the
     * transaction is left exactly as it was: a truncated auth entry used to
     * decode as a default-filled one and be installed and signed, and bytes
     * after the footprint were ignored. And with no minResourceFee at all,
     * the footprint's own resource fee is the one charged — it used to be
     * read as "0".
     */
    void testApplySimulationRefusesWhatItCannotDecodeWhole()
    {
        Account* acc = new Account(sourceKeyPair(), 45);
        TransactionBuilder b(acc, Network::testnetNetwork());
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        b.addOperation(
            InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(native));
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        QScopedPointer<Transaction> tx(b.build());

        const QString data = xdrToBase64(
            SorobanDataBuilder().setResources(1u, 1u, 1u).setResourceFee(777).build());

        QJsonObject truncatedAuth;
        truncatedAuth.insert("latestLedger", 1);
        truncatedAuth.insert("transactionData", data);
        truncatedAuth.insert("minResourceFee", QStringLiteral("777"));
        QJsonObject result;
        result.insert("xdr", QStringLiteral("AAAAAQ=="));
        result.insert("auth", QJsonArray{QStringLiteral("AAAAAA==")});
        truncatedAuth.insert("results", QJsonArray{result});

        QJsonObject trailing;
        trailing.insert("latestLedger", 1);
        trailing.insert("transactionData", QString::fromLatin1(
            (QByteArray::fromBase64(data.toLatin1()) + QByteArray("GARBAGE!")).toBase64()));
        trailing.insert("minResourceFee", QStringLiteral("777"));

        for (const QJsonObject& bad : {truncatedAuth, trailing}) {
            bool threw = false;
            try {
                SorobanServer::applyTransactionSimulation(
                    tx.data(), SimulateTransactionResponse::fromJson(bad));
            } catch (const std::exception&) {
                threw = true;
            }
            QVERIFY(threw);
            QCOMPARE(tx->getFee(), qint64(100));
            QVERIFY(tx->getSorobanData().isNull());
        }

        QJsonObject noMinFee;
        noMinFee.insert("latestLedger", 1);
        noMinFee.insert("transactionData", data);
        SorobanServer::applyTransactionSimulation(
            tx.data(), SimulateTransactionResponse::fromJson(noMinFee));
        QCOMPARE(tx->getFee(), qint64(100 + 777));

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
        result0.insert("xdr", QStringLiteral("AAAAAQ=="));
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

    /**
     * A transaction whose own soroban data carries a negative resource fee has
     * no bid to recover from it, and is refused before any arithmetic, through
     * prepareTransaction as well. The fee less that resource fee was taken for
     * the bid: fee 100 with -1000 was prepared at 1400 stroops for a
     * 300-stroop simulation (with -4000000000, at 4000000400), and INT64_MIN
     * overflowed.
     */
    void testApplySimulationRefusesANegativeResourceFeeOfItsOwn()
    {
        Account acc(sourceKeyPair(), 48);
        TransactionBuilder b(&acc, Network::testnetNetwork());
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        b.addOperation(
            InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(native));
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        b.setSorobanData(SorobanDataBuilder().setResources(1u, 1u, 1u).setResourceFee(1000).build());
        QScopedPointer<Transaction> built(b.build());

        QJsonObject simJson;
        simJson.insert("latestLedger", 1);
        simJson.insert("transactionData", xdrToBase64(
            SorobanDataBuilder().setResources(2u, 2u, 2u).setResourceFee(300).build()));
        simJson.insert("minResourceFee", QStringLiteral("300"));
        const SimulateTransactionResponse sim = SimulateTransactionResponse::fromJson(simJson);

        for (const qint64 negative : { qint64(-1000), std::numeric_limits<qint64>::min() }) {
            stellar::TransactionEnvelope envelope = built->toEnvelopeXdr();
            envelope.v1.tx.fee = 100;
            envelope.v1.tx.ext.sorobanData.resourceFee = negative;
            QScopedPointer<Transaction> tx(dynamic_cast<Transaction*>(
                AbstractTransaction::fromEnvelopeXdr(envelope, Network::testnetNetwork())));
            QVERIFY(tx);
            QCOMPARE(tx->getSorobanData()->resourceFee, negative);

            QString message;
            try { SorobanServer::applyTransactionSimulation(tx.data(), sim); }
            catch (const std::exception& e) { message = QString::fromUtf8(e.what()); }
            QVERIFY2(message.contains(QStringLiteral("negative resource fee")), qPrintable(message));
            QCOMPARE(tx->getFee(), qint64(100));                            // untouched
            QCOMPARE(tx->getSorobanData()->resourceFee, negative);
            QCOMPARE(tx->getSorobanData()->resources.instructions, quint32(1));

            PreparingSorobanServer server;
            server.sim = sim;
            message.clear();
            try { server.prepareTransaction(tx.data()); }
            catch (const std::exception& e) { message = QString::fromUtf8(e.what()); }
            QVERIFY2(message.contains(QStringLiteral("negative resource fee")), qPrintable(message));
            QCOMPARE(tx->getFee(), qint64(100));                            // untouched
            QCOMPARE(tx->getSorobanData()->resourceFee, negative);
            QCOMPARE(tx->getSorobanData()->resources.instructions, quint32(1));
        }
    }

    /**
     * transactionData and the auth entries are decoded as strict base64, as
     * AssembledTransaction::result() decodes: a character that is not base64
     * used to be skipped and the rest decoded, so a mangled answer was
     * installed, charged and signed as if it had arrived whole.
     */
    void testApplySimulationDecodesBase64Strictly()
    {
        Account acc(sourceKeyPair(), 49);
        TransactionBuilder b(&acc, Network::testnetNetwork());
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        b.addOperation(
            InvokeHostFunctionOperation::createContractFromAssetOperationBuilder(native));
        b.setBaseFee(100);
        b.setTimeout(TransactionBuilder::TIMEOUT_INFINITE);
        QScopedPointer<Transaction> tx(b.build());
        auto* op = dynamic_cast<InvokeHostFunctionOperation*>(tx->getOperations().first());
        QVERIFY(op);

        const QString data = xdrToBase64(
            SorobanDataBuilder().setResources(1u, 1u, 1u).setResourceFee(777).build());
        stellar::SorobanAuthorizationEntry authEntry;
        authEntry.credentials.type = stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_SOURCE_ACCOUNT;
        const QString auth = xdrToBase64(authEntry);
        auto mangled = [](const QString& base64) {
            return base64.left(4) + QStringLiteral("!") + base64.mid(4);
        };
        auto simulation = [](const QString& transactionData, const QString& authEntryBase64) {
            QJsonObject result;
            result.insert("xdr", QStringLiteral("AAAAAQ=="));
            result.insert("auth", QJsonArray{authEntryBase64});
            QJsonObject simJson;
            simJson.insert("latestLedger", 1);
            simJson.insert("transactionData", transactionData);
            simJson.insert("minResourceFee", QStringLiteral("777"));
            simJson.insert("results", QJsonArray{result});
            return simJson;
        };

        // Refused as the answer is read, before anything is applied.
        const QList<QJsonObject> refused = {
            simulation(mangled(data), auth), simulation(data, mangled(auth)),
            simulation(data + QStringLiteral(" "), auth),
        };
        for (const QJsonObject& sim : refused) {
            QString message;
            try {
                SorobanServer::applyTransactionSimulation(
                    tx.data(), SimulateTransactionResponse::fromJson(sim));
            }
            catch (const std::exception& e) { message = QString::fromUtf8(e.what()); }
            QVERIFY2(message.contains(QStringLiteral("is not base64")), qPrintable(message));
            QCOMPARE(tx->getFee(), qint64(100));                            // untouched
            QVERIFY(tx->getSorobanData().isNull());
            QVERIFY(op->getAuth().isEmpty());
        }

        SorobanServer::applyTransactionSimulation(
            tx.data(), SimulateTransactionResponse::fromJson(simulation(data, auth)));
        QCOMPARE(tx->getFee(), qint64(100 + 777));
        QCOMPARE(op->getAuth().size(), 1);
    }

    // ── Bounded requests ───────────────────────────────────────────────

    /**
     * Every request gives up after the server's request timeout. It used to
     * wait for its answer forever: an endpoint that took the connection and
     * never answered blocked the calling thread for good. (The endpoint here
     * hangs up after 10 s, so without a timeout this fails rather than hangs.)
     * The timeout is a positive number of milliseconds; 0 would be a guess.
     */
    void testARequestNobodyAnswersTimesOut()
    {
        SilentRpcEndpoint silent(10000);
        SorobanServer server(silent.url());
        QCOMPARE(server.getRequestTimeout(), STELLAR_QT_REPLY_TIMEOUT);
        server.setRequestTimeout(300);
        QCOMPARE(server.getRequestTimeout(), 300);

        QElapsedTimer clock;
        clock.start();
        QString message;
        try { server.getHealth(); }
        catch (const std::exception& e) { message = QString::fromUtf8(e.what()); }
        QVERIFY2(message.contains(QStringLiteral("no answer within 300 ms")), qPrintable(message));
        QVERIFY2(clock.elapsed() < 5000, qPrintable(QString::number(clock.elapsed())));

        for (const int bad : { 0, -1 }) {
            bool threw = false;
            try { server.setRequestTimeout(bad); }
            catch (const std::invalid_argument&) { threw = true; }
            QVERIFY(threw);
            QCOMPARE(server.getRequestTimeout(), 300);
        }
    }

    /**
     * cancelRequests() aborts a request still waiting for its answer. Queued,
     * it is delivered by the event loop the request waits in; nothing could
     * interrupt that wait before. It does not stick: the next request is sent
     * as usual (and here times out, as nobody answers).
     */
    void testCancelRequestsReleasesAStalledRequest()
    {
        SilentRpcEndpoint silent(10000);
        SorobanServer server(silent.url());

        QTimer::singleShot(200, &server, &SorobanServer::cancelRequests);
        QElapsedTimer clock;
        clock.start();
        QString message;
        try { server.getHealth(); }
        catch (const std::exception& e) { message = QString::fromUtf8(e.what()); }
        QVERIFY2(message.contains(QStringLiteral("cancelled")), qPrintable(message));
        QVERIFY2(clock.elapsed() < 5000, qPrintable(QString::number(clock.elapsed())));

        server.setRequestTimeout(300);
        message.clear();
        try { server.getHealth(); }
        catch (const std::exception& e) { message = QString::fromUtf8(e.what()); }
        QVERIFY2(message.contains(QStringLiteral("no answer within 300 ms")), qPrintable(message));
    }

    /**
     * A request that fails in transport throws SorobanTransportException, and
     * says whether it may have reached the server: not for a connection that
     * never opened, and for a timeout, a cancel or a connection lost after the
     * request was written it may have. All of these were plain runtime_errors,
     * so restoreFootprint could not tell a send that never left from one the
     * RPC may yet apply, and handed the sequence number back after both.
     */
    void testTransportFailuresSayWhetherTheRequestMayHaveReachedTheServer()
    {
        {
            SorobanServer server(closedPortUrl());
            const TransportOutcome o = sendAndCatch(server);
            QVERIFY2(o.typed, qPrintable(o.message));
            QVERIFY2(!o.mayHaveReachedServer, qPrintable(o.message));
            QVERIFY2(o.message.contains(QStringLiteral("the request was not sent")), qPrintable(o.message));
        }
        {
            SilentRpcEndpoint silent(10000);
            SorobanServer server(silent.url());
            server.setRequestTimeout(300);
            const TransportOutcome o = sendAndCatch(server);
            QVERIFY2(o.typed, qPrintable(o.message));
            QVERIFY(o.mayHaveReachedServer);
            QVERIFY2(o.message.contains(QStringLiteral("no answer within 300 ms")), qPrintable(o.message));
        }
        {
            SilentRpcEndpoint silent(10000);
            SorobanServer server(silent.url());
            QTimer::singleShot(200, &server, &SorobanServer::cancelRequests);
            const TransportOutcome o = sendAndCatch(server);
            QVERIFY2(o.typed, qPrintable(o.message));
            QVERIFY(o.mayHaveReachedServer);
            QVERIFY2(o.message.contains(QStringLiteral("cancelled")), qPrintable(o.message));
        }
        {
            HangingUpRpcEndpoint endpoint;
            SorobanServer server(endpoint.url());
            server.setRequestTimeout(5000);
            const TransportOutcome o = sendAndCatch(server);
            QVERIFY2(o.typed, qPrintable(o.message));
            QVERIFY2(o.mayHaveReachedServer, qPrintable(o.message));
            QVERIFY2(o.message.contains(QStringLiteral("may have received it")), qPrintable(o.message));
            QVERIFY(endpoint.received() > 0);           // it did get there
        }
    }

    /**
     * A send lost on a kept-alive connection may have reached the server,
     * whatever error ends it. Qt 5.15 sends the request again on a new
     * connection when a kept-alive one drops, and when that one is refused
     * the reply ends with ConnectionRefusedError — a connection that never
     * opened — although the server had already read the whole
     * sendTransaction on the first. Only the body having been reported sent
     * keeps it "may have reached the server"; without that it read as never
     * sent, and restoreFootprint handed back the number of a restore the RPC
     * may still apply.
     *
     * A run in which Qt closed the kept-alive connection itself before the
     * send (see KeepAliveThenDropRpcEndpoint) drops no kept-alive connection,
     * so it proves nothing here and is run again, up to 10 times; the send
     * must still read as one that may have reached the server. It used to be
     * judged on that run, and failed about once in 35 runs under load.
     */
    void testASendLostOnAKeptAliveConnectionMayHaveReachedTheServer()
    {
        const int runs = 10;
        for (int run = 1; run <= runs; ++run) {
            KeepAliveThenDropRpcEndpoint endpoint;
            SorobanServer server(endpoint.url());
            server.setRequestTimeout(15000);
            server.sendRequest(QStringLiteral("getHealth"));    // answered; the connection stays open
            const TransportOutcome o = sendAndCatch(server);
            // Over whichever connection, the server read the whole send.
            QCOMPARE(endpoint.requests(), 2);
            QVERIFY(endpoint.lastRequest().contains("\"method\":\"sendTransaction\""));
            QVERIFY2(o.typed, qPrintable(o.message));
            QVERIFY2(o.mayHaveReachedServer, qPrintable(o.message));
            QVERIFY2(o.message.contains(QStringLiteral("may")), qPrintable(o.message));
            if (endpoint.carriers().at(1) != endpoint.carriers().at(0)) {
                // Sent over a new connection: no kept-alive drop to judge.
                qInfo("run %d: Qt closed the kept-alive connection before the send; running again", run);
                continue;
            }
            // Both requests over the one kept-alive connection.
            QCOMPARE(endpoint.connections(), 1);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            // What the guard is for: Qt 5.15 sent it again, and that connection was refused.
            QVERIFY2(o.message.contains(QStringLiteral("Connection refused")), qPrintable(o.message));
#endif
            return;
        }
        QFAIL(qPrintable(QStringLiteral("in %1 runs the send never travelled over the kept-alive "
                                        "connection: Qt closed it every time").arg(runs)));
    }

    /**
     * The RPC read the sendTransaction whole and its connection closed with
     * no answer, so it may have taken the transaction in; Qt 5 sends the
     * request again, by itself, on a new connection, and the RPC answers that
     * later send. ERROR (here txBAD_SEQ, what the RPC answers a transaction
     * sent again once the first has been applied), TRY_AGAIN_LATER or an
     * error envelope answers the later send only: it throws
     * SorobanTransportException with resent() and mayHaveReachedServer()
     * true, naming what the RPC said. It came back as the answer to the one
     * send, and an ERROR or TRY_AGAIN_LATER read as "never taken in", so the
     * sequence number was handed back or the payment built again while the
     * first could land. PENDING and DUPLICATE say the RPC has it and are
     * returned as ever, and so is any other method's answer to a later send
     * (getHealth). A later send lost as well throws what it met, resent. Qt 6
     * does not send it again: the request fails as a connection lost after it
     * was written, which may have reached the server, and is not resent.
     */
    void testAnAnswerToASendQtSentAgainIsNotFinal_data()
    {
        QTest::addColumn<QString>("answer");
        QTest::newRow("ERROR") << QStringLiteral("ERROR");
        QTest::newRow("TRY_AGAIN_LATER") << QStringLiteral("TRY_AGAIN_LATER");
        QTest::newRow("error envelope") << QStringLiteral("rpcError");
        QTest::newRow("PENDING") << QStringLiteral("PENDING");
        QTest::newRow("DUPLICATE") << QStringLiteral("DUPLICATE");
        QTest::newRow("lost again") << QStringLiteral("lost");
        QTest::newRow("getHealth") << QStringLiteral("getHealth");
    }
    void testAnAnswerToASendQtSentAgainIsNotFinal()
    {
        QFETCH(QString, answer);
        const bool health = answer == QLatin1String("getHealth");
        const QString hash = QStringLiteral("1c2eb969d67a1846070bbbb799fcfc2c58c5272f9dc597f67c0e469726dfea96");
        const QString badSeq = QStringLiteral("AAAAAAAAAGT////7AAAAAA==");      // txBAD_SEQ
        QJsonObject result;
        if (health) {
            result.insert("status", QStringLiteral("healthy"));
            result.insert("latestLedger", 4917043);
            result.insert("oldestLedger", 4796084);
            result.insert("ledgerRetentionWindow", 120960);
        } else {
            result.insert("status", answer);
            result.insert("hash", hash);
            result.insert("latestLedger", 4919962);
            result.insert("latestLedgerCloseTime", QStringLiteral("1790623397"));
            if (answer == QLatin1String("ERROR"))
                result.insert("errorResultXdr", badSeq);
        }
        QJsonObject envelope;
        envelope.insert("jsonrpc", QStringLiteral("2.0"));
        envelope.insert("id", QStringLiteral("1"));
        if (answer == QLatin1String("rpcError")) {
            QJsonObject error;
            error.insert("code", -32603);
            error.insert("message", QStringLiteral("could not submit transaction to stellar-core"));
            envelope.insert("error", error);
        } else {
            envelope.insert("result", result);
        }
        DropsThenAnswersRpcEndpoint endpoint(answer == QLatin1String("lost") ? 100 : 1,
                                             QJsonDocument(envelope).toJson(QJsonDocument::Compact));
        SorobanServer server(endpoint.url());
        server.setRequestTimeout(15000);

        TransportOutcome o;
        bool resent = false;
        SendTransactionResponse response;
        try {
            if (health)
                server.getHealth();
            else
                response = server.sendTransactionXdr(QStringLiteral("AAAAAgAAAAA="));
            o.message = QStringLiteral("no exception");
        } catch (const qstellar::exception::SorobanTransportException& e) {
            o.typed = true;
            o.mayHaveReachedServer = e.mayHaveReachedServer();
            o.message = QString::fromUtf8(e.what());
            resent = e.resent();
        } catch (const std::exception& e) {
            o.message = QString::fromUtf8(e.what());
        }

        // The one request, read whole each time it was sent.
        const QList<QByteArray> bodies = endpoint.bodies();
        QVERIFY(!bodies.isEmpty());
        QVERIFY(bodies.first().contains(health ? QByteArray("\"method\":\"getHealth\"")
                                               : QByteArray("\"method\":\"sendTransaction\"")));
        for (const QByteArray& body : bodies)
            QCOMPARE(body, bodies.first());
        const int sends = int(bodies.size());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QVERIFY2(sends >= 2, qPrintable(o.message));    // Qt 5 sends it again
#endif
        if (sends == 1) {
            // Not sent again: the connection closed on the only send.
            qInfo("sent once, not again: %s", qPrintable(o.message));
            QVERIFY2(o.typed && o.mayHaveReachedServer, qPrintable(o.message));
            QVERIFY2(!resent, qPrintable(o.message));
            return;
        }
        if (answer == QLatin1String("lost")) {
            // Every send lost: what the last one met, said as resent.
            QVERIFY2(o.typed && o.mayHaveReachedServer && resent, qPrintable(o.message));
            QVERIFY2(o.message.contains(QStringLiteral("Qt had sent it again")), qPrintable(o.message));
            return;
        }
        QCOMPARE(sends, 2);
        if (health || answer == QLatin1String("PENDING") || answer == QLatin1String("DUPLICATE")) {
            // The RPC has it (or it is no send): the answer is taken.
            QVERIFY2(o.message == QLatin1String("no exception"), qPrintable(o.message));
            if (!health) {
                QCOMPARE(response.getStatus(), answer == QLatin1String("PENDING")
                                                   ? SendTransactionResponse::Status::PENDING
                                                   : SendTransactionResponse::Status::DUPLICATE);
                QCOMPARE(response.getHash(), hash);
            }
            return;
        }
        QVERIFY2(o.typed, qPrintable(o.message));
        QVERIFY2(o.mayHaveReachedServer && resent, qPrintable(o.message));
        const QString said = answer == QLatin1String("ERROR")
            ? QStringLiteral("with ERROR (hash %1, errorResultXdr %2)").arg(hash, badSeq)
            : answer == QLatin1String("TRY_AGAIN_LATER")
                ? QStringLiteral("with TRY_AGAIN_LATER (hash %1)").arg(hash)
                : QStringLiteral("could not submit transaction to stellar-core");
        QVERIFY2(o.message.contains(said), qPrintable(o.message));
        QVERIFY2(o.message.contains(QStringLiteral("the outcome is not known")), qPrintable(o.message));
    }

    /**
     * However a send that Qt sent again ends — a 500, a status line cut
     * short, no answer until the request timeout — it says resent() and that
     * it may have reached the server, small or big (a 200 KB body is written
     * in pieces). Before, a timed-out resend lost resent(), and signAndSend
     * then reported a plain transport failure instead of an unknown outcome.
     * A single send answered ERROR is not taken for a resend.
     */
    void testEveryEndOfASendQtSentAgainSaysResent_data()
    {
        QTest::addColumn<QString>("mode");
        QTest::addColumn<bool>("big");
        for (const char* m : { "500", "partial", "hold", "once-error" }) {
            QTest::newRow(m) << QString::fromLatin1(m) << false;
            QTest::newRow((QByteArray(m) + " big").constData()) << QString::fromLatin1(m) << true;
        }
    }
    void testEveryEndOfASendQtSentAgainSaysResent()
    {
        QFETCH(QString, mode);
        QFETCH(bool, big);
        ResendEndingEndpoint endpoint(mode);
        SorobanServer server(endpoint.url());
        server.setRequestTimeout(mode == QLatin1String("hold") ? 3000 : 15000);
        QString env = QStringLiteral("AAAAAgAAAAA=");
        if (big)
            env = QString(200000, QLatin1Char('A'));
        TransportOutcome o;
        bool resent = false;
        try {
            const SendTransactionResponse response = server.sendTransactionXdr(env);
            o.message = QStringLiteral("no exception, status %1").arg(int(response.getStatus()));
        } catch (const qstellar::exception::SorobanTransportException& e) {
            o.typed = true;
            o.mayHaveReachedServer = e.mayHaveReachedServer();
            o.message = QString::fromUtf8(e.what());
            resent = e.resent();
        } catch (const std::exception& e) {
            o.message = QString::fromUtf8(e.what());
        }
        if (mode == QLatin1String("once-error")) {
            QCOMPARE(endpoint.count, 1);
            QVERIFY2(!resent, qPrintable(o.message));
            QVERIFY2(o.message.startsWith(QLatin1String("no exception")), qPrintable(o.message));
            return;
        }
        if (endpoint.count >= 2)
            QVERIFY2(o.typed && o.mayHaveReachedServer && resent, qPrintable(o.message));
        else
            QVERIFY2(o.typed && o.mayHaveReachedServer, qPrintable(o.message));
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

    /**
     * cancelRequests() ends a pollTransaction between two polls. It only
     * aborted a request on the wire, so the wait between polls ran to its end
     * and the poll went on until its timeout — here 3 s waits for 6 s. A
     * cancel that lands while a poll's request is being answered ends it too,
     * with no further wait or poll, and a poll started afterwards runs as usual.
     */
    void testCancelRequestsEndsAPollBetweenPolls()
    {
        ScriptedSorobanServer srv;
        srv.script = { GetTransactionResponse::Status::NOT_FOUND };
        auto poll = [&srv]() {
            TransportOutcome out;
            try {
                srv.pollTransaction(QStringLiteral("hash"), 6000, 3000);
                out.message = QStringLiteral("no exception");
            } catch (const qstellar::exception::SorobanTransportException& e) {
                out.typed = true;
                out.mayHaveReachedServer = e.mayHaveReachedServer();
                out.message = QString::fromUtf8(e.what());
            } catch (const std::exception& e) {
                out.message = QString::fromUtf8(e.what());
            }
            return out;
        };

        QTimer::singleShot(200, &srv, &SorobanServer::cancelRequests);
        QElapsedTimer clock;
        clock.start();
        TransportOutcome o = poll();
        QVERIFY2(o.typed && o.message.contains(QStringLiteral("cancelled")), qPrintable(o.message));
        QVERIFY(o.mayHaveReachedServer);
        QVERIFY2(clock.elapsed() < 2500, qPrintable(QString::number(clock.elapsed())));
        QCOMPARE(srv.idx, 1);

        srv.idx = 0;
        srv.cancelInFetch = true;
        clock.restart();
        o = poll();
        QVERIFY2(o.typed && o.message.contains(QStringLiteral("cancelled")), qPrintable(o.message));
        QVERIFY2(clock.elapsed() < 2500, qPrintable(QString::number(clock.elapsed())));
        QCOMPARE(srv.idx, 1);

        srv.idx = 0;
        srv.script = { GetTransactionResponse::Status::NOT_FOUND,
                       GetTransactionResponse::Status::SUCCESS };
        const GetTransactionResponse r = srv.pollTransaction(QStringLiteral("hash"), 5000, 1);
        QCOMPARE(r.getStatus(), GetTransactionResponse::Status::SUCCESS);
        QCOMPARE(srv.idx, 2);
    }

    /**
     * getTransaction takes only an answer about the transaction it asked for:
     * the RPC echoes the hash asked for, found or not (in the case it was
     * asked in). An answer about another transaction used to be taken for this
     * one's, and pollTransaction returned its outcome.
     */
    void testGetTransactionRefusesAnAnswerAboutAnotherTransaction()
    {
        const QString asked = QStringLiteral("00000000000000000000000000000000000000000000000000000000000000ab");
        // A real testnet answer for `asked`, which it does not have.
        const QByteArray notFound =
            "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"latestLedger\":4919935,\"latestLedgerCloseTime\":\"1790623262\","
            "\"oldestLedger\":4798976,\"oldestLedgerCloseTime\":\"1790018467\",\"status\":\"NOT_FOUND\","
            "\"txHash\":\"00000000000000000000000000000000000000000000000000000000000000ab\",\"applicationOrder\":0,"
            "\"feeBump\":false,\"events\":{},\"ledger\":0,\"createdAt\":\"0\"}}";
        AnsweringSorobanServer server;
        server.answer = notFound;
        QCOMPARE(server.getTransaction(asked).getStatus(), GetTransactionResponse::Status::NOT_FOUND);
        QCOMPARE(server.getTransaction(asked.toUpper()).getStatus(), GetTransactionResponse::Status::NOT_FOUND);

        const QString other = QStringLiteral("00000000000000000000000000000000000000000000000000000000000000ac");
        for (const bool polling : { false, true }) {
            QString message;
            try {
                if (polling)
                    server.pollTransaction(other, 60000, 10);
                else
                    server.getTransaction(other);
            } catch (const std::runtime_error& e) {
                message = QString::fromUtf8(e.what());
            }
            QVERIFY2(message.contains(other) && message.contains(asked), qPrintable(message));
        }
    }

    /**
     * simulateTransaction sends what SimulateTransactionOptions asks for as
     * js-stellar-sdk 17.1.0 sends it: simulateTransaction(tx, {cpuInstructions:
     * 1000000}, "record_allow_nonroot") posts the second params below (js adds
     * useUpgradedAuth true unless told otherwise), and (tx, undefined,
     * "enforce", false) the third. With no options it sends the transaction
     * alone, as before and as py 14.1.0 does (py adds a null resourceConfig and
     * authMode). There was no way to send any of them.
     */
    void testSimulateTransactionSendsTheOptionsAsJsDoes()
    {
        AnsweringSorobanServer server;
        server.answer = "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"latestLedger\":4919962}}";
        const QString envelope = QStringLiteral("AAAAAgAAAAA=");
        auto sent = [](const char* json) { return QJsonDocument::fromJson(QByteArray(json)).object(); };

        server.simulateTransactionXdr(envelope);
        QCOMPARE(server.lastParams.toObject(), sent("{\"transaction\":\"AAAAAgAAAAA=\"}"));

        SimulateTransactionOptions options;
        options.instructionLeeway = 1000000;
        options.authMode = SimulateTransactionOptions::AuthMode::RecordAllowNonroot;
        options.useUpgradedAuth = SimulateTransactionOptions::UpgradedAuth::Requested;
        server.simulateTransactionXdr(envelope, options);
        QCOMPARE(server.lastParams.toObject(), sent(
            "{\"transaction\":\"AAAAAgAAAAA=\",\"authMode\":\"record_allow_nonroot\","
            "\"useUpgradedAuth\":true,\"resourceConfig\":{\"instructionLeeway\":1000000}}"));

        options = SimulateTransactionOptions();
        options.authMode = SimulateTransactionOptions::AuthMode::Enforce;
        options.useUpgradedAuth = SimulateTransactionOptions::UpgradedAuth::NotRequested;
        server.simulateTransactionXdr(envelope, options);
        QCOMPARE(server.lastParams.toObject(), sent(
            "{\"transaction\":\"AAAAAgAAAAA=\",\"authMode\":\"enforce\",\"useUpgradedAuth\":false}"));

        options = SimulateTransactionOptions();
        options.authMode = SimulateTransactionOptions::AuthMode::Record;
        server.simulateTransactionXdr(envelope, options);
        QCOMPARE(server.lastParams.toObject(),
                 sent("{\"transaction\":\"AAAAAgAAAAA=\",\"authMode\":\"record\"}"));
        QCOMPARE(server.methods.size(), 4);
        QCOMPARE(server.methods.count(QStringLiteral("simulateTransaction")), server.methods.size());
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

    /**
     * The SAC id is the hash of the asset's XDR, and a pool share is not an
     * Asset: its partial encoding was hashed into the id of a contract that
     * cannot exist, and getSACBalance read the balance there as zero.
     */
    void testBuildSACBalanceLedgerKeyRefusesAPoolShare()
    {
        stellar::Asset poolShare;
        poolShare.type = stellar::AssetType::ASSET_TYPE_POOL_SHARE;
        QString message;
        try {
            SorobanServer::buildSACBalanceLedgerKey(
                QStringLiteral("GCACCFMIWJAHUUASSE2WC7V6VVDLYRLSJYZ3DJEXCG523FSHTNII6KOG"),
                poolShare, Network::testnetNetwork()->getNetworkPassphrase());
        } catch (const std::runtime_error& e) {
            message = QString::fromUtf8(e.what());
        }
        QVERIFY2(message.contains(QStringLiteral("cannot be encoded")), qPrintable(message));
    }

    /**
     * getLedgerEntries answers with a LedgerEntryData — the entry type, then
     * the ContractDataEntry {ext, contract, key, durability, val} — and the
     * entries js-stellar-sdk and py-stellar-sdk build are read exactly, the
     * i128 amount included whatever its size. This read a layout of its own,
     * a LedgerEntry prefix and no ext: it refused every real entry as not
     * contract data, and this test passed only because it built that same
     * layout by hand — which is now refused (the old test's buffer, below).
     */
    void testParseSACBalanceRoundTrip()
    {
        const SACBalance js = SorobanServer::parseSACBalanceFromLedgerEntryXdr(jsUsdcBalanceEntry());
        QVERIFY(js.exists);
        QCOMPARE(js.amount, i128("7fffffffffffffffffffffffffffffff"));
        QCOMPARE(Scv::decimalText(Scv::toInt128(js.amount)),
                 QStringLiteral("170141183460469231731687303715884105727"));
        QCOMPARE(js.authorized, false);
        QCOMPARE(js.clawbackEnabled, true);

        const SACBalance py = SorobanServer::parseSACBalanceFromLedgerEntryXdr(pyNativeBalanceEntry());
        QVERIFY(py.exists);
        QCOMPARE(py.amount, i128("00000000000000010000000000000005"));
        QCOMPARE(py.authorized, true);
        QCOMPARE(py.clawbackEnabled, true);

        // The same bytes as js-stellar-sdk's, from the parts: the layout is
        // stellar-xdr's, not a coincidence of these two fixtures.
        const stellar::SCVal usdcValue = rawMap({
            { QStringLiteral("amount"), Scv::toInt128Parts(qint64(0x7FFFFFFFFFFFFFFFLL), quint64(0xFFFFFFFFFFFFFFFFULL)) },
            { QStringLiteral("authorized"), Scv::toBoolean(false) },
            { QStringLiteral("clawback"), Scv::toBoolean(true) } });
        QCOMPARE(contractDataEntry(usdcSac(), balanceKey(sacHolder()), 1, usdcValue),
                 jsUsdcBalanceEntry());

        // The old test's hand-made layout: a LedgerEntry prefix, no ext.
        QByteArray old;
        {
            QDataStream s(&old, QIODevice::WriteOnly);
            s << quint32(12345) << static_cast<qint32>(stellar::LedgerEntryType::CONTRACT_DATA)
              << Scv::toAddress(usdcSac()).address << balanceKey(sacHolder())
              << static_cast<qint32>(stellar::ContractDataDurability::PERSISTENT)
              << Scv::toMap({ Scv::toSymbol(QStringLiteral("amount")), Scv::toSymbol(QStringLiteral("authorized")),
                              Scv::toSymbol(QStringLiteral("clawback")) },
                            { Scv::toInt128Parts(qint64(0), quint64(987654321ULL)), Scv::toBoolean(true),
                              Scv::toBoolean(false) })
              << qint32(0);
        }
        bool threw = false;
        try { SorobanServer::parseSACBalanceFromLedgerEntryXdr(QString::fromLatin1(old.toBase64())); }
        catch (const std::runtime_error&) { threw = true; }
        QVERIFY(threw);
    }

    /**
     * Anything that is not exactly a balance entry is refused, where it used
     * to be read: base64 with a character that is not base64 (skipped) or
     * bytes left over (ignored), an entry cut short, a TEMPORARY entry or one
     * under another key, and a value that is not the balance map — a flag
     * missing or not a bool (taken as its default), an extra or misordered
     * entry, an amount that is not an i128 or is negative.
     */
    void testParseSACBalanceRefusesWhatIsNotABalance()
    {
        const QString whole = jsUsdcBalanceEntry();
        const QByteArray raw = QByteArray::fromBase64(whole.toLatin1());
        const stellar::SCVal key = balanceKey(sacHolder());
        const stellar::SCVal amount = Scv::toInt128(qint64(5));
        const stellar::SCVal yes = Scv::toBoolean(true);
        const QString a = QStringLiteral("amount"), au = QStringLiteral("authorized"),
                      c = QStringLiteral("clawback");

        const QList<QPair<const char*, QString>> refused = {
            { "not base64", whole.left(8) + QStringLiteral("!") + whole.mid(8) },
            { "trailing space", whole + QStringLiteral(" ") },
            { "bytes left over", QString::fromLatin1((raw + QByteArray(4, '\0')).toBase64()) },
            { "cut short", QString::fromLatin1(raw.left(raw.size() - 4).toBase64()) },
            { "empty", QString() },
            { "ext not 0", QString::fromLatin1((raw.left(4) + QByteArray::fromHex("00000001") + raw.mid(8)).toBase64()) },
            { "not contract data", QString::fromLatin1((QByteArray::fromHex("00000009") + raw.mid(4)).toBase64()) },
            { "temporary", contractDataEntry(usdcSac(), key, 0, rawMap({ { a, amount }, { au, yes }, { c, yes } })) },
            { "durability 2", contractDataEntry(usdcSac(), key, 2, rawMap({ { a, amount }, { au, yes }, { c, yes } })) },
            { "another key", contractDataEntry(usdcSac(), Scv::toVec({ Scv::toSymbol(QStringLiteral("Allowance")), Scv::toAddress(sacHolder()) }),
                                               1, rawMap({ { a, amount }, { au, yes }, { c, yes } })) },
            { "no clawback", contractDataEntry(usdcSac(), key, 1, rawMap({ { a, amount }, { au, yes } })) },
            { "authorized not a bool", contractDataEntry(usdcSac(), key, 1, rawMap({ { a, amount }, { au, Scv::toUint32(quint32(1)) }, { c, yes } })) },
            { "an extra entry", contractDataEntry(usdcSac(), key, 1, rawMap({ { a, amount }, { au, yes }, { c, yes }, { QStringLiteral("extra"), yes } })) },
            { "misordered", contractDataEntry(usdcSac(), key, 1, rawMap({ { au, yes }, { a, amount }, { c, yes } })) },
            { "amount an i64", contractDataEntry(usdcSac(), key, 1, rawMap({ { a, Scv::toInt64(qint64(5)) }, { au, yes }, { c, yes } })) },
            { "negative amount", contractDataEntry(usdcSac(), key, 1, rawMap({ { a, Scv::toInt128(qint64(-1)) }, { au, yes }, { c, yes } })) },
            { "not a map", contractDataEntry(usdcSac(), key, 1, amount) },
        };
        for (const auto& bad : refused) {
            QString message;
            try {
                const SACBalance b = SorobanServer::parseSACBalanceFromLedgerEntryXdr(bad.second);
                message = QStringLiteral("read as a balance of ") + QString::fromLatin1(b.amount.toHex());
            } catch (const std::runtime_error& e) {
                message = QString::fromUtf8(e.what());
            }
            QVERIFY2(message.startsWith(QStringLiteral("parseSACBalanceFromLedgerEntryXdr: ")),
                     qPrintable(QString::fromLatin1(bad.first) + QStringLiteral(": ") + message));
        }

        // What is left once the refusals are out: the same parts, whole.
        const SACBalance fine = SorobanServer::parseSACBalanceFromLedgerEntryXdr(
            contractDataEntry(usdcSac(), key, 1, rawMap({ { a, amount }, { au, yes }, { c, Scv::toBoolean(false) } })));
        QCOMPARE(fine.amount, i128("00000000000000000000000000000005"));
        QVERIFY(fine.authorized);
        QVERIFY(!fine.clawbackEnabled);
    }

    /**
     * getSACBalance reads a real testnet answer (saved as it came) exactly,
     * and asks for the key js-stellar-sdk builds. It threw on every real
     * balance: the entry was read with a layout of its own.
     */
    void testGetSACBalanceReadsARealTestnetAnswer()
    {
        AnsweringSorobanServer server;
        server.answer = realSacBalanceAnswer();
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;

        const SACBalance b = server.getSACBalance(sacHolder(), native, Network::testnetNetwork());
        QCOMPARE(server.methods, QStringList{ QStringLiteral("getLedgerEntries") });
        QCOMPARE(server.lastParams.toObject().value("keys").toArray(),
                 QJsonArray{ nativeBalanceKey() });
        QVERIFY(b.exists);
        QCOMPARE(b.amount, i128("0000000000000000000000acbc389640"));
        QCOMPARE(Scv::decimalText(Scv::toInt128(b.amount)), QStringLiteral("741892200000"));
        QVERIFY(b.authorized);
        QVERIFY(!b.clawbackEnabled);

        // The same key answered with py-stellar-sdk's entry: 2^64 + 5, exactly.
        server.answer = ledgerEntriesAnswer({ { nativeBalanceKey(), pyNativeBalanceEntry() } });
        const SACBalance big = server.getSACBalance(sacHolder(), native, Network::testnetNetwork());
        QCOMPARE(big.amount, i128("00000000000000010000000000000005"));
        QVERIFY(big.clawbackEnabled);

        // No entry: the contract never held it.
        server.answer = ledgerEntriesAnswer({});
        const SACBalance none = server.getSACBalance(sacHolder(), native, Network::testnetNetwork());
        QVERIFY(!none.exists);
        QCOMPARE(none.amount, QByteArray(16, '\0'));
    }

    /**
     * getSACBalance refuses an account, whose balance of an asset contract is
     * its trustline (or, for XLM, its account) and never a contract data
     * entry: it looked one up all the same and read every account's balance
     * as zero. And it takes only the entry it asked for: the first entry of
     * the answer was read whatever its key, and however many came.
     */
    void testGetSACBalanceRefusesWhatIsNotTheBalanceAskedFor()
    {
        stellar::Asset native;
        native.type = stellar::AssetType::ASSET_TYPE_NATIVE;
        const QString usdcKey = QStringLiteral("AAAABgAAAAENXOi2+C6QUF0YDivdWZ1Pe2wbNghDzazFPRNf5p+ZZwAAABAAAAABAAAAAgAAAA8AAAAHQmFsYW5jZQAAAAASAAAAAcDOO5r86PBOab2xhhb9o5b5wu5djOD4YD+Rsg5ci+hoAAAAAQ==");

        AnsweringSorobanServer account;
        account.answer = ledgerEntriesAnswer({});
        QString message;
        try { account.getSACBalance(usdcIssuer(), native, Network::testnetNetwork()); }
        catch (const std::runtime_error& e) { message = QString::fromUtf8(e.what()); }
        QVERIFY2(message.contains(QStringLiteral("trustline")), qPrintable(message));
        QVERIFY(account.methods.isEmpty());                     // nothing asked

        const QList<QPair<const char*, QByteArray>> refused = {
            { "two entries", ledgerEntriesAnswer({ { nativeBalanceKey(), pyNativeBalanceEntry() },
                                                   { nativeBalanceKey(), pyNativeBalanceEntry() } }) },
            { "another key", ledgerEntriesAnswer({ { usdcKey, jsUsdcBalanceEntry() } }) },
            { "another entry under this key", ledgerEntriesAnswer({ { nativeBalanceKey(), jsUsdcBalanceEntry() } }) },
            { "no key", ledgerEntriesAnswer({ { QString(), pyNativeBalanceEntry() } }) },
        };
        for (const auto& bad : refused) {
            AnsweringSorobanServer server;
            server.answer = bad.second;
            message.clear();
            try {
                const SACBalance b = server.getSACBalance(sacHolder(), native, Network::testnetNetwork());
                message = QStringLiteral("read as a balance of ") + QString::fromLatin1(b.amount.toHex());
            } catch (const std::runtime_error& e) {
                message = QString::fromUtf8(e.what());
            }
            // An answer whose key is not base64 XDR is refused as it is read.
            const bool unreadable = QByteArray(bad.first) == "no key";
            QVERIFY2(message.startsWith(unreadable ? QStringLiteral("Soroban RPC: getLedgerEntries answered entries[0].key, ")
                                                   : QStringLiteral("getSACBalance: ")),
                     qPrintable(QString::fromLatin1(bad.first) + QStringLiteral(": ") + message));
        }

        // The key the USDC entry was built for is the one this builds.
        QScopedPointer<Asset> usdc(Asset::createNonNativeAsset(QStringLiteral("USDC"), usdcIssuer()));
        QCOMPARE(SorobanServer::buildSACBalanceLedgerKey(sacHolder(), usdc->toXdr(),
                     Network::testnetNetwork()->getNetworkPassphrase()), usdcKey);
    }

    // ── getAssetBalance ───────────────────────────────────────────────

    /**
     * An account's balance of an asset, over RPC: its trustline, or for XLM
     * its account entry, asked for under the key js-stellar-sdk builds and
     * read from real testnet answers exactly as js-stellar-sdk 17.1.0's
     * getAssetBalance reads them (amount and flags) and Horizon reports them
     * (liabilities, and XLM, which js cannot read). There was no way to read
     * one: getSACBalance refuses an account.
     */
    void testGetAssetBalanceReadsRealTestnetAnswers()
    {
        struct Case {
            const char* answer;
            QString holder;
            stellar::Asset asset;
            const char* amount;
            bool authorized, maintain, clawback;
            qint64 buying, selling;
        };
        const QList<Case> cases = {
            { "aUsdc", holderA(), issued(QStringLiteral("USDC"), circleUsdcIssuer()), "11340000",
              true, false, false, 0, 0 },
            { "aTestn", holderA(), issued(QStringLiteral("TESTN"), testIssuer()), "12145678901",
              true, false, true, 0, 1000000003 },
            { "aTn4", holderA(), issued(QStringLiteral("TN4"), testIssuer()), "9223372036854775807",
              true, false, true, 0, 0 },
            { "bTestn", holderB(), issued(QStringLiteral("TESTN"), testIssuer()), "420000001",
              false, true, true, 0, 0 },
            // Horizon: 9969.4999300 XLM, buying liabilities 700.0000021.
            { "aXlm", holderA(), nativeAsset(), "99694999300", true, false, false, 7000000021LL, 0 },
            { "bXlm", holderB(), nativeAsset(), "99999999900", true, false, false, 0, 0 },
        };
        for (const Case& c : cases) {
            AnsweringSorobanServer server;
            server.answer = realAssetAnswer(c.answer);
            const AssetBalance b = server.getAssetBalance(c.holder, c.asset, nullptr);
            QVERIFY2(b.exists, c.answer);
            QCOMPARE(server.methods, QStringList{ QStringLiteral("getLedgerEntries") });
            QCOMPARE(server.lastParams.toObject().value("keys").toArray(),
                     QJsonArray{ answeredField(server.answer, "key") });
            QCOMPARE(Scv::decimalText(Scv::toInt128(b.amount)), QString::fromLatin1(c.amount));
            QCOMPARE(b.amount.size(), 16);
            QCOMPARE(b.authorized, c.authorized);
            QCOMPARE(b.authorizedToMaintainLiabilities, c.maintain);
            QCOMPARE(b.clawbackEnabled, c.clawback);
            QCOMPARE(b.buyingLiabilities, c.buying);
            QCOMPARE(b.sellingLiabilities, c.selling);
            QCOMPARE(b.latestLedger, quint32(4917043));
        }

        // No entry is no balance, not an error: the issuer holds no line to
        // its own asset (js-stellar-sdk throws "not found").
        AnsweringSorobanServer none;
        none.answer = realAssetAnswer("issuerTestn");
        const AssetBalance n = none.getAssetBalance(testIssuer(),
                                                    issued(QStringLiteral("TESTN"), testIssuer()), nullptr);
        QCOMPARE(none.lastParams.toObject().value("keys").toArray(), QJsonArray{ issuerTestnKey() });
        QVERIFY(!n.exists);
        QCOMPARE(n.amount, QByteArray(16, '\0'));
        QVERIFY(!n.authorized && !n.authorizedToMaintainLiabilities && !n.clawbackEnabled);
        QCOMPARE(n.buyingLiabilities, qint64(0));
        QCOMPARE(n.latestLedger, quint32(4917043));
    }

    /**
     * Entries js-stellar-sdk and py-stellar-sdk build are read exactly, at
     * INT64_MAX and with every extension: a trustline with ext v1 liabilities
     * and ext v2, one with v1 and no v2, an account with the whole chain
     * (signers, a home domain, sponsorships, v3) and one with none.
     */
    void testGetAssetBalanceReadsJsAndPyEntries()
    {
        const qint64 max = std::numeric_limits<qint64>::max();
        struct Case {
            const char* name;
            QString key, xdr;
            stellar::Asset asset;
            bool authorized, maintain, clawback;
            qint64 amount, buying, selling;
        };
        const QString testnKey = answeredField(realAssetAnswer("aTestn"), "key");
        const QString usdcKey = answeredField(realAssetAnswer("aUsdc"), "key");
        const QString accountKey = answeredField(realAssetAnswer("aXlm"), "key");
        const QList<Case> cases = {
            { "js TESTN line", testnKey, jsTestnLineEntry(), issued(QStringLiteral("TESTN"), testIssuer()),
              false, true, true, max, 123, max },
            { "py USDC line", usdcKey, pyUsdcLineEntry(), issued(QStringLiteral("USDC"), circleUsdcIssuer()),
              true, false, false, 11340000, max - 11340000, 11340000 },
            { "py account", accountKey, pyAccountFullEntry(), nativeAsset(),
              true, false, false, max, 7000000021LL, max },
            { "js account", accountKey, jsAccountExt0Entry(), nativeAsset(),
              true, false, false, max, 0, 0 },
        };
        for (const Case& c : cases) {
            AnsweringSorobanServer server;
            server.answer = ledgerEntriesAnswer({ { c.key, c.xdr } });
            AssetBalance b;
            try {
                b = server.getAssetBalance(holderA(), c.asset, nullptr);
            } catch (const std::exception& e) {
                QFAIL(qPrintable(QString::fromLatin1(c.name) + QStringLiteral(": ") + QString::fromUtf8(e.what())));
            }
            QVERIFY2(b.exists, c.name);
            QCOMPARE(b.amount, Scv::fromInt128(Scv::toInt128(c.amount)));
            QCOMPARE(b.authorized, c.authorized);
            QCOMPARE(b.authorizedToMaintainLiabilities, c.maintain);
            QCOMPARE(b.clawbackEnabled, c.clawback);
            QCOMPARE(b.buyingLiabilities, c.buying);
            QCOMPARE(b.sellingLiabilities, c.selling);
            QCOMPARE(b.latestLedger, quint32(4916136));
        }
    }

    /**
     * Anything that is not exactly the entry asked for is refused: more than
     * one entry, another key, another entry (or kind of entry) under the key,
     * base64 that is not strictly base64, bytes left over or missing, an ext
     * arm stellar-xdr does not have, a trustline flag Core does not define or
     * both authorizations at once, a negative balance or liability. So are,
     * before anything is asked, a muxed account (whose balance is its G
     * account's), an address that holds no balance, a contract without its
     * network, a pool share and an asset code no ledger can hold.
     */
    void testGetAssetBalanceRefusesWhatIsNotTheBalanceAskedFor()
    {
        const stellar::Asset tn4 = issued(QStringLiteral("TN4"), testIssuer());
        const QString tn4Key = answeredField(realAssetAnswer("aTn4"), "key");
        const QString tn4Xdr = answeredField(realAssetAnswer("aTn4"), "xdr");
        const QString accountKey = answeredField(realAssetAnswer("aXlm"), "key");
        const QByteArray raw = QByteArray::fromBase64(tn4Xdr.toLatin1());

        // The real TN4 line from its parts, and the js account: the variants
        // below differ from what Core and js wrote only in what is varied.
        stellar::TrustLineEntry line;
        line.accountID = stellar::PublicKey::from(StellarAddress::fromString(holderA()).getBytes());
        line.asset = stellar::TrustLineAsset(tn4);
        line.balance = std::numeric_limits<qint64>::max();
        line.limit = std::numeric_limits<qint64>::max();
        line.flags = 5;
        QCOMPARE(trustLineData(line), tn4Xdr);
        auto varied = [&line](const std::function<void(stellar::TrustLineEntry&)>& change) {
            stellar::TrustLineEntry v = line;
            change(v);
            return trustLineData(v);
        };
        stellar::AccountEntry account;
        account.accountID = line.accountID;
        account.balance = std::numeric_limits<qint64>::max();
        account.seqNum = 21118470158548995LL;
        account.thresholds.thresholds[0] = 1;
        QCOMPARE(accountData(account), jsAccountExt0Entry());

        struct Refused { const char* what; QString key; QString xdr; bool native; };
        const QList<Refused> refused = {
            { "two entries", QString(), QString(), false },            // built below
            { "another key", answeredField(realAssetAnswer("bTestn"), "key"),
              answeredField(realAssetAnswer("bTestn"), "xdr"), false },
            { "another line under this key", tn4Key, answeredField(realAssetAnswer("aUsdc"), "xdr"), false },
            { "an account under a line's key", tn4Key, answeredField(realAssetAnswer("aXlm"), "xdr"), false },
            { "a line under the account's key", accountKey, tn4Xdr, true },
            { "no key", QString(), tn4Xdr, false },
            { "not base64", tn4Key, tn4Xdr.left(8) + QStringLiteral("!") + tn4Xdr.mid(8), false },
            { "trailing space", tn4Key, tn4Xdr + QStringLiteral(" "), false },
            { "bytes left over", tn4Key, QString::fromLatin1((raw + QByteArray(4, '\0')).toBase64()), false },
            { "cut short", tn4Key, QString::fromLatin1(raw.left(raw.size() - 4).toBase64()), false },
            { "empty", tn4Key, QString(), false },
            { "line ext 2", tn4Key, withLastWord(tn4Xdr, 2), false },
            { "line v1 ext 1", tn4Key, withLastWord(varied([](stellar::TrustLineEntry& v) { v.ext.reserved = 1; }), 1), false },
            { "flag 8", tn4Key, varied([](stellar::TrustLineEntry& v) { v.flags = 5 | 8; }), false },
            { "both authorizations", tn4Key, varied([](stellar::TrustLineEntry& v) { v.flags = 7; }), false },
            { "negative balance", tn4Key, varied([](stellar::TrustLineEntry& v) { v.balance = -1; }), false },
            { "negative liabilities", tn4Key, varied([](stellar::TrustLineEntry& v) {
                  v.ext.reserved = 1; v.v1.liabilities.buying = 0; v.v1.liabilities.selling = -1; }), false },
            { "account ext 2", accountKey, withLastWord(jsAccountExt0Entry(), 2), true },
            { "negative XLM", accountKey, [&account]() {
                  stellar::AccountEntry v = account; v.balance = -1; return accountData(v); }(), true },
        };
        for (const Refused& bad : refused) {
            AnsweringSorobanServer server;
            server.answer = QByteArray(bad.what) == "two entries"
                ? ledgerEntriesAnswer({ { tn4Key, tn4Xdr }, { tn4Key, tn4Xdr } })
                : ledgerEntriesAnswer({ { bad.key, bad.xdr } });
            const QString message = assetBalanceOutcome(server, holderA(), bad.native ? nativeAsset() : tn4, nullptr);
            // An answer whose key or entry is not base64 XDR is refused as it is read.
            const QByteArray what(bad.what);
            const bool unreadable = what == "no key" || what == "not base64" || what == "trailing space"
                                    || what == "empty";
            QVERIFY2(message.startsWith(unreadable ? QStringLiteral("Soroban RPC: getLedgerEntries answered entries[0].")
                                                   : QStringLiteral("getAssetBalance: ")),
                     qPrintable(QString::fromLatin1(bad.what) + QStringLiteral(": ") + message));
        }
        // What is left once the refusals are out reads, as the real one does.
        {
            AnsweringSorobanServer server;
            server.answer = ledgerEntriesAnswer({ { tn4Key, varied([](stellar::TrustLineEntry& v) { v.flags = 2; }) } });
            const AssetBalance b = server.getAssetBalance(holderA(), tn4, nullptr);
            QVERIFY(b.exists && !b.authorized && b.authorizedToMaintainLiabilities && !b.clawbackEnabled);
        }

        // Refused before anything is asked.
        stellar::Asset poolShare;
        poolShare.type = stellar::AssetType::ASSET_TYPE_POOL_SHARE;
        auto code4 = [](const QByteArray& code) {
            stellar::Asset a = issued(QStringLiteral("TN4"), testIssuer());
            std::memset(a.alphaNum4.assetCode, 0, 4);
            std::memcpy(a.alphaNum4.assetCode, code.constData(), size_t(qMin(4, int(code.size()))));
            return a;
        };
        auto code12 = [](const QByteArray& code) {
            stellar::Asset a = issued(QStringLiteral("TESTN"), testIssuer());
            std::memset(a.alphaNum12.assetCode, 0, 12);
            std::memcpy(a.alphaNum12.assetCode, code.constData(), size_t(qMin(12, int(code.size()))));
            return a;
        };
        const QString pool = StellarAddress(StellarAddress::Type::LIQUIDITY_POOL,
                                            QByteArray::fromHex("5a6b9707b55d67cf75af6a5115e020ce3d90e41a26a3970ce901a9d8a19a2410")).toString();
        // SEP-23's muxed example: GA7QYNF7...VSGZ with id 2^63.
        const QString muxed = QStringLiteral("MA7QYNF7SOWQ3GLR2BGMZEHXAVIRZA4KVWLTJJFC7MGXUA74P7UJVAAAAAAAAAAAAAJLK");
        QCOMPARE(StellarAddress::fromString(muxed).getType(), StellarAddress::Type::MUXED_ACCOUNT);
        struct Early { const char* what; QString address; stellar::Asset asset; Network* network; const char* says; };
        const QList<Early> early = {
            { "muxed", muxed, nativeAsset(), Network::testnetNetwork(), "(G...)" },
            { "a pool", pool, nativeAsset(), Network::testnetNetwork(), "only an account" },
            { "a contract without its network", sacHolder(), nativeAsset(), nullptr, "network" },
            { "a pool share", holderA(), poolShare, nullptr, "pool share" },
            { "an empty code", holderA(), code4(QByteArray()), nullptr, "asset code" },
            { "a dash", holderA(), code4("US-D"), nullptr, "asset code" },
            { "a gap", holderA(), code4(QByteArray("U\0SD", 4)), nullptr, "asset code" },
            { "not ASCII", holderA(), code4("\xC3\xA9"), nullptr, "asset code" },
            { "4 chars as alphanum12", holderA(), code12("ABCD"), nullptr, "asset code" },
        };
        for (const Early& bad : early) {
            AnsweringSorobanServer server;
            server.answer = ledgerEntriesAnswer({});
            const QString message = assetBalanceOutcome(server, bad.address, bad.asset, bad.network);
            QVERIFY2(message.startsWith(QStringLiteral("getAssetBalance: ")) && message.contains(QLatin1String(bad.says)),
                     qPrintable(QString::fromLatin1(bad.what) + QStringLiteral(": ") + message));
            QVERIFY2(server.methods.isEmpty(), bad.what);            // nothing asked
        }
        // Codes a ledger can hold are asked for: lower case, digits, 12 characters.
        for (const stellar::Asset& fine : { code4("usd1"), code4("A"), code12("abcdefghijk9"), code12("ABCDE") }) {
            AnsweringSorobanServer server;
            server.answer = ledgerEntriesAnswer({});
            QVERIFY(!server.getAssetBalance(holderA(), fine, nullptr).exists);
            QCOMPARE(server.methods.size(), 1);
        }
    }

    /**
     * A contract's balance is its asset contract's, as getSACBalance reads
     * it — under the same key, with the same checks — with the ledger the
     * RPC answered at.
     */
    void testGetAssetBalanceOfAContractIsItsSacBalance()
    {
        AnsweringSorobanServer server;
        server.answer = realSacBalanceAnswer();
        const AssetBalance b = server.getAssetBalance(sacHolder(), nativeAsset(), Network::testnetNetwork());
        QCOMPARE(server.lastParams.toObject().value("keys").toArray(), QJsonArray{ nativeBalanceKey() });
        QVERIFY(b.exists);
        QCOMPARE(Scv::decimalText(Scv::toInt128(b.amount)), QStringLiteral("741892200000"));
        QVERIFY(b.authorized && !b.clawbackEnabled && !b.authorizedToMaintainLiabilities);
        QCOMPARE(b.buyingLiabilities, qint64(0));
        QCOMPARE(b.sellingLiabilities, qint64(0));
        QCOMPARE(b.latestLedger, quint32(4916136));

        server.answer = ledgerEntriesAnswer({ { nativeBalanceKey(), pyNativeBalanceEntry() } });
        const AssetBalance big = server.getAssetBalance(sacHolder(), nativeAsset(), Network::testnetNetwork());
        QCOMPARE(big.amount, i128("00000000000000010000000000000005"));
        QVERIFY(big.clawbackEnabled);

        server.answer = ledgerEntriesAnswer({});
        const AssetBalance none = server.getAssetBalance(sacHolder(), nativeAsset(), Network::testnetNetwork());
        QVERIFY(!none.exists);
        QCOMPARE(none.amount, QByteArray(16, '\0'));
        QVERIFY(!none.authorized && !none.clawbackEnabled);

        server.answer = ledgerEntriesAnswer({ { nativeBalanceKey(), jsUsdcBalanceEntry() } });
        const QString message = assetBalanceOutcome(server, sacHolder(), nativeAsset(), Network::testnetNetwork());
        QVERIFY2(message.startsWith(QStringLiteral("getAssetBalance: ")), qPrintable(message));
    }
};

ADD_TEST(SorobanServerHelpersTest)
#endif // SOROBANSERVERHELPERSTEST_H
