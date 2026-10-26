#ifndef SOROBANQMLTEST_H
#define SOROBANQMLTEST_H

#include <QObject>
#include <QtTest>
#include "../testcollector.h"
#include "../fakeserver.h"

#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QSet>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <functional>
#include <limits>
#include <memory>
#include <type_traits>

#include "../../src/qml/sorobanqml.h"
#include "../../src/util.h"
#include "../../src/abstracttransaction.h"
#include "../../src/account.h"
#include "../../src/invokehostfunctionoperation.h"
#include "../../src/network.h"
#include "../../src/scval/scv.h"
#include "../../src/stellaraddress.h"
#include "../../src/transaction.h"
#include "../../src/transactionbuilder.h"
#include "../../xdr/sorobantxdata.h"

// Mostly qualified, although SorobanQml has the global alias the legacy flag
// gives every other QML class (testTheLegacyFlagAliasesSorobanQml pins it).

/**
 * A JSON-RPC endpoint that answers `getHealth`-shaped results with its own
 * ledger number, and, when held, only once released. It counts the requests
 * that reached it, so a test can tell whether one was ever sent.
 */
class HeldRpcEndpoint
{
public:
    HeldRpcEndpoint(int latestLedger, bool held) : m_ledger(latestLedger), m_held(held)
    {
        QObject::connect(&m_server, &QTcpServer::newConnection, [this]() {
            while (m_server.hasPendingConnections()) {
                QTcpSocket* socket = m_server.nextPendingConnection();
                QObject::connect(socket, &QTcpSocket::readyRead,
                                 [this, socket]() { read(socket); });
            }
        });
        m_server.listen(QHostAddress::LocalHost, 0);
    }

    QString url() const { return QStringLiteral("http://127.0.0.1:%1/").arg(m_server.serverPort()); }
    int requests() const { return m_requests; }

    /** Answers everything held so far, and everything from now on at once. */
    void release()
    {
        m_held = false;
        for (QTcpSocket* socket : m_waiting)
            answer(socket);
        m_waiting.clear();
    }

    /** Answers everything from now on at once; what is held stays unanswered. */
    void stopHolding() { m_held = false; }

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
            ++m_requests;
            if (m_held)
                m_waiting.append(socket);
            else
                answer(socket);
        }
    }

    void answer(QTcpSocket* socket)
    {
        const QByteArray body = QStringLiteral(
            "{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"result\":{\"status\":\"healthy\","
            "\"latestLedger\":%1,\"oldestLedger\":1,\"ledgerRetentionWindow\":17280}}")
            .arg(m_ledger).toUtf8();
        socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: "
                      + QByteArray::number(body.size()) + "\r\n\r\n" + body);
    }

    QTcpServer m_server;
    QHash<QTcpSocket*, QByteArray> m_buffers;
    QList<QTcpSocket*> m_waiting;
    int m_ledger;
    bool m_held;
    int m_requests = 0;
};

/**
 * A JSON-RPC endpoint that answers by method, as a real one does: getNetwork
 * with a real testnet answer unless given another, and every other method
 * with the answer it was given (a "method not found" error until then). A
 * method can be held (read, never answered) or hung up on (read in full, then
 * the connection dropped), every time or only the first `times` it is read
 * (hangUpFirst: held or answered after that). It counts the requests of each
 * method that reached it, and calls `onRequest` with each one's method once
 * it has been handled.
 */
class MethodRpcEndpoint
{
public:
    /** soroban-testnet.stellar.org's getNetwork answer, as given on 2026-09-29. */
    static QByteArray testnetNetwork()
    {
        return "{\"jsonrpc\":\"2.0\",\"id\":8675309,\"result\":{\"friendbotUrl\":"
               "\"https://friendbot.stellar.org/\",\"passphrase\":\"Test SDF Network ; "
               "September 2015\",\"protocolVersion\":28}}";
    }

    /** mainnet.sorobanrpc.com's getNetwork answer, as given on 2026-09-29 (no friendbot). */
    static QByteArray mainnetNetwork()
    {
        return "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"passphrase\":\"Public Global "
               "Stellar Network ; September 2015\",\"protocolVersion\":28}}";
    }

    MethodRpcEndpoint()
    {
        m_answers.insert(QStringLiteral("getNetwork"), testnetNetwork());
        QObject::connect(&m_server, &QTcpServer::newConnection, [this]() {
            while (m_server.hasPendingConnections()) {
                QTcpSocket* socket = m_server.nextPendingConnection();
                QObject::connect(socket, &QTcpSocket::readyRead,
                                 [this, socket]() { read(socket); });
            }
        });
        m_server.listen(QHostAddress::LocalHost, 0);
    }

    QString url() const { return QStringLiteral("http://127.0.0.1:%1/").arg(m_server.serverPort()); }

    void answer(const QString& method, const QByteArray& body) { m_answers.insert(method, body); }
    void hold(const QString& method) { m_held.insert(method); }
    void hangUp(const QString& method) { m_hangUps.insert(method); }
    void hangUpFirst(const QString& method, int times) { m_hangUpsLeft.insert(method, times); }
    int requests(const QString& method) const { return int(m_methods.count(method)); }
    /** The `params` of the last request of that method that reached it. */
    QJsonValue lastParams(const QString& method) const { return m_params.value(method); }

    std::function<void(const QString&)> onRequest;

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
            const QString method = call.value(QStringLiteral("method")).toString();
            buffer.remove(0, end + 4 + length);
            m_methods.append(method);
            m_params.insert(method, call.value(QStringLiteral("params")));
            bool hangUp = m_hangUps.contains(method);
            if (!hangUp && m_hangUpsLeft.value(method) > 0) {
                m_hangUpsLeft.insert(method, m_hangUpsLeft.value(method) - 1);
                hangUp = true;
            }
            if (hangUp) {
                buffer.clear();
                socket->abort();
            } else if (!m_held.contains(method)) {
                const QByteArray body = m_answers.value(method,
                    "{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"error\":{\"code\":-32601,"
                    "\"message\":\"method not found\"}}");
                socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: "
                              + QByteArray::number(body.size()) + "\r\n\r\n" + body);
                socket->flush();
            }
            if (onRequest)
                onRequest(method);
            if (hangUp)
                return;
        }
    }

    QTcpServer m_server;
    QHash<QTcpSocket*, QByteArray> m_buffers;
    QHash<QString, QByteArray> m_answers;
    QSet<QString> m_held;
    QSet<QString> m_hangUps;
    QHash<QString, int> m_hangUpsLeft;
    QStringList m_methods;
    QHash<QString, QJsonValue> m_params;
};

/**
 * Reading a contract, which is also the first half of deploying one.
 *
 * The RPC side of SorobanQml needs a network and is left to the live suite;
 * these three invokables need nothing but bytes, and they are the ones an app
 * builds a deploy on. What is pinned here is that the report carries the
 * module itself, not only a description of it: an app that can read a .wasm
 * and cannot hand those bytes to `uploadContractWasm` has stopped one step
 * short of the only thing the file is for. (How a simulated return value is
 * shown is the one RPC case here: a local FakeServer answers it.)
 */
class SorobanQmlTest: public QObject
{
    Q_OBJECT

    /** The eight bytes every WebAssembly module starts with. */
    static QByteArray header()
    {
        return QByteArray::fromHex("0061736d01000000");
    }

    static QByteArray leb(quint32 value)
    {
        QByteArray out;
        do {
            quint8 byte = value & 0x7F;
            value >>= 7;
            if (value) byte |= 0x80;
            out.append(char(byte));
        } while (value);
        return out;
    }

    static QByteArray customSection(const QByteArray& name, const QByteArray& payload)
    {
        const QByteArray body = leb(quint32(name.size())) + name + payload;
        return QByteArray(1, char(0)) + leb(quint32(body.size())) + body;
    }

    /** A module that is real WebAssembly and carries no contract interface. */
    static QByteArray plainModule()
    {
        return header() + customSection("producers", "clang");
    }

    /** A Soroban envelope for testnet, signed or not, and its hash. */
    static QString sorobanEnvelope(QString* hash, bool sign = false)
    {
        const QString source = QStringLiteral("GD65DGZZEC5ADUA5QVCA2OA6P5DKDDKOB2FMEMHCNJPCPT6XN3HQQYUG");
        Account account(source, 1);
        TransactionBuilder builder(&account, Network::testnetNetwork());
        builder.addOperation(InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
            StellarAddress::fromString(QStringLiteral("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC")),
            QStringLiteral("balance"), QList<stellar::SCVal>{Scv::toAddress(source)}));
        builder.setBaseFee(100);
        builder.setTimeout(300);
        QScopedPointer<Transaction> tx(builder.build());
        if (sign) {
            QScopedPointer<KeyPair> signer(KeyPair::random());
            tx->sign(signer.data());
        }
        if (hash)
            *hash = tx->hashHex();
        return tx->toEnvelopeXdrBase64();
    }

    /** A sendTransaction answer, as the RPC gives one (with its errorResultXdr, if given). */
    static QByteArray sendAnswer(const QString& status, const QString& hash,
                                 const QString& errorResultXdr = QString())
    {
        QJsonObject result;
        result.insert(QStringLiteral("status"), status);
        result.insert(QStringLiteral("hash"), hash);
        result.insert(QStringLiteral("latestLedger"), 4917043);
        result.insert(QStringLiteral("latestLedgerCloseTime"), QStringLiteral("1790608752"));
        if (!errorResultXdr.isEmpty())
            result.insert(QStringLiteral("errorResultXdr"), errorResultXdr);
        QJsonObject response;
        response.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
        response.insert(QStringLiteral("id"), QStringLiteral("1"));
        response.insert(QStringLiteral("result"), result);
        return QJsonDocument(response).toJson(QJsonDocument::Compact);
    }

    /** The envelope's hash on the network with that passphrase. */
    static QString hashOn(const QString& envelope, const QString& passphrase)
    {
        Network network(passphrase);
        QScopedPointer<AbstractTransaction> parsed(
            AbstractTransaction::fromEnvelopeXdr(envelope, &network));
        return parsed->hashHex();
    }

    /** What `start` ends in, sent or failed: { "sent"|"failed", map }. */
    template <typename Start>
    static QPair<QString, QVariantMap> outcomeOf(qstellar::SorobanQml& soroban, Start start,
                                                 int timeout = 20000)
    {
        QSignalSpy sent(&soroban, &qstellar::SorobanQml::sent);
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);
        const int id = start();
        QElapsedTimer clock;
        clock.start();
        while (clock.elapsed() < timeout) {
            for (const QList<QVariant>& args : sent) {
                if (args.at(0).toInt() == id)
                    return { QStringLiteral("sent"), args.at(1).toMap() };
            }
            for (const QList<QVariant>& args : failed) {
                if (args.at(0).toInt() == id)
                    return { QStringLiteral("failed"), args.at(1).toMap() };
            }
            QTest::qWait(5);
        }
        return { QStringLiteral("nothing"), QVariantMap() };
    }

    /** The failure `start` ends in, or an empty map if none arrives. */
    template <typename Start>
    static QVariantMap failureOf(qstellar::SorobanQml& soroban, Start start)
    {
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);
        const int id = start();
        for (int waited = 0; waited < 40 && failed.isEmpty(); ++waited)
            failed.wait(500);
        for (const QList<QVariant>& args : failed) {
            if (args.at(0).toInt() == id)
                return args.at(1).toMap();
        }
        return QVariantMap();
    }

private slots:
    void initTestCase() {}
    void cleanupTestCase() {}

    // ─── The bytes come back ─────────────────────────────────────────

    /** The whole point: what was read can be uploaded without leaving the app. */
    void testTheModuleComesBackAsHexForAnUpload()
    {
        qstellar::SorobanQml soroban;
        const QByteArray wasm = plainModule();

        const QVariantMap report = soroban.readContractHex(QString::fromLatin1(wasm.toHex()));

        QVERIFY(report.value(QStringLiteral("isWasm")).toBool());
        QCOMPARE(report.value(QStringLiteral("wasmHex")).toString(),
                 QString::fromLatin1(wasm.toHex()));
        QCOMPARE(report.value(QStringLiteral("wasmSize")).toInt(), int(wasm.size()));
    }

    /** A module with no published interface is still deployable, so it still
     *  carries its bytes. Nothing about an upload needs a spec. */
    void testAModuleWithNoInterfaceStillCarriesItsBytes()
    {
        qstellar::SorobanQml soroban;
        const QVariantMap report =
            soroban.readContractHex(QString::fromLatin1(plainModule().toHex()));

        QVERIFY(report.value(QStringLiteral("functions")).toList().isEmpty());
        QVERIFY(!report.value(QStringLiteral("wasmHex")).toString().isEmpty());
    }

    /** The hash the ledger files code under, and the one createContract names,
     *  are the same SHA-256 — known before anything is sent, which is what
     *  lets an app fill in the second transaction from the first file. */
    void testWasmHashIsTheSha256OfThoseSameBytes()
    {
        qstellar::SorobanQml soroban;
        const QByteArray wasm = plainModule();

        const QVariantMap report =
            soroban.readContractHex(QString::fromLatin1(wasm.toHex()));

        QCOMPARE(report.value(QStringLiteral("wasmHash")).toString(),
                 QString::fromLatin1(Util::hash(wasm).toHex()));
        QCOMPARE(report.value(QStringLiteral("wasmHash")).toString().size(), 64);
    }

    /** Hex pasted from a hex dump is often upper case; the operation field is
     *  not, so the report normalises rather than handing back what it was given. */
    void testHexIsNormalisedWhateverCaseItArrivesIn()
    {
        qstellar::SorobanQml soroban;
        const QByteArray wasm = plainModule();

        const QVariantMap upper =
            soroban.readContractHex(QString::fromLatin1(wasm.toHex().toUpper()));

        QCOMPARE(upper.value(QStringLiteral("wasmHex")).toString(),
                 QString::fromLatin1(wasm.toHex()));
    }

    // ─── And are absent when there is nothing to upload ──────────────

    /** Offering to upload something that is not a contract would be offering
     *  to spend a fee on it, so the keys are missing rather than empty. */
    void testSomethingThatIsNotWebAssemblyCarriesNoBytes()
    {
        qstellar::SorobanQml soroban;
        const QVariantMap report =
            soroban.readContractHex(QString::fromLatin1(QByteArray("not a module").toHex()));

        QVERIFY(!report.value(QStringLiteral("isWasm")).toBool());
        QVERIFY(!report.contains(QStringLiteral("wasmHex")));
        QVERIFY(!report.contains(QStringLiteral("wasmHash")));
        QVERIFY(!report.contains(QStringLiteral("wasmSize")));
        QVERIFY(!report.value(QStringLiteral("problem")).toString().isEmpty());
    }

    /** A file that is not there is a different failure from a file that is not
     *  a contract, and neither may throw: QML cannot catch a C++ exception. */
    void testUnreadableInputFailsWithAMessageRatherThanAnException()
    {
        qstellar::SorobanQml soroban;

        const QVariantMap missing =
            soroban.readContractPath(QStringLiteral("no-such-contract.wasm"));
        QVERIFY(!missing.value(QStringLiteral("isWasm")).toBool());
        QVERIFY(!missing.contains(QStringLiteral("wasmHex")));
        QVERIFY(missing.value(QStringLiteral("problem")).toString()
                    .contains(QStringLiteral("no-such-contract.wasm")));

        const QStringList garbage = {
            QString(), QStringLiteral(""), QStringLiteral("   "),
            QStringLiteral("zzzz"), QStringLiteral("0061736d0100000"),
            QStringLiteral("<html>"), QString(1000, QChar('g')),
        };
        for (const QString& input : garbage) {
            const QVariantMap report = soroban.readContractHex(input);
            QVERIFY2(!report.contains(QStringLiteral("wasmHex")),
                     qPrintable(QStringLiteral("offered an upload for: ") + input));
        }
    }

    // ─── What a simulation returned ──────────────────────────────────

    /**
     * A simulated return value is shown exactly whatever its integer width.
     * An i128 — what every token balance is — came back as "(type 10)", and
     * u128 / u256 / i256 the same way. A local fake RPC answers
     * simulateTransaction with each value in turn.
     */
    void testSimulatedWideIntegersAreShownExactly()
    {
        QString transactionData;
        {
            QByteArray raw;
            QDataStream s(&raw, QIODevice::WriteOnly);
            s << stellar::SorobanTransactionData();
            transactionData = QString::fromLatin1(raw.toBase64());
        }
        FakeServer rpc;
        auto answer = [&](const QString& returnXdr) {
            QJsonObject first;
            first.insert(QStringLiteral("xdr"), returnXdr);
            first.insert(QStringLiteral("auth"), QJsonArray());
            QJsonObject result;
            result.insert(QStringLiteral("latestLedger"), 123);
            result.insert(QStringLiteral("minResourceFee"), QStringLiteral("100"));
            result.insert(QStringLiteral("transactionData"), transactionData);
            result.insert(QStringLiteral("results"), QJsonArray{first});
            QJsonObject response;
            response.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
            response.insert(QStringLiteral("id"), QStringLiteral("1"));
            response.insert(QStringLiteral("result"), result);
            FakeServer::Reply reply;
            reply.contentType = QStringLiteral("application/json");
            reply.body = QString::fromUtf8(QJsonDocument(response).toJson(QJsonDocument::Compact));
            rpc.addPostFull(QStringLiteral("/"), reply);
        };

        const QString source = QStringLiteral("GD65DGZZEC5ADUA5QVCA2OA6P5DKDDKOB2FMEMHCNJPCPT6XN3HQQYUG");
        Account account(source, 1);
        TransactionBuilder builder(&account, Network::testnetNetwork());
        builder.addOperation(InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
            StellarAddress::fromString(QStringLiteral("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC")),
            QStringLiteral("balance"), QList<stellar::SCVal>{Scv::toAddress(source)}));
        builder.setBaseFee(100);
        builder.setTimeout(300);
        QScopedPointer<Transaction> tx(builder.build());
        const QString envelope = tx->toEnvelopeXdrBase64();

        qstellar::SorobanQml soroban;
        soroban.setEndpoint(rpc.baseUrl());
        QSignalSpy simulated(&soroban, &qstellar::SorobanQml::simulated);
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);

        const QList<QPair<stellar::SCVal, QString>> cases = {
            { Scv::toInt128(qint64(100000000000LL)), QStringLiteral("100000000000") },
            { Scv::toInt128(-1), QStringLiteral("-1") },
            { Scv::toInt128Parts(qint64(-9223372036854775807LL - 1), quint64(0)),
              QStringLiteral("-170141183460469231731687303715884105728") },
            { Scv::toUint128Parts(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL),
              QStringLiteral("340282366920938463463374607431768211455") },
            { Scv::toUint256(QByteArray(32, '\xff')),
              QStringLiteral("115792089237316195423570985008687907853269984665640564039457584007913129639935") },
            { Scv::toInt256(QByteArray(1, '\xfb')), QStringLiteral("-5") },
        };
        for (const auto& c : cases) {
            QByteArray raw;
            { QDataStream s(&raw, QIODevice::WriteOnly); s << c.first; }
            const QString returnXdr = QString::fromLatin1(raw.toBase64());
            answer(returnXdr);
            simulated.clear();
            soroban.simulate(envelope);
            QVERIFY2(simulated.wait(15000),
                     qPrintable(failed.isEmpty() ? QStringLiteral("no answer")
                                                 : failed.first().at(1).toMap().value(QStringLiteral("message")).toString()));
            const QVariantMap result = simulated.first().at(1).toMap();
            QCOMPARE(result.value(QStringLiteral("returnValue")).toString(), c.second);
            QCOMPARE(result.value(QStringLiteral("returnValueXdr")).toString(), returnXdr);
        }
    }

    /**
     * What a simulation returned is shown only when it decodes whole: strict
     * base64 and every byte read. A character that is not base64 used to be
     * skipped and bytes left over ignored, so a mangled answer was shown as
     * the value the contract returned, and a mangled footprint as its fee.
     */
    void testASimulatedValueIsShownOnlyWhenItDecodesWhole()
    {
        QByteArray valueXdr;
        { QDataStream s(&valueXdr, QIODevice::WriteOnly); s << Scv::toUint32(42); }
        stellar::SorobanTransactionData footprint;
        footprint.resourceFee = 777;
        QByteArray footprintXdr;
        { QDataStream s(&footprintXdr, QIODevice::WriteOnly); s << footprint; }
        const QString value = QString::fromLatin1(valueXdr.toBase64());
        const QString data = QString::fromLatin1(footprintXdr.toBase64());
        auto mangled = [](const QString& base64) {
            return base64.left(4) + QStringLiteral("!") + base64.mid(4);
        };
        auto trailing = [](const QByteArray& xdr) {
            return QString::fromLatin1((xdr + QByteArray(4, '\0')).toBase64());
        };

        FakeServer rpc;
        auto answer = [&](const QString& returnXdr, const QString& transactionData) {
            QJsonObject first;
            first.insert(QStringLiteral("xdr"), returnXdr);
            first.insert(QStringLiteral("auth"), QJsonArray());
            QJsonObject result;
            result.insert(QStringLiteral("latestLedger"), 123);
            result.insert(QStringLiteral("transactionData"), transactionData);
            result.insert(QStringLiteral("results"), QJsonArray{first});
            QJsonObject response;
            response.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
            response.insert(QStringLiteral("id"), QStringLiteral("1"));
            response.insert(QStringLiteral("result"), result);
            FakeServer::Reply reply;
            reply.contentType = QStringLiteral("application/json");
            reply.body = QString::fromUtf8(QJsonDocument(response).toJson(QJsonDocument::Compact));
            rpc.addPostFull(QStringLiteral("/"), reply);
        };

        qstellar::SorobanQml soroban;
        soroban.setEndpoint(rpc.baseUrl());
        QSignalSpy simulated(&soroban, &qstellar::SorobanQml::simulated);
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);
        auto simulate = [&](const QString& returnXdr, const QString& transactionData) {
            answer(returnXdr, transactionData);
            simulated.clear();
            soroban.simulate(QStringLiteral("AAAAAgAAAAA="));   // the fake RPC does not read it
            if (!simulated.wait(15000))
                return QVariantMap();
            return simulated.first().at(1).toMap();
        };

        const QVariantMap whole = simulate(value, data);
        QCOMPARE(whole.value(QStringLiteral("returnValue")).toString(), QStringLiteral("42"));
        QCOMPARE(whole.value(QStringLiteral("resourceFee")).toString(), QStringLiteral("777"));

        // Bytes left over: the answer is shown, without the value or the fee.
        const QVariantMap shown = simulate(trailing(valueXdr), trailing(footprintXdr));
        QVERIFY(!shown.isEmpty());
        QVERIFY2(!shown.contains(QStringLiteral("returnValue")),
                 qPrintable(shown.value(QStringLiteral("returnValue")).toString()));
        QVERIFY2(!shown.contains(QStringLiteral("resourceFee")),
                 qPrintable(shown.value(QStringLiteral("resourceFee")).toString()));
        QCOMPARE(failed.size(), 0);

        // Not base64 at all: the answer itself is refused as it is read, and
        // nothing of it is shown.
        simulated.clear();
        answer(mangled(value), mangled(data));
        soroban.simulate(QStringLiteral("AAAAAgAAAAA="));
        QVERIFY(failed.wait(15000));
        QVERIFY(simulated.isEmpty());
        QVERIFY2(failed.first().at(1).toMap().value(QStringLiteral("message")).toString()
                     .contains(QStringLiteral("is not base64")),
                 qPrintable(failed.first().at(1).toMap().value(QStringLiteral("message")).toString()));
    }

    /**
     * prepare() refuses an envelope whose own soroban data carries a negative
     * resource fee, and hands back nothing to sign. The bid was recovered as
     * the fee less that amount: fee 100 with -1000 came back prepared at 1400
     * stroops for a 300-stroop simulation, and INT64_MIN overflowed. A local
     * fake RPC answers the simulation.
     */
    void testPrepareRefusesANegativeResourceFeeOfItsOwn()
    {
        stellar::SorobanTransactionData simulated;
        simulated.resourceFee = 300;
        QByteArray simulatedXdr;
        { QDataStream s(&simulatedXdr, QIODevice::WriteOnly); s << simulated; }
        QByteArray voidXdr;
        { QDataStream s(&voidXdr, QIODevice::WriteOnly); s << Scv::toVoid(); }
        QJsonObject first;
        first.insert(QStringLiteral("xdr"), QString::fromLatin1(voidXdr.toBase64()));
        first.insert(QStringLiteral("auth"), QJsonArray());
        QJsonObject result;
        result.insert(QStringLiteral("latestLedger"), 123);
        result.insert(QStringLiteral("minResourceFee"), QStringLiteral("300"));
        result.insert(QStringLiteral("transactionData"), QString::fromLatin1(simulatedXdr.toBase64()));
        result.insert(QStringLiteral("results"), QJsonArray{first});
        QJsonObject response;
        response.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
        response.insert(QStringLiteral("id"), QStringLiteral("1"));
        response.insert(QStringLiteral("result"), result);
        FakeServer rpc;
        FakeServer::Reply reply;
        reply.contentType = QStringLiteral("application/json");
        reply.body = QString::fromUtf8(QJsonDocument(response).toJson(QJsonDocument::Compact));
        rpc.addPostFull(QStringLiteral("/"), reply);

        const QString source = QStringLiteral("GD65DGZZEC5ADUA5QVCA2OA6P5DKDDKOB2FMEMHCNJPCPT6XN3HQQYUG");
        Account account(source, 1);
        TransactionBuilder builder(&account, Network::testnetNetwork());
        builder.addOperation(InvokeHostFunctionOperation::invokeContractFunctionOperationBuilder(
            StellarAddress::fromString(QStringLiteral("CDLZFC3SYJYDZT7K67VZ75HPJVIEUVNIXF47ZG2FB2RMQQVU2HHGCYSC")),
            QStringLiteral("balance"), QList<stellar::SCVal>{Scv::toAddress(source)}));
        builder.setBaseFee(100);
        builder.setTimeout(300);
        stellar::SorobanTransactionData own;
        own.resourceFee = 1000;
        builder.setSorobanData(own);
        QScopedPointer<Transaction> tx(builder.build());

        qstellar::SorobanQml soroban;
        soroban.setEndpoint(rpc.baseUrl());
        QSignalSpy prepared(&soroban, &qstellar::SorobanQml::prepared);
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);

        for (const qint64 negative : { qint64(-1000), std::numeric_limits<qint64>::min() }) {
            stellar::TransactionEnvelope envelope = tx->toEnvelopeXdr();
            envelope.v1.tx.fee = 100;
            envelope.v1.tx.ext.sorobanData.resourceFee = negative;
            QByteArray raw;
            { QDataStream s(&raw, QIODevice::WriteOnly); s << envelope; }

            prepared.clear();
            failed.clear();
            const int id = soroban.prepare(QString::fromLatin1(raw.toBase64()),
                                           Network::testnetNetwork()->getNetworkPassphrase());
            QTRY_VERIFY_WITH_TIMEOUT(!prepared.isEmpty() || !failed.isEmpty(), 15000);
            QVERIFY2(prepared.isEmpty(), prepared.isEmpty() ? ""
                     : qPrintable(QStringLiteral("prepared at a fee of ")
                                  + prepared.first().at(1).toMap().value(QStringLiteral("fee")).toString()));
            QCOMPARE(failed.first().at(0).toInt(), id);
            const QString message =
                failed.first().at(1).toMap().value(QStringLiteral("message")).toString();
            QVERIFY2(message.contains(QStringLiteral("negative resource fee")), qPrintable(message));
        }
    }

    // ─── Changing the endpoint while a request is on the wire ────────

    /**
     * The worker waits for every RPC answer in a nested event loop, and that
     * loop delivered the next request: one started after an endpoint change
     * ran inside the call in flight and deleted its server — and the reply it
     * was waiting on — from under it, so the first request never answered.
     * Requests still run one after the other, each against the endpoint it was
     * started with, but an endpoint change now abandons the request on the
     * wire: it fails as "cancelled" and the next one runs. Until now it ran to
     * completion first, so here, where the first endpoint never answers, the
     * request to the second waited forever. (This test used to release the
     * first endpoint and expect its answer, 111, before the second's; that
     * expectation was the stall.)
     */
    void testAnEndpointChangeMidRequestWaitsItsTurn()
    {
        HeldRpcEndpoint first(111, /*held*/ true);
        HeldRpcEndpoint second(222, /*held*/ false);

        qstellar::SorobanQml soroban;
        QSignalSpy health(&soroban, &qstellar::SorobanQml::health);
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);
        QList<int> order;
        QObject::connect(&soroban, &qstellar::SorobanQml::health,
                         [&order](int id, const QVariantMap&) { order.append(id); });
        QObject::connect(&soroban, &qstellar::SorobanQml::failed,
                         [&order](int id, const QVariantMap&) { order.append(id); });

        soroban.setEndpoint(first.url());
        const int a = soroban.checkHealth();
        QTRY_COMPARE_WITH_TIMEOUT(first.requests(), 1, 15000);   // on the wire, never answered

        soroban.setEndpoint(second.url());
        const int b = soroban.checkHealth();
        QTRY_COMPARE_WITH_TIMEOUT(health.size(), 1, 15000);
        QCOMPARE(failed.size(), 1);
        QCOMPARE(failed.first().at(0).toInt(), a);
        QCOMPARE(failed.first().at(1).toMap().value(QStringLiteral("kind")).toString(),
                 QStringLiteral("cancelled"));
        QCOMPARE(health.first().at(0).toInt(), b);
        QCOMPARE(health.first().at(1).toMap().value(QStringLiteral("latestLedger")).toInt(), 222);
        QCOMPARE(order, (QList<int>{ a, b }));      // one after the other, never nested
        QCOMPARE(first.requests(), 1);
        QCOMPARE(second.requests(), 1);
        QCOMPARE(soroban.pending(), 0);
    }

    /**
     * cancelAll() aborts the request on the wire, and the next one runs at
     * once. It used to run to completion — for an endpoint that never
     * answers, never — and every request after it waited behind it. Nothing
     * of the cancelled one is delivered.
     */
    void testCancelAllReleasesAStalledRequest()
    {
        HeldRpcEndpoint endpoint(111, /*held*/ true);

        qstellar::SorobanQml soroban;
        QSignalSpy health(&soroban, &qstellar::SorobanQml::health);
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);

        soroban.setEndpoint(endpoint.url());
        soroban.checkHealth();
        QTRY_COMPARE_WITH_TIMEOUT(endpoint.requests(), 1, 15000);   // on the wire, never answered

        soroban.cancelAll();
        QCOMPARE(soroban.pending(), 0);
        endpoint.stopHolding();                 // the held request stays unanswered
        const int after = soroban.checkHealth();
        QTRY_COMPARE_WITH_TIMEOUT(health.size(), 1, 15000);
        QCOMPARE(health.first().at(0).toInt(), after);
        QCOMPARE(health.first().at(1).toMap().value(QStringLiteral("latestLedger")).toInt(), 111);
        QTest::qWait(100);
        QCOMPARE(health.size(), 1);
        QCOMPARE(failed.size(), 0);
        QCOMPARE(endpoint.requests(), 2);
        QCOMPARE(soroban.pending(), 0);
    }

    /**
     * cancelAll() drops a request still waiting behind the one on the wire:
     * a cancelled send is never sent, and nothing is said of it. (Nested, it
     * used to be on its way before cancelAll() could even be called.) The
     * worker is free afterwards. The send is a real envelope: one that does
     * not decode makes no request at all now, so it could not show that the
     * send was dropped.
     *
     * The send is queued behind a request held on the wire to the endpoint
     * it goes to, so it cannot start before cancelAll() whatever the timing.
     * It used to be queued right after an endpoint change had aborted the
     * request on the wire, with the worker idle: when this thread was held up
     * between send() and cancelAll() the send started, and its first request
     * reached the endpoint.
     */
    void testCancelAllDropsWhatHasNotStarted()
    {
        HeldRpcEndpoint first(111, /*held*/ true);
        HeldRpcEndpoint second(222, /*held*/ true);

        qstellar::SorobanQml soroban;
        QSignalSpy health(&soroban, &qstellar::SorobanQml::health);
        QSignalSpy sent(&soroban, &qstellar::SorobanQml::sent);
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);

        soroban.setEndpoint(first.url());
        const int onFirst = soroban.checkHealth();
        QTRY_COMPARE_WITH_TIMEOUT(first.requests(), 1, 15000);

        // The endpoint change aborts it and reports it; the next request is
        // then on the wire to the second endpoint, held there.
        soroban.setEndpoint(second.url());
        soroban.checkHealth();
        QTRY_COMPARE_WITH_TIMEOUT(second.requests(), 1, 15000);
        QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 1, 15000);
        QCOMPARE(failed.first().at(0).toInt(), onFirst);

        soroban.send(sorobanEnvelope(nullptr));     // waits behind the one held
        soroban.cancelAll();
        // The send until the worker says it never started it, which it does
        // as soon as the request held on the wire is aborted.
        QCOMPARE(soroban.pending(), 1);
        QTRY_COMPARE_WITH_TIMEOUT(soroban.pending(), 0, 15000);

        first.release();
        second.stopHolding();                   // the held request stays unanswered
        const int after = soroban.checkHealth();
        QTRY_COMPARE_WITH_TIMEOUT(health.size(), 1, 15000);
        QCOMPARE(health.first().at(0).toInt(), after);
        QCOMPARE(health.first().at(1).toMap().value(QStringLiteral("latestLedger")).toInt(), 222);
        // Requests run in order, so the cancelled send would have reached the
        // second endpoint (its network lookup) before this health check did.
        QCOMPARE(second.requests(), 2);
        QCOMPARE(sent.size(), 0);
        QCOMPARE(failed.size(), 1);                 // the first endpoint's, only
        QCOMPARE(soroban.pending(), 0);
    }

    /**
     * cancelAll() never forgets a send the worker had started: it is
     * reported, once, and `pending` counts it until then. Stopped during its
     * network lookup it was not sent: "cancelled before it was sent",
     * mayHaveReachedServer false, no hash. Stopped once the endpoint had read
     * its sendTransaction whole, it may land: mayHaveReachedServer true, with
     * its hash. The same when an endpoint change stopped it and cancelAll()
     * came before its report. Its report used to be dropped with the rest, so
     * a wallet that cancelled while a send was on the wire never heard that
     * it may land, and could pay again. The other requests are as
     * documented: a simulation or a preparation on the wire reports nothing
     * and leaves `pending` at once.
     */
    void testCancelAllStillReportsASendThatHadStarted()
    {
        const QString passphrase = Network::TESTNET_NETWORK_PASSPHRASE();
        QString hash;
        const QString envelope = sorobanEnvelope(&hash);
        qstellar::SorobanQml soroban;
        QSignalSpy sent(&soroban, &qstellar::SorobanQml::sent);
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);
        // Every report of `id` so far: { "sent"|"failed", map }.
        auto reportsOf = [&](int id) {
            QList<QPair<QString, QVariantMap>> reports;
            for (const QList<QVariant>& args : sent) {
                if (args.at(0).toInt() == id)
                    reports.append({ QStringLiteral("sent"), args.at(1).toMap() });
            }
            for (const QList<QVariant>& args : failed) {
                if (args.at(0).toInt() == id)
                    reports.append({ QStringLiteral("failed"), args.at(1).toMap() });
            }
            return reports;
        };
        // The one report of `id`, once it has come: a failure, and no other.
        auto onlyFailureOf = [&](int id) -> QVariantMap {
            QTest::qWait(50);
            const QList<QPair<QString, QVariantMap>> reports = reportsOf(id);
            if (reports.size() != 1 || reports.first().first != QStringLiteral("failed"))
                return QVariantMap();
            return reports.first().second;
        };
        const QList<QPair<const char*, std::function<int()>>> sends = {
            { "given its network", [&]() { return soroban.send(envelope, passphrase); } },
            { "without it", [&]() { return soroban.send(envelope); } },
        };
        for (const auto& send : sends) {
            // Held during its network lookup: not sent, and said so.
            {
                MethodRpcEndpoint rpc;
                rpc.hold(QStringLiteral("getNetwork"));
                rpc.answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), hash));
                soroban.setEndpoint(rpc.url());
                const int id = send.second();
                QTRY_COMPARE_WITH_TIMEOUT(rpc.requests(QStringLiteral("getNetwork")), 1, 15000);
                soroban.cancelAll();
                QCOMPARE(soroban.pending(), 1);
                QTRY_COMPARE_WITH_TIMEOUT(reportsOf(id).size(), 1, 15000);
                QCOMPARE(soroban.pending(), 0);
                const QVariantMap error = onlyFailureOf(id);
                QVERIFY2(!error.isEmpty(), send.first);
                QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("cancelled"));
                QCOMPARE(error.value(QStringLiteral("message")).toString(),
                         QStringLiteral("cancelled before it was sent"));
                QVERIFY2(error.contains(QStringLiteral("mayHaveReachedServer")), send.first);
                QVERIFY2(!error.value(QStringLiteral("mayHaveReachedServer")).toBool(), send.first);
                QVERIFY2(!error.contains(QStringLiteral("hash")), send.first);
                QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), 0);
            }
            // Its sendTransaction read whole by the endpoint, and held: it may land.
            {
                MethodRpcEndpoint rpc;
                rpc.hold(QStringLiteral("sendTransaction"));
                soroban.setEndpoint(rpc.url());
                const int id = send.second();
                QTRY_COMPARE_WITH_TIMEOUT(rpc.requests(QStringLiteral("sendTransaction")), 1, 15000);
                soroban.cancelAll();
                QCOMPARE(soroban.pending(), 1);
                QTRY_COMPARE_WITH_TIMEOUT(reportsOf(id).size(), 1, 15000);
                QCOMPARE(soroban.pending(), 0);
                const QVariantMap error = onlyFailureOf(id);
                QVERIFY2(!error.isEmpty(), send.first);
                QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("cancelled"));
                QVERIFY2(error.value(QStringLiteral("mayHaveReachedServer")).toBool(), send.first);
                QCOMPARE(error.value(QStringLiteral("hash")).toString(), hash);
                QVERIFY2(error.value(QStringLiteral("message")).toString().contains(QStringLiteral("look it up")),
                         qPrintable(error.value(QStringLiteral("message")).toString()));
                QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), 1);
            }
            // Stopped by an endpoint change, and cancelAll() before its report
            // came: reported all the same.
            {
                MethodRpcEndpoint rpc;
                rpc.hold(QStringLiteral("sendTransaction"));
                HeldRpcEndpoint elsewhere(222, /*held*/ false);
                soroban.setEndpoint(rpc.url());
                const int id = send.second();
                QTRY_COMPARE_WITH_TIMEOUT(rpc.requests(QStringLiteral("sendTransaction")), 1, 15000);
                soroban.setEndpoint(elsewhere.url());
                soroban.cancelAll();
                QCOMPARE(soroban.pending(), 1);
                QTRY_COMPARE_WITH_TIMEOUT(reportsOf(id).size(), 1, 15000);
                QCOMPARE(soroban.pending(), 0);
                const QVariantMap error = onlyFailureOf(id);
                QVERIFY2(!error.isEmpty(), send.first);
                QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("cancelled"));
                QVERIFY2(error.value(QStringLiteral("mayHaveReachedServer")).toBool(), send.first);
                QCOMPARE(error.value(QStringLiteral("hash")).toString(), hash);
                QCOMPARE(elsewhere.requests(), 0);
            }
        }

        // A simulation and a preparation on the wire: nothing, as documented.
        MethodRpcEndpoint rpc;
        rpc.hold(QStringLiteral("simulateTransaction"));
        rpc.answer(QStringLiteral("getHealth"),
                   "{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"result\":{\"status\":\"healthy\","
                   "\"latestLedger\":444,\"oldestLedger\":1,\"ledgerRetentionWindow\":17280}}");
        soroban.setEndpoint(rpc.url());
        QSignalSpy simulated(&soroban, &qstellar::SorobanQml::simulated);
        QSignalSpy prepared(&soroban, &qstellar::SorobanQml::prepared);
        QSignalSpy health(&soroban, &qstellar::SorobanQml::health);
        const int reportsBefore = sent.size() + failed.size();
        const QList<QPair<const char*, std::function<int()>>> others = {
            { "simulate", [&]() { return soroban.simulate(envelope); } },
            { "prepare", [&]() { return soroban.prepare(envelope, passphrase); } },
        };
        int simulations = 0;
        for (const auto& other : others) {
            other.second();
            ++simulations;
            QTRY_COMPARE_WITH_TIMEOUT(rpc.requests(QStringLiteral("simulateTransaction")), simulations, 15000);
            soroban.cancelAll();
            QVERIFY2(soroban.pending() == 0, other.first);
        }
        // Reports come in order: once this one's has, theirs would have.
        const int after = soroban.checkHealth();
        QVERIFY(health.wait(15000));
        QCOMPARE(health.first().at(0).toInt(), after);
        QCOMPARE(simulated.size(), 0);
        QCOMPARE(prepared.size(), 0);
        QCOMPARE(sent.size() + failed.size(), reportsBefore);
    }

    /**
     * A send that had already ended when cancelAll() came — its answer on the
     * way to this thread — reports that end, as it would have without the
     * cancel. Here the RPC took it (sent(), status "pending"): a wallet told
     * nothing would build it again. Deterministic: the endpoint runs on a
     * thread of its own and this thread takes no event from the send until
     * cancelAll(), so the send's report is still queued here; the request
     * behind it reaching the endpoint says the send has ended, since requests
     * run in order. Its report used to be dropped.
     */
    void testCancelAllStillReportsASendThatHadEnded()
    {
        QString hash;
        const QString envelope = sorobanEnvelope(&hash);
        QAtomicInt healthAsked(0);
        // The endpoint on its own thread, deleted there whatever happens here.
        struct EndpointThread
        {
            QThread thread;
            QObject* context = nullptr;
            MethodRpcEndpoint* rpc = nullptr;
            ~EndpointThread()
            {
                QMetaObject::invokeMethod(context, [this]() { delete rpc; }, Qt::BlockingQueuedConnection);
                thread.quit();
                thread.wait();
            }
        } endpoint;
        endpoint.context = new QObject;
        endpoint.context->moveToThread(&endpoint.thread);
        QObject::connect(&endpoint.thread, &QThread::finished, endpoint.context, &QObject::deleteLater);
        endpoint.thread.start();
        QString url;
        QMetaObject::invokeMethod(endpoint.context, [&]() {
            endpoint.rpc = new MethodRpcEndpoint;
            endpoint.rpc->answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), hash));
            endpoint.rpc->hold(QStringLiteral("getHealth"));
            endpoint.rpc->onRequest = [&healthAsked](const QString& method) {
                if (method == QLatin1String("getHealth"))
                    healthAsked.fetchAndAddOrdered(1);
            };
            url = endpoint.rpc->url();
        }, Qt::BlockingQueuedConnection);

        qstellar::SorobanQml soroban;
        QSignalSpy sent(&soroban, &qstellar::SorobanQml::sent);
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);
        soroban.setEndpoint(url);
        const int id = soroban.send(envelope);
        soroban.checkHealth();
        QElapsedTimer clock;
        clock.start();
        while (healthAsked.loadAcquire() == 0 && clock.elapsed() < 15000)
            QThread::msleep(1);                 // takes no event
        QCOMPARE(healthAsked.loadAcquire(), 1);
        QCOMPARE(sent.size(), 0);               // its report is queued here
        QCOMPARE(soroban.pending(), 2);
        soroban.cancelAll();
        QCOMPARE(soroban.pending(), 1);         // the send's; the health check's is forgotten
        QTRY_COMPARE_WITH_TIMEOUT(sent.size(), 1, 15000);
        QCOMPARE(sent.first().at(0).toInt(), id);
        QCOMPARE(sent.first().at(1).toMap().value(QStringLiteral("status")).toString(), QStringLiteral("pending"));
        QCOMPARE(sent.first().at(1).toMap().value(QStringLiteral("hash")).toString().toLower(), hash);
        QCOMPARE(soroban.pending(), 0);
        QTest::qWait(50);
        QCOMPARE(sent.size(), 1);
        QCOMPARE(failed.size(), 0);
    }

    /**
     * Destroying the facade while its worker is busy for longer than the
     * destructor waits (2 s) leaves the worker to finish on its own thread:
     * the destructor returns after its wait, with a warning; the busy step
     * runs to its end; the worker then leaves at once the event loop it was
     * waiting in (the request it waits for is still held: quit() ended the
     * loop), and is deleted on its thread with its network objects; and the
     * thread object is deleted afterwards on this thread. A new facade works.
     * The worker used to be terminated after the 2 s: the busy step never
     * finished, and its QNetworkAccessManager and the reply it waited for
     * were torn down from this thread, which a Qt 6 debug build aborts on
     * ("Cannot send events to objects owned by a different thread").
     *
     * The worker is made busy by a call queued to its thread while it waits
     * for a held request: the nested event loop it waits in delivers the
     * call, which then takes no event for 5 s.
     */
    void testDestroyingItWhileTheWorkerIsBusyNeverTerminatesIt()
    {
        HeldRpcEndpoint held(111, /*held*/ true);
        std::unique_ptr<qstellar::SorobanQml> soroban(new qstellar::SorobanQml);
        soroban->setEndpoint(held.url());
        soroban->checkHealth();
        QTRY_COMPARE_WITH_TIMEOUT(held.requests(), 1, 15000);   // the worker waits for it

        QThread* thread = soroban->findChild<QThread*>();
        QVERIFY(thread);
        const QPointer<QThread> threadAlive(thread);
        auto started = std::make_shared<QAtomicInt>(0);
        auto ended = std::make_shared<QAtomicInt>(0);
        QObject* busy = new QObject;
        busy->moveToThread(thread);
        QObject::connect(thread, &QThread::finished, busy, &QObject::deleteLater);
        QMetaObject::invokeMethod(busy, [started, ended]() {
            started->storeRelease(1);
            QThread::msleep(5000);              // takes no event
            ended->storeRelease(1);
        }, Qt::QueuedConnection);
        QTRY_COMPARE_WITH_TIMEOUT(started->loadAcquire(), 1, 15000);

        QTest::ignoreMessage(QtWarningMsg, "SorobanQml: the RPC worker did not stop within 2 s; "
                                           "it is left to finish on its own thread");
        QElapsedTimer clock;
        clock.start();
        soroban.reset();
        const qint64 took = clock.elapsed();
        QVERIFY2(took < 4500, qPrintable(QStringLiteral("the destructor took %1 ms").arg(took)));
        // Left to finish, not deleted with the facade (nor terminated).
        QVERIFY(!threadAlive.isNull());
        QTRY_COMPARE_WITH_TIMEOUT(ended->loadAcquire(), 1, 15000);   // the busy step ran to its end
        // Then the worker ended, well before the held request's own timeout,
        // and the thread object went.
        QTRY_VERIFY_WITH_TIMEOUT(threadAlive.isNull(), 15000);

        HeldRpcEndpoint ok(222, /*held*/ false);
        qstellar::SorobanQml again;
        again.setEndpoint(ok.url());
        QSignalSpy health(&again, &qstellar::SorobanQml::health);
        again.checkHealth();
        QVERIFY(health.wait(15000));
        QCOMPARE(health.first().at(1).toMap().value(QStringLiteral("latestLedger")).toInt(), 222);
    }

    // ─── Whether a failed request may have reached the server ────────

    /**
     * A failed send says whether it may have reached the server and the hash
     * to look it up by, with its network given or not. The map said neither:
     * a send whose connection never opened and one the server read before
     * the connection dropped looked the same, and nothing said what to look
     * up before sending again. Sent without its network, it then said
     * mayHaveReachedServer and no hash; this test expected exactly that
     * ("not known, and not guessed"), and now expects the same hash as the
     * send given its network: the endpoint's network is asked, not guessed.
     * (The endpoints here answer getNetwork as a real one does, and only
     * then fail the transaction.)
     */
    void testAFailedSendSaysWhetherItMayHaveReachedTheServer()
    {
        const QString passphrase = Network::testnetNetwork()->getNetworkPassphrase();
        QString hash;
        const QString envelope = sorobanEnvelope(&hash);
        qstellar::SorobanQml soroban;
        const QList<QPair<const char*, std::function<int()>>> sends = {
            { "given its network", [&]() { return soroban.send(envelope, passphrase); } },
            { "without it", [&]() { return soroban.send(envelope); } },
        };

        // A connection that never opens: known not to have left.
        QString closed;
        {
            QTcpServer probe;
            probe.listen(QHostAddress::LocalHost, 0);
            closed = QStringLiteral("http://127.0.0.1:%1/").arg(probe.serverPort());
        }
        soroban.setEndpoint(closed);
        for (const auto& send : sends) {
            const QVariantMap error = failureOf(soroban, send.second);
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("connection"));
            QVERIFY2(error.contains(QStringLiteral("mayHaveReachedServer")),
                     qPrintable(error.value(QStringLiteral("message")).toString()));
            QVERIFY2(!error.value(QStringLiteral("mayHaveReachedServer")).toBool(), send.first);
            QVERIFY2(!error.contains(QStringLiteral("hash")), send.first);
        }

        // Read by the server, then the connection dropped: it may be applied.
        MethodRpcEndpoint hangingUp;
        hangingUp.hangUp(QStringLiteral("sendTransaction"));
        soroban.setEndpoint(hangingUp.url());
        for (const auto& send : sends) {
            const QVariantMap error = failureOf(soroban, send.second);
            QVERIFY2(error.value(QStringLiteral("kind")).toString() == QStringLiteral("connection"),
                     qPrintable(error.value(QStringLiteral("message")).toString()));
            QVERIFY2(error.value(QStringLiteral("mayHaveReachedServer")).toBool(), send.first);
            QCOMPARE(error.value(QStringLiteral("hash")).toString(), hash);
            QVERIFY2(!error.contains(QStringLiteral("rpcHash")), send.first);
        }
        QVERIFY(hangingUp.requests(QStringLiteral("sendTransaction")) >= 2);   // Qt may retry one

        // An RPC error answer: the server had it.
        MethodRpcEndpoint rpc;
        rpc.answer(QStringLiteral("sendTransaction"),
                   "{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"error\":{\"code\":-32603,"
                   "\"message\":\"could not submit transaction to stellar-core\"}}");
        soroban.setEndpoint(rpc.url());
        for (const auto& send : sends) {
            const QVariantMap error = failureOf(soroban, send.second);
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("rpc"));
            QVERIFY2(error.value(QStringLiteral("mayHaveReachedServer")).toBool(), send.first);
            QCOMPARE(error.value(QStringLiteral("hash")).toString(), hash);
        }
    }

    /**
     * The RPC read the sendTransaction whole and its connection closed with
     * no answer, so it may have taken the transaction in; Qt 5 sends it
     * again, by itself, and the RPC answers that later send. ERROR (with
     * txBAD_SEQ, what the RPC answers a transaction sent again once the first
     * has been applied) or TRY_AGAIN_LATER answers the later send only: the
     * send fails, kind "rpc", with mayHaveReachedServer true, the hash,
     * `resent`, and a message that starts "not final" and names the hash, so
     * the wallet looks the transaction up before anything else. It was sent(),
     * status "error" or "tryAgainLater", as the RPC's final word, and a wallet
     * that built the payment again with a fresh sequence number paid twice.
     * PENDING says the RPC has it: sent(), as ever. A cancel while the later
     * send is on the wire says `resent` too. Qt 6 does not send it again: the
     * failure is then "connection", with the hash, and not `resent`.
     */
    void testAnAnswerToASendQtSentAgainIsNotFinal_data()
    {
        QTest::addColumn<QString>("answer");
        QTest::newRow("ERROR") << QStringLiteral("ERROR");
        QTest::newRow("TRY_AGAIN_LATER") << QStringLiteral("TRY_AGAIN_LATER");
        QTest::newRow("PENDING") << QStringLiteral("PENDING");
        QTest::newRow("cancel") << QStringLiteral("cancel");
    }
    void testAnAnswerToASendQtSentAgainIsNotFinal()
    {
        QFETCH(QString, answer);
        const QString passphrase = Network::TESTNET_NETWORK_PASSPHRASE();
        QString hash;
        const QString envelope = sorobanEnvelope(&hash);
        const QString badSeq = QStringLiteral("AAAAAAAAAGT////7AAAAAA==");      // txBAD_SEQ
        const bool cancel = answer == QLatin1String("cancel");
        MethodRpcEndpoint rpc;
        rpc.hangUpFirst(QStringLiteral("sendTransaction"), 1);
        if (cancel)
            rpc.hold(QStringLiteral("sendTransaction"));    // the later send, read whole and held
        else
            rpc.answer(QStringLiteral("sendTransaction"),
                       sendAnswer(answer, hash, answer == QLatin1String("ERROR") ? badSeq : QString()));
        qstellar::SorobanQml soroban;
        soroban.setEndpoint(rpc.url());
        QSignalSpy sent(&soroban, &qstellar::SorobanQml::sent);
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);
        const int id = soroban.send(envelope, passphrase);
        if (cancel) {
            QTRY_VERIFY_WITH_TIMEOUT(rpc.requests(QStringLiteral("sendTransaction")) == 2
                                         || failed.size() + sent.size() > 0, 20000);
            if (rpc.requests(QStringLiteral("sendTransaction")) == 2) {
                QTest::qWait(100);      // what the connection said it wrote, delivered
                QCOMPARE(failed.size() + sent.size(), 0);
                soroban.cancelAll();
            }
        }
        QTRY_COMPARE_WITH_TIMEOUT(failed.size() + sent.size(), 1, 20000);
        QTest::qWait(200);
        QCOMPARE(failed.size() + sent.size(), 1);
        QCOMPARE(soroban.pending(), 0);

        const int sends = rpc.requests(QStringLiteral("sendTransaction"));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QCOMPARE(sends, 2);             // Qt 5 sends it again, and the later send is answered
#endif
        QCOMPARE(rpc.lastParams(QStringLiteral("sendTransaction")).toObject()
                     .value(QStringLiteral("transaction")).toString(), envelope);
        if (sends == 1) {
            // Not sent again: the connection closed on it, and it may land.
            QCOMPARE(failed.size(), 1);
            const QVariantMap error = failed.first().at(1).toMap();
            qInfo("sent once, not again: %s",
                  qPrintable(error.value(QStringLiteral("message")).toString()));
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("connection"));
            QCOMPARE(error.value(QStringLiteral("mayHaveReachedServer")), QVariant(true));
            QCOMPARE(error.value(QStringLiteral("hash")).toString(), hash);
            QVERIFY(!error.contains(QStringLiteral("resent")));
            return;
        }
        QCOMPARE(sends, 2);

        if (answer == QLatin1String("PENDING")) {
            // The RPC says it has the transaction: that holds of the first send too.
            QCOMPARE(failed.size(), 0);
            QCOMPARE(sent.first().at(0).toInt(), id);
            const QVariantMap result = sent.first().at(1).toMap();
            QCOMPARE(result.value(QStringLiteral("status")).toString(), QStringLiteral("pending"));
            QCOMPARE(result.value(QStringLiteral("hash")).toString(), hash);
            return;
        }

        const QVariantMap result = sent.isEmpty() ? QVariantMap() : sent.first().at(1).toMap();
        QVERIFY2(failed.size() == 1,
                 qPrintable(QStringLiteral("the RPC read the first send whole and answered Qt's second, "
                                           "yet it was sent(), status %1")
                                .arg(result.value(QStringLiteral("status")).toString())));
        QCOMPARE(failed.first().at(0).toInt(), id);
        const QVariantMap error = failed.first().at(1).toMap();
        const QString message = error.value(QStringLiteral("message")).toString();
        QVERIFY2(error.value(QStringLiteral("resent")) == QVariant(true), qPrintable(message));
        QCOMPARE(error.value(QStringLiteral("mayHaveReachedServer")), QVariant(true));
        QCOMPARE(error.value(QStringLiteral("hash")).toString(), hash);
        QVERIFY2(message.startsWith(QLatin1String("not final: "))
                     && message.contains(QStringLiteral("by its hash, ") + hash),
                 qPrintable(message));
        if (cancel) {
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("cancelled"));
            QVERIFY2(message.contains(QLatin1String("cancelled while it was on the wire")),
                     qPrintable(message));
            return;
        }
        QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("rpc"));
        QVERIFY2(message.contains(QStringLiteral("with ") + answer), qPrintable(message));
        if (answer == QLatin1String("ERROR"))
            QVERIFY2(message.contains(badSeq), qPrintable(message));
    }

    /**
     * Cancelled and refused requests say whether they may have reached the
     * server: the send on the wire when the endpoint changed may have (with
     * its hash), the one queued behind it was never sent, and a request
     * refused before it was sent — an envelope that does not decode, an empty
     * passphrase, an empty simulation, a signed envelope to prepare — never
     * left. Sent with its network, a send whose hash cannot be worked out is
     * not sent at all. (The first endpoint answers getNetwork, as a real one
     * does, and holds the transaction.)
     */
    void testCancelledAndRefusedRequestsSayWhetherTheyMayHaveReachedTheServer()
    {
        const QString passphrase = Network::testnetNetwork()->getNetworkPassphrase();
        QString hash;
        const QString envelope = sorobanEnvelope(&hash);
        MethodRpcEndpoint first;
        first.hold(QStringLiteral("sendTransaction"));
        HeldRpcEndpoint second(222, /*held*/ false);

        qstellar::SorobanQml soroban;
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);
        soroban.setEndpoint(first.url());
        const int onTheWire = soroban.send(envelope, passphrase);
        QTRY_COMPARE_WITH_TIMEOUT(first.requests(QStringLiteral("sendTransaction")), 1, 15000);
        const int queued = soroban.send(envelope, passphrase);
        soroban.setEndpoint(second.url());
        QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 2, 15000);
        QVariantMap wire, behind;
        for (const QList<QVariant>& args : failed) {
            QVERIFY(args.at(0).toInt() == onTheWire || args.at(0).toInt() == queued);
            (args.at(0).toInt() == onTheWire ? wire : behind) = args.at(1).toMap();
        }
        QCOMPARE(wire.value(QStringLiteral("kind")).toString(), QStringLiteral("cancelled"));
        QCOMPARE(wire.value(QStringLiteral("mayHaveReachedServer")).toBool(), true);
        QCOMPARE(wire.value(QStringLiteral("hash")).toString(), hash);
        QCOMPARE(behind.value(QStringLiteral("kind")).toString(), QStringLiteral("cancelled"));
        QVERIFY(behind.contains(QStringLiteral("mayHaveReachedServer")));
        QCOMPARE(behind.value(QStringLiteral("mayHaveReachedServer")).toBool(), false);
        QVERIFY(!behind.contains(QStringLiteral("hash")));
        QCOMPARE(first.requests(QStringLiteral("sendTransaction")), 1);

        QString signedHash;
        const QString signedEnvelope = sorobanEnvelope(&signedHash, /*sign*/ true);
        const QList<QPair<const char*, std::function<int()>>> refused = {
            { "not an envelope", [&]() { return soroban.send(QStringLiteral("AAAAAgAAAAA="), passphrase); } },
            { "no passphrase", [&]() { return soroban.send(envelope, QString()); } },
            { "nothing to simulate", [&]() { return soroban.simulate(QString()); } },
            { "a signed envelope to prepare", [&]() { return soroban.prepare(signedEnvelope, passphrase); } },
        };
        for (const auto& r : refused) {
            const QVariantMap error = failureOf(soroban, r.second);
            QVERIFY2(!error.isEmpty(), r.first);
            QVERIFY2(error.contains(QStringLiteral("mayHaveReachedServer")), r.first);
            QVERIFY2(!error.value(QStringLiteral("mayHaveReachedServer")).toBool(), r.first);
            QVERIFY2(!error.contains(QStringLiteral("hash")), r.first);
        }
        QCOMPARE(second.requests(), 0);                 // none of them was sent
        QCOMPARE(failureOf(soroban, refused.first().second).value(QStringLiteral("kind")).toString(),
                 QStringLiteral("validation"));
    }

    /**
     * An answer with another hash than the one worked out for the envelope
     * is refused, as not about it — and it carries the RPC's hash too, as
     * `rpcHash`, the one the message says can land: the RPC works the hash
     * out on the network it submits to. The failure used to carry only
     * `hash`, so a caller looking that up got NOT_FOUND for a transaction
     * the RPC may have applied under its own, and could pay again. The next
     * send asks the endpoint's network again, since the answer contradicted
     * it. The right hash is taken whatever its case, with or without the
     * network given.
     */
    void testASendAnsweredForAnotherTransactionFails()
    {
        const QString passphrase = Network::testnetNetwork()->getNetworkPassphrase();
        QString hash;
        const QString envelope = sorobanEnvelope(&hash);
        const QString other = QStringLiteral("f92787696aef6d181fb7f703fc115f00eff3398c1b54129a2e365afa6c18dea0");
        MethodRpcEndpoint rpc;
        qstellar::SorobanQml soroban;
        soroban.setEndpoint(rpc.url());

        rpc.answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), other.toUpper()));
        const QList<QPair<const char*, std::function<int()>>> sends = {
            { "given its network", [&]() { return soroban.send(envelope, passphrase); } },
            { "without it", [&]() { return soroban.send(envelope); } },
        };
        int asked = 0;
        for (const auto& send : sends) {
            const QVariantMap error = failureOf(soroban, send.second);
            const QString message = error.value(QStringLiteral("message")).toString();
            QVERIFY2(error.value(QStringLiteral("kind")).toString() == QStringLiteral("rpc"), qPrintable(message));
            QVERIFY2(error.value(QStringLiteral("mayHaveReachedServer")).toBool(), send.first);
            QCOMPARE(error.value(QStringLiteral("hash")).toString(), hash);
            QCOMPARE(error.value(QStringLiteral("rpcHash")).toString(), other);
            QVERIFY2(message.contains(hash), qPrintable(message));
            QVERIFY2(message.contains(other + QStringLiteral(" is the one it can land under")),
                     qPrintable(message));
            // Contradicted, the endpoint's network is asked again each time.
            QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), ++asked);
        }

        rpc.answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), hash.toUpper()));
        for (const auto& send : sends) {
            const QPair<QString, QVariantMap> outcome = outcomeOf(soroban, send.second);
            QVERIFY2(outcome.first == QStringLiteral("sent"),
                     qPrintable(outcome.second.value(QStringLiteral("message")).toString()));
            QCOMPARE(outcome.second.value(QStringLiteral("status")).toString(), QStringLiteral("pending"));
            QVERIFY(!outcome.second.contains(QStringLiteral("rpcHash")));
        }
        QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), asked + 1);   // learned once more, then kept
    }

    // ─── Which network the endpoint serves ───────────────────────────

    /**
     * A send asks the endpoint which network it serves before posting, once
     * per endpoint: again after an endpoint change, even back to the same
     * URL. No other request asks it. (Nothing asked it: the one-argument
     * send had no hash to report, and the two-argument one trusted the
     * passphrase it was given.)
     */
    void testASendLearnsTheEndpointsNetworkOncePerEndpoint()
    {
        QString hash;
        const QString envelope = sorobanEnvelope(&hash);
        const QString passphrase = Network::TESTNET_NETWORK_PASSPHRASE();
        MethodRpcEndpoint a;
        MethodRpcEndpoint b;
        a.answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), hash));
        b.answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), hash));
        a.answer(QStringLiteral("getHealth"),
                 "{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"result\":{\"status\":\"healthy\","
                 "\"latestLedger\":111,\"oldestLedger\":1,\"ledgerRetentionWindow\":17280}}");
        qstellar::SorobanQml soroban;
        soroban.setEndpoint(a.url());
        auto sendOk = [&](std::function<int()> start) {
            const QPair<QString, QVariantMap> outcome = outcomeOf(soroban, start);
            return outcome.first == QStringLiteral("sent")
                && outcome.second.value(QStringLiteral("hash")).toString() == hash;
        };

        QVERIFY(sendOk([&]() { return soroban.send(envelope); }));
        QCOMPARE(a.requests(QStringLiteral("getNetwork")), 1);
        QVERIFY(sendOk([&]() { return soroban.send(envelope, passphrase); }));
        QVERIFY(sendOk([&]() { return soroban.send(envelope); }));
        QCOMPARE(a.requests(QStringLiteral("getNetwork")), 1);
        QCOMPARE(a.requests(QStringLiteral("sendTransaction")), 3);

        soroban.setEndpoint(b.url());
        QVERIFY(sendOk([&]() { return soroban.send(envelope, passphrase); }));
        QCOMPARE(b.requests(QStringLiteral("getNetwork")), 1);
        soroban.setEndpoint(a.url());
        QVERIFY(sendOk([&]() { return soroban.send(envelope); }));
        QCOMPARE(a.requests(QStringLiteral("getNetwork")), 2);
        // Back and forth with nothing sent in between: still asked again.
        soroban.setEndpoint(b.url());
        soroban.setEndpoint(a.url());
        QVERIFY(sendOk([&]() { return soroban.send(envelope, passphrase); }));
        QCOMPARE(a.requests(QStringLiteral("getNetwork")), 3);

        // The other requests are as they were: none asks the network.
        QSignalSpy health(&soroban, &qstellar::SorobanQml::health);
        soroban.checkHealth();
        QVERIFY(health.wait(15000));
        QCOMPARE(health.first().at(1).toMap().value(QStringLiteral("latestLedger")).toInt(), 111);
        (void)failureOf(soroban, [&]() { return soroban.simulate(envelope); });
        (void)failureOf(soroban, [&]() { return soroban.prepare(envelope, passphrase); });
        QCOMPARE(a.requests(QStringLiteral("simulateTransaction")), 2);
        QCOMPARE(a.requests(QStringLiteral("getNetwork")), 3);
    }

    /**
     * Given a passphrase that is not the endpoint's network, a send is
     * refused before the transaction is posted. It was sent, and a failure
     * after that pointed at the hash worked out from the given passphrase —
     * one the endpoint's network never gives it, so a wallet looking it up
     * kept getting NOT_FOUND while the RPC may have applied it under its own.
     */
    void testASendGivenAnotherNetworkThanTheEndpointsIsNotSent()
    {
        QString hash;
        const QString envelope = sorobanEnvelope(&hash);
        qstellar::SorobanQml soroban;
        const QList<QPair<QByteArray, QString>> cases = {
            { MethodRpcEndpoint::testnetNetwork(), Network::PUBLIC_NETWORK_PASSPHRASE() },
            { MethodRpcEndpoint::mainnetNetwork(), Network::TESTNET_NETWORK_PASSPHRASE() },
            // Exactly the passphrase: another byte is another network.
            { MethodRpcEndpoint::testnetNetwork(), Network::TESTNET_NETWORK_PASSPHRASE() + QLatin1Char(' ') },
        };
        for (const auto& c : cases) {
            MethodRpcEndpoint rpc;
            rpc.answer(QStringLiteral("getNetwork"), c.first);
            rpc.answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), hash));
            soroban.setEndpoint(rpc.url());
            const QVariantMap error = failureOf(soroban, [&]() { return soroban.send(envelope, c.second); });
            const QString message = error.value(QStringLiteral("message")).toString();
            QVERIFY2(error.value(QStringLiteral("kind")).toString() == QStringLiteral("validation"),
                     qPrintable(message));
            QVERIFY(error.contains(QStringLiteral("mayHaveReachedServer")));
            QVERIFY(!error.value(QStringLiteral("mayHaveReachedServer")).toBool());
            QVERIFY(!error.contains(QStringLiteral("hash")));
            QVERIFY2(message.contains(QStringLiteral("\"") + c.second + QStringLiteral("\"")),
                     qPrintable(message));
            QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), 1);
            QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), 0);
        }
    }

    /**
     * Without its network, a send is hashed on the network the endpoint
     * serves — the hash the transaction is applied under there — and a
     * failure that may have reached the server carries it, as the
     * two-argument send's does. Here a testnet envelope goes to a mainnet
     * endpoint: its hash there is the mainnet one. (It carried no hash.)
     */
    void testASendWithoutItsNetworkIsHashedOnTheEndpointsNetwork()
    {
        QString testnetHash;
        const QString envelope = sorobanEnvelope(&testnetHash);
        const QString mainnetHash = hashOn(envelope, Network::PUBLIC_NETWORK_PASSPHRASE());
        QVERIFY(mainnetHash != testnetHash);
        MethodRpcEndpoint rpc;
        rpc.answer(QStringLiteral("getNetwork"), MethodRpcEndpoint::mainnetNetwork());
        rpc.hangUp(QStringLiteral("sendTransaction"));
        qstellar::SorobanQml soroban;
        soroban.setEndpoint(rpc.url());

        const QVariantMap error = failureOf(soroban, [&]() { return soroban.send(envelope); });
        QVERIFY2(error.value(QStringLiteral("mayHaveReachedServer")).toBool(),
                 qPrintable(error.value(QStringLiteral("message")).toString()));
        QCOMPARE(error.value(QStringLiteral("hash")).toString(), mainnetHash);
        const int posted = rpc.requests(QStringLiteral("sendTransaction"));
        QVERIFY(posted >= 1);

        // The testnet passphrase is refused there, once the endpoint has been
        // asked again (the answer it differs from was remembered, not fresh);
        // the mainnet one is taken on the answer that refusal fetched.
        QCOMPARE(failureOf(soroban, [&]() {
                     return soroban.send(envelope, Network::TESTNET_NETWORK_PASSPHRASE()); })
                     .value(QStringLiteral("kind")).toString(), QStringLiteral("validation"));
        QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), posted);
        QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), 2);
        QCOMPARE(failureOf(soroban, [&]() {
                     return soroban.send(envelope, Network::PUBLIC_NETWORK_PASSPHRASE()); })
                     .value(QStringLiteral("hash")).toString(), mainnetHash);
        QVERIFY(rpc.requests(QStringLiteral("sendTransaction")) > posted);
        QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), 2);
    }

    /**
     * When the endpoint's network cannot be read, nothing is sent: the send
     * fails with the lookup's kind, mayHaveReachedServer false and no hash.
     * An envelope that does not decode is not sent without its network
     * either. (The one-argument send posted both as they were.)
     */
    void testASendWhoseNetworkCannotBeReadIsNotSent()
    {
        QString hash;
        const QString envelope = sorobanEnvelope(&hash);
        const QString passphrase = Network::TESTNET_NETWORK_PASSPHRASE();
        qstellar::SorobanQml soroban;
        const QList<QPair<const char*, std::function<int()>>> sends = {
            { "given its network", [&]() { return soroban.send(envelope, passphrase); } },
            { "without it", [&]() { return soroban.send(envelope); } },
        };
        const QList<QPair<QByteArray, QString>> lookups = {
            // What an RPC without the method answers.
            { "{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"error\":{\"code\":-32601,\"message\":\"method not found\"}}",
              QStringLiteral("rpc") },
            // An answer that names no network.
            { "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"protocolVersion\":28}}", QStringLiteral("rpc") },
        };
        for (const auto& lookup : lookups) {
            MethodRpcEndpoint rpc;
            rpc.answer(QStringLiteral("getNetwork"), lookup.first);
            rpc.answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), hash));
            soroban.setEndpoint(rpc.url());
            for (const auto& send : sends) {
                const QVariantMap error = failureOf(soroban, send.second);
                const QString message = error.value(QStringLiteral("message")).toString();
                QVERIFY2(error.value(QStringLiteral("kind")).toString() == lookup.second, qPrintable(message));
                QVERIFY2(message.startsWith(QStringLiteral("not sent")), qPrintable(message));
                QVERIFY(error.contains(QStringLiteral("mayHaveReachedServer")));
                QVERIFY2(!error.value(QStringLiteral("mayHaveReachedServer")).toBool(), send.first);
                QVERIFY2(!error.contains(QStringLiteral("hash")), send.first);
            }
            QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), 2);
            QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), 0);
        }

        // Read by the server, and the connection dropped before it answered.
        MethodRpcEndpoint hangingUp;
        hangingUp.hangUp(QStringLiteral("getNetwork"));
        hangingUp.answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), hash));
        soroban.setEndpoint(hangingUp.url());
        for (const auto& send : sends) {
            const QVariantMap error = failureOf(soroban, send.second);
            QVERIFY2(error.value(QStringLiteral("kind")).toString() == QStringLiteral("connection"),
                     qPrintable(error.value(QStringLiteral("message")).toString()));
            QVERIFY2(!error.value(QStringLiteral("mayHaveReachedServer")).toBool(), send.first);
            QVERIFY2(!error.contains(QStringLiteral("hash")), send.first);
        }
        QVERIFY(hangingUp.requests(QStringLiteral("getNetwork")) >= 2);
        QCOMPARE(hangingUp.requests(QStringLiteral("sendTransaction")), 0);

        // Without its network, an envelope that does not decode: not even
        // the network is asked.
        MethodRpcEndpoint rpc;
        soroban.setEndpoint(rpc.url());
        const QVariantMap error = failureOf(soroban, [&]() { return soroban.send(QStringLiteral("AAAAAgAAAAA=")); });
        QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("validation"));
        QVERIFY(!error.value(QStringLiteral("mayHaveReachedServer")).toBool());
        QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), 0);
        QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), 0);
    }

    /**
     * A send cancelled while the endpoint's network is being asked is not
     * sent, and says so: cancelled before it was sent, mayHaveReachedServer
     * false, no hash. Taken for the send's own abort it would say the
     * transaction may have reached the server.
     */
    void testASendCancelledDuringTheNetworkLookupIsNotSent()
    {
        QString hash;
        const QString envelope = sorobanEnvelope(&hash);
        const QString passphrase = Network::TESTNET_NETWORK_PASSPHRASE();
        HeldRpcEndpoint elsewhere(222, /*held*/ false);
        qstellar::SorobanQml soroban;
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);
        const QList<QPair<const char*, std::function<int()>>> sends = {
            { "given its network", [&]() { return soroban.send(envelope, passphrase); } },
            { "without it", [&]() { return soroban.send(envelope); } },
        };
        for (const auto& send : sends) {
            MethodRpcEndpoint rpc;
            rpc.hold(QStringLiteral("getNetwork"));
            rpc.answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), hash));
            soroban.setEndpoint(rpc.url());
            failed.clear();
            const int id = send.second();
            QTRY_COMPARE_WITH_TIMEOUT(rpc.requests(QStringLiteral("getNetwork")), 1, 15000);
            soroban.setEndpoint(elsewhere.url());
            QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 1, 15000);
            QCOMPARE(failed.first().at(0).toInt(), id);
            const QVariantMap error = failed.first().at(1).toMap();
            QCOMPARE(error.value(QStringLiteral("kind")).toString(), QStringLiteral("cancelled"));
            QCOMPARE(error.value(QStringLiteral("message")).toString(),
                     QStringLiteral("cancelled before it was sent"));
            QVERIFY2(!error.value(QStringLiteral("mayHaveReachedServer")).toBool(), send.first);
            QVERIFY2(!error.contains(QStringLiteral("hash")), send.first);
            QTest::qWait(50);
            QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), 0);
        }

        // cancelAll() the same: nothing is posted, and the worker is free.
        MethodRpcEndpoint rpc;
        rpc.hold(QStringLiteral("getNetwork"));
        soroban.setEndpoint(rpc.url());
        soroban.send(envelope);
        QTRY_COMPARE_WITH_TIMEOUT(rpc.requests(QStringLiteral("getNetwork")), 1, 15000);
        soroban.cancelAll();
        rpc.answer(QStringLiteral("getHealth"),
                   "{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"result\":{\"status\":\"healthy\","
                   "\"latestLedger\":333,\"oldestLedger\":1,\"ledgerRetentionWindow\":17280}}");
        QSignalSpy health(&soroban, &qstellar::SorobanQml::health);
        soroban.checkHealth();
        QVERIFY(health.wait(15000));
        QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), 0);
    }

    /**
     * A cancel that lands just as the network lookup ends — delivered in the
     * batch of events that finished it, with nothing left to abort — still
     * keeps the transaction from being posted. Without the check after the
     * lookup the send went out after the cancel had been spent on the
     * lookup, and against an endpoint that holds it, it hung until its
     * timeout. The endpoint changes at a spread of moments after it answers
     * the lookup, so some of them land in that batch; every send must end,
     * promptly, as cancelled, and one that says it was not sent must not
     * have been.
     */
    void testACancelAsTheNetworkLookupEndsIsNotLost()
    {
        QString hash;
        const QString envelope = sorobanEnvelope(&hash);
        HeldRpcEndpoint elsewhere(222, /*held*/ false);
        MethodRpcEndpoint rpc;
        rpc.hold(QStringLiteral("sendTransaction"));
        qstellar::SorobanQml soroban;

        qint64 delay = 0;                               // nanoseconds
        rpc.onRequest = [&](const QString& method) {
            if (method != QLatin1String("getNetwork"))
                return;
            QElapsedTimer spin;
            spin.start();
            while (spin.nsecsElapsed() < delay) {}
            soroban.setEndpoint(elsewhere.url());
        };
        int notSent = 0;
        for (int step = 0; step < 120; ++step) {
            delay = qint64(step % 60) * 10000;          // 0 to 590 microseconds
            soroban.setEndpoint(rpc.url());
            const int postedBefore = rpc.requests(QStringLiteral("sendTransaction"));
            const QPair<QString, QVariantMap> outcome =
                outcomeOf(soroban, [&]() { return soroban.send(envelope); }, 5000);
            QVERIFY2(outcome.first == QStringLiteral("failed"),
                     qPrintable(QStringLiteral("step %1 ended as %2").arg(step).arg(outcome.first)));
            QCOMPARE(outcome.second.value(QStringLiteral("kind")).toString(), QStringLiteral("cancelled"));
            if (!outcome.second.value(QStringLiteral("mayHaveReachedServer")).toBool()) {
                ++notSent;
                QTest::qWait(20);
                QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), postedBefore);
            }
        }
        QVERIFY(notSent > 0);
    }

    /**
     * The RPC behind the same URL can come to serve another network, as a
     * local quickstart restarted with another network does. A passphrase that
     * differs from the answer remembered from an earlier send has the
     * endpoint asked again first, and is refused only when that fresh answer
     * disagrees too. The remembered answer was kept: the right passphrase was
     * refused for as long as the endpoint stayed set, and the network was
     * never asked again, since only a posted send that came back with another
     * hash dropped it and a refused send is never posted.
     */
    void testASendIsRefusedOnAFreshAnswerOnly()
    {
        QString testnetHash;
        const QString envelope = sorobanEnvelope(&testnetHash);
        const QString testnet = Network::TESTNET_NETWORK_PASSPHRASE();
        const QString pubnet = Network::PUBLIC_NETWORK_PASSPHRASE();
        const QString mainnetHash = hashOn(envelope, pubnet);
        QVERIFY(mainnetHash != testnetHash);
        MethodRpcEndpoint rpc;
        rpc.answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), testnetHash));
        qstellar::SorobanQml soroban;
        soroban.setEndpoint(rpc.url());
        auto sendWith = [&](const QString& passphrase) {
            return outcomeOf(soroban, [&]() {
                return passphrase.isEmpty() ? soroban.send(envelope) : soroban.send(envelope, passphrase);
            });
        };
        auto expectSent = [&](const QString& passphrase, const QString& hash) {
            const QPair<QString, QVariantMap> outcome = sendWith(passphrase);
            const QString what = QStringLiteral("%1 with \"%2\": %3").arg(outcome.first, passphrase,
                outcome.second.value(QStringLiteral("message")).toString());
            return outcome.first == QStringLiteral("sent")
                       && outcome.second.value(QStringLiteral("hash")).toString() == hash
                ? QString() : what;
        };
        auto expectNotSent = [&](const QString& passphrase, const QString& kind) {
            const QPair<QString, QVariantMap> outcome = sendWith(passphrase);
            const QVariantMap& error = outcome.second;
            const QString message = error.value(QStringLiteral("message")).toString();
            const QString what = QStringLiteral("%1 with \"%2\": %3 %4").arg(outcome.first, passphrase,
                error.value(QStringLiteral("kind")).toString(), message);
            return outcome.first == QStringLiteral("failed")
                       && error.value(QStringLiteral("kind")).toString() == kind
                       && message.startsWith(QStringLiteral("not sent"))
                       && error.contains(QStringLiteral("mayHaveReachedServer"))
                       && !error.value(QStringLiteral("mayHaveReachedServer")).toBool()
                       && !error.contains(QStringLiteral("hash"))
                ? QString() : what;
        };

        QString problem = expectSent(testnet, testnetHash);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), 1);

        // Restarted on mainnet, same URL.
        rpc.answer(QStringLiteral("getNetwork"), MethodRpcEndpoint::mainnetNetwork());
        rpc.answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), mainnetHash));
        problem = expectSent(pubnet, mainnetHash);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), 2);
        // The fresh answer is kept, for both overloads.
        problem = expectSent(pubnet, mainnetHash);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        problem = expectSent(QString(), mainnetHash);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), 2);
        QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), 4);

        // Now the testnet passphrase is the wrong one: refused, each time on
        // an answer asked for that send, and never posted.
        for (int asked = 3; asked <= 4; ++asked) {
            problem = expectNotSent(testnet, QStringLiteral("validation"));
            QVERIFY2(problem.isEmpty(), qPrintable(problem));
            QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), asked);
        }
        const QVariantMap refused = sendWith(testnet).second;
        QVERIFY2(refused.value(QStringLiteral("message")).toString()
                     .contains(QStringLiteral("serves the network \"") + pubnet + QStringLiteral("\"")),
                 qPrintable(refused.value(QStringLiteral("message")).toString()));
        QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), 5);
        QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), 4);

        // Asked again and not readable: not sent, with the lookup's kind (not
        // refused on the remembered answer), and nothing is remembered, so
        // the next send asks once more.
        rpc.answer(QStringLiteral("getNetwork"),
                   "{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"error\":{\"code\":-32601,\"message\":\"method not found\"}}");
        problem = expectNotSent(testnet, QStringLiteral("rpc"));
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), 6);
        rpc.answer(QStringLiteral("getNetwork"), MethodRpcEndpoint::mainnetNetwork());
        problem = expectSent(pubnet, mainnetHash);
        QVERIFY2(problem.isEmpty(), qPrintable(problem));
        QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), 7);
        QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), 5);

        // A cancel while it is asked again: cancelled before it was sent.
        HeldRpcEndpoint elsewhere(222, /*held*/ false);
        rpc.hold(QStringLiteral("getNetwork"));
        QSignalSpy failed(&soroban, &qstellar::SorobanQml::failed);
        const int id = soroban.send(envelope, testnet);
        QTRY_COMPARE_WITH_TIMEOUT(rpc.requests(QStringLiteral("getNetwork")), 8, 15000);
        soroban.setEndpoint(elsewhere.url());
        QTRY_COMPARE_WITH_TIMEOUT(failed.size(), 1, 15000);
        QCOMPARE(failed.first().at(0).toInt(), id);
        const QVariantMap cancelled = failed.first().at(1).toMap();
        QCOMPARE(cancelled.value(QStringLiteral("kind")).toString(), QStringLiteral("cancelled"));
        QCOMPARE(cancelled.value(QStringLiteral("message")).toString(),
                 QStringLiteral("cancelled before it was sent"));
        QVERIFY(!cancelled.value(QStringLiteral("mayHaveReachedServer")).toBool());
        QVERIFY(!cancelled.contains(QStringLiteral("hash")));
        QTest::qWait(50);
        QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), 5);
        QCOMPARE(elsewhere.requests(), 0);
    }

    /**
     * An envelope that does not decode exactly is refused before any
     * request, with its network given or not, whether the endpoint can be
     * reached or not: kind "validation", mayHaveReachedServer false, no hash.
     * Without its network it used to be read only after the network lookup,
     * so it cost a getNetwork request, and against an endpoint that could not
     * be reached it failed as "connection", where the doc says "validation".
     */
    void testAnEnvelopeThatDoesNotDecodeCostsNoRequest()
    {
        const QString passphrase = Network::TESTNET_NETWORK_PASSPHRASE();
        QString hash;
        const QString envelope = sorobanEnvelope(&hash);
        const QByteArray raw = QByteArray::fromBase64(envelope.toLatin1());
        const QList<QPair<const char*, QString>> envelopes = {
            { "not an envelope", QStringLiteral("AAAAAgAAAAA=") },
            { "not base64", QStringLiteral("AAAA!AgAAAAA=") },
            { "cut short", QString::fromLatin1(raw.left(raw.size() - 12).toBase64()) },
            { "followed by other bytes", QString::fromLatin1((raw + QByteArray(8, '\0')).toBase64()) },
        };
        // Each is refused by the SDK's own reading, the one a send makes.
        for (const auto& e : envelopes) {
            bool threw = false;
            try {
                QScopedPointer<AbstractTransaction> read(
                    AbstractTransaction::fromEnvelopeXdr(e.second, Network::testnetNetwork()));
            } catch (const std::exception&) {
                threw = true;
            }
            QVERIFY2(threw, e.first);
        }

        QString closed;
        {
            QTcpServer probe;
            probe.listen(QHostAddress::LocalHost, 0);
            closed = QStringLiteral("http://127.0.0.1:%1/").arg(probe.serverPort());
        }
        MethodRpcEndpoint rpc;
        rpc.answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), hash));
        qstellar::SorobanQml soroban;
        for (const QString& endpoint : { closed, rpc.url() }) {
            soroban.setEndpoint(endpoint);
            for (const auto& e : envelopes) {
                for (const bool given : { false, true }) {
                    const QVariantMap error = failureOf(soroban, [&]() {
                        return given ? soroban.send(e.second, passphrase) : soroban.send(e.second);
                    });
                    const QString message = error.value(QStringLiteral("message")).toString();
                    const QString what = QStringLiteral("%1, %2, %3: %4 %5")
                        .arg(endpoint == closed ? QStringLiteral("unreachable") : QStringLiteral("reachable"),
                             QString::fromLatin1(e.first),
                             given ? QStringLiteral("given its network") : QStringLiteral("without it"),
                             error.value(QStringLiteral("kind")).toString(), message);
                    QVERIFY2(error.value(QStringLiteral("kind")).toString() == QStringLiteral("validation"),
                             qPrintable(what));
                    QVERIFY2(message.startsWith(QStringLiteral("not sent")), qPrintable(what));
                    QVERIFY2(error.contains(QStringLiteral("mayHaveReachedServer")), qPrintable(what));
                    QVERIFY2(!error.value(QStringLiteral("mayHaveReachedServer")).toBool(), qPrintable(what));
                    QVERIFY2(!error.contains(QStringLiteral("hash")), qPrintable(what));
                }
            }
        }
        QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), 0);
        QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), 0);

        // The same endpoint takes an envelope that decodes: the refusals were
        // about the envelopes.
        const QPair<QString, QVariantMap> outcome = outcomeOf(soroban, [&]() { return soroban.send(envelope); });
        QVERIFY2(outcome.first == QStringLiteral("sent"),
                 qPrintable(outcome.second.value(QStringLiteral("message")).toString()));
        QCOMPARE(outcome.second.value(QStringLiteral("hash")).toString(), hash);
        QCOMPARE(rpc.requests(QStringLiteral("getNetwork")), 1);
    }

    /**
     * An envelope pasted with whitespace in it (wrapped over lines, spaced
     * out, with tabs or a no-break space) decodes, and is posted as the SDK
     * writes it back: the same bytes, without the whitespace, given its
     * network or not. It was posted as given, and the RPC refuses a space
     * inside an envelope (soroban-testnet answered -32602 "invalid_xdr" on
     * 2026-09-29; a newline it skips): a send taken and hashed here was
     * refused there, and reported as one that may have reached the server,
     * under a hash never to be found.
     */
    void testAnEnvelopeWithWhitespaceIsPostedWithoutIt()
    {
        QString hash;
        const QString envelope = sorobanEnvelope(&hash, /*sign*/ true);
        const QString passphrase = Network::TESTNET_NETWORK_PASSPHRASE();
        auto spaced = [&envelope](const QString& gap, int every) {
            QString out;
            for (int i = 0; i < envelope.size(); i += every)
                out += (i ? gap : QString()) + envelope.mid(i, every);
            return out;
        };
        const QList<QPair<const char*, QString>> pasted = {
            { "wrapped over lines", spaced(QStringLiteral("\n"), 64) },
            { "wrapped over CRLF lines", spaced(QStringLiteral("\r\n"), 76) },
            { "spaced out", spaced(QStringLiteral(" "), 4) },
            { "with tabs and no-break spaces",
              QStringLiteral("\t") + spaced(QString(QChar(0x00A0)), 20) + QStringLiteral(" \n") },
        };
        MethodRpcEndpoint rpc;
        rpc.answer(QStringLiteral("sendTransaction"), sendAnswer(QStringLiteral("PENDING"), hash));
        qstellar::SorobanQml soroban;
        soroban.setEndpoint(rpc.url());
        int posted = 0;
        for (const auto& p : pasted) {
            // Each one decodes, as the transaction the envelope is.
            QVERIFY2(p.second != envelope, p.first);
            QCOMPARE(hashOn(p.second, passphrase), hash);
            for (const bool given : { false, true }) {
                const QPair<QString, QVariantMap> outcome = outcomeOf(soroban, [&]() {
                    return given ? soroban.send(p.second, passphrase) : soroban.send(p.second);
                });
                const QString what = QStringLiteral("%1, %2: %3 %4").arg(QString::fromLatin1(p.first),
                    given ? QStringLiteral("given its network") : QStringLiteral("without it"),
                    outcome.first, outcome.second.value(QStringLiteral("message")).toString());
                QVERIFY2(outcome.first == QStringLiteral("sent"), qPrintable(what));
                QVERIFY2(outcome.second.value(QStringLiteral("hash")).toString() == hash, qPrintable(what));
                QCOMPARE(rpc.requests(QStringLiteral("sendTransaction")), ++posted);
                const QString sent = rpc.lastParams(QStringLiteral("sendTransaction")).toObject()
                                         .value(QStringLiteral("transaction")).toString();
                QVERIFY2(sent == envelope, qPrintable(what + QStringLiteral("; posted ") + sent));
            }
        }
    }

    /**
     * With STELLAR_QT_LEGACY_GLOBAL_NS (this suite defines it) SorobanQml is
     * reachable unqualified, like StellarQml and every other QML class. It had
     * no alias, so this did not compile.
     */
    void testTheLegacyFlagAliasesSorobanQml()
    {
        static_assert(std::is_same<SorobanQml, qstellar::SorobanQml>::value,
                      "SorobanQml is aliased at global scope under the legacy flag");
        SorobanQml soroban;
        QCOMPARE(soroban.rpcUrlFor(QStringLiteral("testnet")),
                 QStringLiteral("https://soroban-testnet.stellar.org"));
    }
};

ADD_TEST(SorobanQmlTest)
#endif // SOROBANQMLTEST_H
