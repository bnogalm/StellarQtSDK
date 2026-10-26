#include "sorobanqml.h"

#include <QDataStream>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QThread>
#include <QVariantList>
#include <atomic>
#include <stdexcept>

#include "contractspec.h"
#include "abstracttransaction.h"
#include "network.h"
#include "transaction.h"
#include "sorobanserver.h"
#include "util.h"

#include "responses/sorobanrpc/gethealthresponse.h"
#include "responses/sorobanrpc/sendtransactionresponse.h"
#include "responses/sorobanrpc/simulatetransactionresponse.h"

#include "scval/scv.h"
#include "xdr/scval.h"
#include "xdr/sorobantxdata.h"

QSTELLAR_BEGIN_NS

namespace {

const char* const TESTNET_RPC = "https://soroban-testnet.stellar.org";
const char* const PUBLIC_RPC = "https://mainnet.sorobanrpc.com";

/**
 * Decodes base64 XDR into `value`, or returns false. Never throws.
 *
 * Whole or nothing, as the SDK decodes what it signs: strict base64, and every
 * byte read. A character that is not base64 used to be skipped, and bytes left
 * over ignored, so a mangled answer was shown as a return value and a fee.
 */
template <typename T>
bool decodeXdr(const QString& base64, T& value)
{
    if (base64.isEmpty())
        return false;
    const auto decoded = QByteArray::fromBase64Encoding(
        base64.toLatin1(), XDR_BASE64ENCODING | QByteArray::AbortOnBase64DecodingErrors);
    if (!decoded || (*decoded).isEmpty())
        return false;
    QByteArray raw = *decoded;
    QDataStream stream(&raw, QIODevice::ReadOnly);
    try {
        stream >> value;
    } catch (...) {
        return false;
    }
    return stream.status() == QDataStream::Ok && stream.atEnd();
}

/** A footprint entry, named the way a person would name it. */
QString describeKey(const stellar::SorobanLedgerKey& key)
{
    switch (key.type) {
    case stellar::LedgerEntryType::CONTRACT_DATA:
        return QStringLiteral("contract data");
    case stellar::LedgerEntryType::CONTRACT_CODE:
        return QStringLiteral("contract code");
    case stellar::LedgerEntryType::TTL:
        return QStringLiteral("time to live");
    case stellar::LedgerEntryType::ACCOUNT:
        return QStringLiteral("account");
    case stellar::LedgerEntryType::TRUSTLINE:
        return QStringLiteral("trustline");
    default:
        return QStringLiteral("ledger entry");
    }
}

/**
 * An SCVal as something a person can read.
 *
 * The SDK has no pretty-printer — the only textual form it produces is base64
 * XDR — so this is the difference between a lab that shows you what a contract
 * returned and one that shows you eighty characters of base64.
 */
QString describeScVal(const stellar::SCVal& value)
{
    using stellar::SCValType;
    try {
        switch (value.type) {
        case SCValType::SCV_VOID:    return QStringLiteral("void");
        case SCValType::SCV_BOOL:    return value.b ? QStringLiteral("true")
                                                    : QStringLiteral("false");
        case SCValType::SCV_U32:     return QString::number(value.u32);
        case SCValType::SCV_I32:     return QString::number(value.i32);
        case SCValType::SCV_U64:     return QString::number(value.u64);
        case SCValType::SCV_I64:     return QString::number(value.i64);
        case SCValType::SCV_TIMEPOINT: return QString::number(value.timepoint);
        case SCValType::SCV_DURATION:  return QString::number(value.duration);
        // Exact decimal, never a double: an i128 is what a token balance is,
        // and it used to be shown as "(type 10)".
        case SCValType::SCV_U128:
        case SCValType::SCV_I128:
        case SCValType::SCV_U256:
        case SCValType::SCV_I256:    return Scv::decimalText(value);
        case SCValType::SCV_SYMBOL:  return Scv::fromSymbol(value);
        case SCValType::SCV_STRING:  return Scv::fromString(value);
        case SCValType::SCV_BYTES:   return QString::fromLatin1(
                                            Scv::fromBytes(value).toHex());
        case SCValType::SCV_ADDRESS: return Scv::fromAddress(value).toString();
        case SCValType::SCV_VEC:     return QStringLiteral("vector of %1")
                                            .arg(value.vec ? value.vec->size() : 0);
        case SCValType::SCV_MAP:     return QStringLiteral("map of %1")
                                            .arg(value.map ? value.map->size() : 0);
        default:                     return QStringLiteral("(type %1)")
                                            .arg(int(value.type));
        }
    } catch (const std::exception&) {
        return QStringLiteral("(unreadable)");
    }
}

/**
 * A contract report, flattened for QML, carrying the bytes it was read from.
 *
 * The bytes are here because reading a contract and deploying one are the same
 * file seen twice, and there was no way across: `uploadContractWasm` wants the
 * module as hex, and hexing a file by hand outside the app is not a step, it is
 * a wall. `wasmHash` is the SHA-256 of those same bytes, which is the hash the
 * ledger files the code under and the one `createContract` points at — so it is
 * known BEFORE the upload rather than read out of its result.
 *
 * Both are omitted for anything that is not WebAssembly: offering to upload a
 * JPEG would be offering to spend a fee on one.
 */
QVariantMap contractToMap(const ContractSpec::Contract& contract,
                          const QByteArray& wasm)
{
    QVariantMap out;
    out.insert(QStringLiteral("isWasm"), contract.isWasm);
    out.insert(QStringLiteral("hasEnvMeta"), contract.hasEnvMeta);
    out.insert(QStringLiteral("hasSpec"), contract.hasSpec);
    out.insert(QStringLiteral("protocolVersion"), contract.protocolVersion);
    out.insert(QStringLiteral("preRelease"), contract.preRelease);
    out.insert(QStringLiteral("problem"), contract.problem);
    out.insert(QStringLiteral("sections"), QVariant(contract.sectionNames));

    if (contract.isWasm) {
        out.insert(QStringLiteral("wasmSize"), int(wasm.size()));
        out.insert(QStringLiteral("wasmHex"), QString::fromLatin1(wasm.toHex()));
        out.insert(QStringLiteral("wasmHash"),
                   QString::fromLatin1(Util::hash(wasm).toHex()));
    }

    QVariantList metadata;
    for (const auto& pair : contract.metadata) {
        QVariantMap entry;
        entry.insert(QStringLiteral("key"), pair.first);
        entry.insert(QStringLiteral("value"), pair.second);
        metadata.append(entry);
    }
    out.insert(QStringLiteral("metadata"), metadata);

    QVariantList functions;
    for (const auto& function : contract.functions) {
        QVariantMap entry;
        entry.insert(QStringLiteral("name"), function.name);
        entry.insert(QStringLiteral("doc"), function.doc);
        entry.insert(QStringLiteral("returns"), function.returns);

        QVariantList inputs;
        bool editable = true;
        for (const auto& input : function.inputs) {
            QVariantMap argument;
            argument.insert(QStringLiteral("name"), input.name);
            argument.insert(QStringLiteral("type"), input.type);
            argument.insert(QStringLiteral("scvType"), input.scvType);
            argument.insert(QStringLiteral("doc"), input.doc);
            inputs.append(argument);
            if (input.scvType.isEmpty())
                editable = false;
        }
        entry.insert(QStringLiteral("inputs"), inputs);
        // Whether a form can offer an editor for every argument, or whether the
        // caller has to write the JSON by hand for at least one of them.
        entry.insert(QStringLiteral("fullyEditable"), editable);
        functions.append(entry);
    }
    out.insert(QStringLiteral("functions"), functions);
    return out;
}

/** A request refused before anything was sent: reported as "validation". */
struct RefusedBeforeSending : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

/**
 * A send whose transaction was never posted: the lookup of the endpoint's
 * network before it failed, or a cancel came meanwhile. Reported as `kind`,
 * and never as having reached the server — the lookup may have, the
 * transaction did not.
 */
struct NotSent : std::runtime_error
{
    NotSent(const QString& failureKind, const QString& message)
        : std::runtime_error(message.toStdString()), kind(failureKind) {}
    QString kind;
};

} // namespace

/**
 * The worker's server. It notes whether the call in progress handed a request
 * to the network, so a failure can say whether it may have reached the
 * server: one that failed before (a refused envelope, an endpoint that is not
 * a URL) cannot have.
 */
class SorobanWorkerServer : public SorobanServer
{
public:
    explicit SorobanWorkerServer(const QUrl& endpoint) : SorobanServer(endpoint) {}

    bool posted = false;

    /**
     * The endpoint's network passphrase as its getNetwork answered, and the
     * endpoint change it was learned after: good until the next change, even
     * back to this same URL. Empty until a send has learned it.
     */
    QString networkPassphrase;
    int networkLearnedAt = -1;

    QJsonValue sendRequest(const QString& method, const QJsonValue& params) override
    {
        posted = true;
        return SorobanServer::sendRequest(method, params);
    }
};

/**
 * Everything that touches the network, living on its own thread.
 *
 * It owns its SorobanServer, which owns a QNetworkAccessManager, which has
 * thread affinity — so the server is created lazily here, inside a slot, and
 * therefore on the worker thread rather than on whichever thread constructed
 * this object.
 *
 * Nothing throws out of run(): an exception escaping a slot invoked by Qt's
 * event loop is undefined behaviour, and the SDK's exceptions are not
 * QException subclasses, so they could not be carried across a thread even if
 * that were allowed. Everything is flattened to a map before it goes back.
 *
 * Requests run ONE AT A TIME, in the order they were started. Every RPC call
 * waits in a nested event loop on this thread, and that loop delivers the next
 * queued run(): a request started meanwhile used to run inside the one in
 * flight, and one for a changed endpoint deleted the server — with its
 * QNetworkAccessManager and the reply the outer call was still waiting on —
 * from under it. The outer call then waited forever on a reply that no longer
 * existed, and touched it once the thread was told to quit.
 *
 * What bounds the one in flight is the server's request timeout, and cancel():
 * the nested loop delivers that queued call too, and it aborts the request, so
 * the queue moves on. Before, nothing did, and one endpoint that never
 * answered blocked every request behind it for good.
 */
class SorobanWorker : public QObject
{
    Q_OBJECT
public:
    explicit SorobanWorker(QObject* parent = nullptr) : QObject(parent) {}
    ~SorobanWorker() override { delete m_server; }

    /**
     * Requests up to this id were cancelled: one of them that has not started
     * yet never does. Written by the facade on its own thread, hence atomic.
     */
    std::atomic<int> cancelledThrough{0};

public slots:
    /** `endpointChange` counts the facade's endpoint changes when it started. */
    void run(int requestId, const QString& endpoint, const QString& method,
             const QString& argument, const QString& context, int endpointChange)
    {
        m_queue.append(Request{ requestId, endpoint, method, argument, context, endpointChange });
        if (m_running)
            return;                     // re-entered from a call in flight: wait
        m_running = true;
        while (!m_queue.isEmpty()) {
            const Request request = m_queue.takeFirst();
            if (request.id <= cancelledThrough.load()) {
                // Cancelled before it started: never sent. Said, as not
                // started, so a facade still waiting for it (an endpoint
                // change) hears that it was not, and one that cancelled it
                // with cancelAll() drops it without a word.
                QVariantMap payload;
                payload.insert(QStringLiteral("message"),
                               QStringLiteral("cancelled before it was sent"));
                payload.insert(QStringLiteral("mayHaveReachedServer"), false);
                emit finished(request.id, QStringLiteral("cancelled"), payload, false);
                continue;
            }
            m_current = request.id;
            m_abortedCurrent = false;
            execute(request);
            m_current = 0;
        }
        m_running = false;
    }

    /**
     * Requests up to `through` are cancelled. The one in flight, if it is one
     * of them, is aborted: it waits in a nested event loop, which delivers
     * this queued call. One started after `through` is left alone — the call
     * can arrive after the cancelled request has already finished.
     */
    void cancel(int through)
    {
        if (m_current != 0 && m_current <= through && m_server) {
            m_abortedCurrent = true;
            m_server->cancelRequests();
        }
    }

signals:
    /**
     * `started` is false only for a request dropped, cancelled, before it
     * was taken up; every request taken up ends in one with it true, the
     * outcome of a cancel it met included.
     */
    void finished(int requestId, const QString& kind, const QVariantMap& payload, bool started);

private:
    struct Request
    {
        int id;
        QString endpoint;
        QString method;
        QString argument;
        QString context;
        int endpointChange;
    };

    void execute(const Request& request)
    {
        QVariantMap payload;
        QString kind;
        // A send's hash on the endpoint's network: what an outcome-unknown
        // send is looked up by.
        QString hash;
        // The hash the RPC answered a send with, when it is not `hash`.
        QString rpcHash;
        // Qt sent the request again, by itself, after it had written it whole
        // (SorobanTransportException::resent()).
        bool resent = false;
        SorobanWorkerServer* server = nullptr;
        // Every failure says whether the request may have reached the server,
        // and a send that may have, its hash. The map said neither, so a
        // caller could not tell a send that never left from one that may yet
        // be applied, nor what to look up.
        //
        // A send Qt sent again says so, whatever the kind: what ended it came
        // of a later send, while the RPC may have taken in, and applied, the
        // first. So not even the RPC's own answer is final: an ERROR (with
        // txBAD_SEQ, most likely because the first was applied) or a
        // TRY_AGAIN_LATER to the later send fails here (SorobanServer refuses
        // it as the answer), where it was sent() as the RPC's final word.
        // `resent` and the message say it, with the hash to look up.
        auto fail = [&](const QString& failureKind, const QString& message, bool reached) {
            kind = failureKind;
            payload.clear();
            const bool notFinal = reached && resent && request.method == QLatin1String("send");
            payload.insert(QStringLiteral("message"),
                           notFinal
                               ? QStringLiteral("not final: Qt sent the transaction again, by itself, "
                                                "after the connection that carried it whole closed "
                                                "with no answer, so what follows came of a later "
                                                "send, and the RPC may have taken in, and applied, "
                                                "the first; look it up by its hash, %1, before "
                                                "sending it again or reusing its sequence number. %2")
                                     .arg(hash, message)
                               : message);
            payload.insert(QStringLiteral("mayHaveReachedServer"), reached);
            if (reached && !hash.isEmpty())
                payload.insert(QStringLiteral("hash"), hash);
            if (reached && !rpcHash.isEmpty())
                payload.insert(QStringLiteral("rpcHash"), rpcHash);
            if (notFinal)
                payload.insert(QStringLiteral("resent"), true);
        };
        // Aborted by cancel(), and said so plainly: for a send, whether it
        // reached the server is not known.
        auto cancelled = [&](bool reached) {
            fail(QStringLiteral("cancelled"),
                 request.method == QLatin1String("send")
                     ? QStringLiteral("cancelled while it was on the wire: the server may have "
                                      "received the transaction, so look it up by its hash before "
                                      "sending it again")
                     : QStringLiteral("cancelled while it was on the wire"),
                 reached);
        };
        try {
            server = serverFor(request.endpoint);
            server->posted = false;
            if (request.method == QLatin1String("simulate"))
                payload = simulate(server, request.argument);
            else if (request.method == QLatin1String("prepare"))
                payload = prepare(server, request.argument, request.context);
            else if (request.method == QLatin1String("send"))
                payload = send(server, request, hash, rpcHash);
            else if (request.method == QLatin1String("health"))
                payload = health(server);
            else
                throw std::runtime_error("unknown Soroban request");
        } catch (const RefusedBeforeSending& e) {
            fail(QStringLiteral("validation"), QString::fromUtf8(e.what()), false);
        } catch (const NotSent& e) {
            // Whatever the lookup before it did, the transaction was not
            // posted. A cancel that aborted the lookup is said as a cancel
            // before sending: taken for the send's own abort, it would say
            // the transaction may have reached the server.
            if (m_abortedCurrent)
                fail(QStringLiteral("cancelled"), QStringLiteral("cancelled before it was sent"), false);
            else
                fail(e.kind, QString::fromUtf8(e.what()), false);
        } catch (const qstellar::exception::SorobanTransportException& e) {
            // Its own word on it: false only for a connection that never opened.
            resent = e.resent();
            if (m_abortedCurrent)
                cancelled(e.mayHaveReachedServer());
            else
                fail(classify(QString::fromUtf8(e.what())), QString::fromUtf8(e.what()),
                     e.mayHaveReachedServer());
        } catch (const std::exception& e) {
            // An RPC error, an answer that does not parse, or a refusal of what
            // came back all come after the request was posted; one that fails
            // before it (an envelope refused, an endpoint that is not a URL)
            // sent nothing.
            const bool reached = server && server->posted;
            if (m_abortedCurrent)
                cancelled(reached);
            else
                fail(classify(QString::fromUtf8(e.what())), QString::fromUtf8(e.what()), reached);
        } catch (...) {
            // Not defensive style: the SDK throws types that are not
            // std::exception, and one of them escaping here would take the
            // process down.
            fail(QStringLiteral("unknown"),
                 QStringLiteral("the Soroban server failed in an unknown way"),
                 server && server->posted);
        }
        emit finished(request.id, kind, payload, true);
    }

    SorobanWorkerServer* serverFor(const QString& endpoint)
    {
        // Built here, so it and its QNetworkAccessManager belong to this thread.
        // Kept between calls so one Soroban flow — simulate, send, poll — reuses
        // a single connection instead of shaking hands with TLS each time.
        // Replaced only between requests (run() never nests one in another), so
        // no call is ever left inside a server that has been deleted.
        if (!m_server || m_endpoint != endpoint) {
            // Null the member BEFORE the allocation that can throw.
            // SorobanServer's constructor rejects an invalid URL by throwing,
            // and this used to leave m_server holding the pointer it had just
            // freed: the next call saw a stale endpoint, deleted it a second
            // time, and corrupted the heap on the worker thread. The endpoint
            // is only recorded once the server really exists.
            delete m_server;
            m_server = nullptr;
            m_endpoint.clear();
            m_server = new SorobanWorkerServer(QUrl(endpoint));
            m_endpoint = endpoint;
        }
        return m_server;
    }

    static QString classify(const QString& message)
    {
        if (message.contains(QLatin1String("HTTP error")))
            return QStringLiteral("connection");
        if (message.contains(QLatin1String("RPC")))
            return QStringLiteral("rpc");
        return QStringLiteral("unknown");
    }

    static QVariantMap simulate(SorobanServer* server, const QString& envelope)
    {
        //! [sorobanSimulate.cpp]
        const SimulateTransactionResponse response =
            server->simulateTransactionXdr(envelope);
        //! [sorobanSimulate.cpp]

        QVariantMap out;
        // A failed simulation is an ANSWER, not an error: the contract ran and
        // refused, and what it said is the most useful thing on the page.
        out.insert(QStringLiteral("ok"), !response.isError());
        out.insert(QStringLiteral("error"), response.getError());
        out.insert(QStringLiteral("latestLedger"), response.getLatestLedger());
        out.insert(QStringLiteral("minResourceFee"), response.getMinResourceFee());
        out.insert(QStringLiteral("transactionData"), response.getTransactionData());

        const auto& results = response.getResults();
        if (!results.isEmpty()) {
            out.insert(QStringLiteral("returnValueXdr"), results.first().xdr);
            stellar::SCVal value;
            if (decodeXdr(results.first().xdr, value))
                out.insert(QStringLiteral("returnValue"), describeScVal(value));
            out.insert(QStringLiteral("auth"), QVariant(results.first().auth));
        }

        // The footprint and the resource fee are not getters: they live inside
        // the base64 transactionData blob, which is the part a caller most
        // needs and the part the SDK does not unpack.
        stellar::SorobanTransactionData data;
        if (decodeXdr(response.getTransactionData(), data)) {
            out.insert(QStringLiteral("resourceFee"), QString::number(data.resourceFee));
            out.insert(QStringLiteral("instructions"), data.resources.instructions);
            out.insert(QStringLiteral("readBytes"), data.resources.diskReadBytes);
            out.insert(QStringLiteral("writeBytes"), data.resources.writeBytes);

            QVariantList readOnly;
            for (const auto& key : data.resources.footprint.readOnly)
                readOnly.append(describeKey(key));
            QVariantList readWrite;
            for (const auto& key : data.resources.footprint.readWrite)
                readWrite.append(describeKey(key));
            out.insert(QStringLiteral("footprintReadOnly"), readOnly);
            out.insert(QStringLiteral("footprintReadWrite"), readWrite);
        }

        out.insert(QStringLiteral("events"), QVariant(response.getEvents()));

        QVariantList changes;
        for (const auto& change : response.getStateChanges()) {
            QVariantMap entry;
            entry.insert(QStringLiteral("type"), change.type);
            entry.insert(QStringLiteral("key"), change.key);
            changes.append(entry);
        }
        out.insert(QStringLiteral("stateChanges"), changes);
        return out;
    }

    /**
     * Simulate, then apply what came back to the transaction itself.
     *
     * prepareTransaction does the three things a Soroban envelope needs and a
     * caller cannot compute: it decodes the simulated SorobanTransactionData
     * into the transaction's ext, fills in the auth entries when the host
     * function has none, and sets the fee to the inclusion fee the caller bid
     * plus the resource fee that SorobanTransactionData carries.
     *
     * The Network is built here from the passphrase rather than fetched from
     * Network::testnetNetwork(). Those accessors memoise into a static QMap
     * with no lock, and this runs on the worker thread — the same trap that
     * had to be avoided when this facade was first written.
     */
    static QVariantMap prepare(SorobanServer* server, const QString& envelope,
                               const QString& passphrase)
    {
        if (passphrase.trimmed().isEmpty())
            throw std::runtime_error("preparing needs the network passphrase: "
                                     "an envelope does not carry one");

        Network network(passphrase);
        QScopedPointer<AbstractTransaction> parsed(
            AbstractTransaction::fromEnvelopeXdr(envelope, &network));
        Transaction* tx = dynamic_cast<Transaction*>(parsed.data());
        if (!tx)
            throw std::runtime_error("a Soroban transaction is a plain one; "
                                     "this envelope is a fee-bump");

        // REFUSE a signed envelope rather than quietly ruining it. Preparing
        // rewrites the fee, the footprint and the auth entries, all of which
        // the signature covers — so a signature made first is now over a
        // transaction that does not exist. The envelope stays well-formed and
        // the signatures ride along, so nothing complains until the network
        // answers tx_bad_auth and says nothing about why.
        if (!tx->getSignatures().isEmpty())
            throw std::runtime_error("this envelope is already signed. Preparing "
                                     "rewrites the transaction and therefore its "
                                     "hash, so prepare first and sign afterwards");

        const qint64 feeBefore = tx->getFee();
        const int signaturesBefore = tx->getSignatures().size();

        // Simulate and apply as two steps rather than calling
        // prepareTransaction, which does both and throws the simulation away.
        // What it discards is the part a caller most needs: the RETURN VALUE.
        // Deploying a contract answers with the contract's address, which
        // nothing else in the response carries. (An asset contract's id can
        // also be derived from its asset, with Util::getContractAddressFromAsset
        // or Asset::toSAC, but this is handed a transaction, not an asset.)
        // applyTransactionSimulation is the documented pure half, made
        // reachable for exactly this reason.
        const SimulateTransactionResponse sim = server->simulateTransaction(tx);
        if (sim.isError())
            throw std::runtime_error(QStringLiteral("the simulation refused it: %1")
                                         .arg(sim.getError()).toStdString());
        SorobanServer::applyTransactionSimulation(tx, sim);

        QVariantMap out;
        out.insert(QStringLiteral("preparedXdr"), tx->toEnvelopeXdrBase64());
        out.insert(QStringLiteral("xdr"), tx->toEnvelopeXdrBase64());
        out.insert(QStringLiteral("hash"), tx->hashHex());
        out.insert(QStringLiteral("feeBefore"), QString::number(feeBefore));
        out.insert(QStringLiteral("fee"), QString::number(tx->getFee()));
        // The resource fee the prepared transaction carries. The difference
        // of the two fees is not it any more: re-preparing replaces the
        // resource fee instead of adding another, so that difference can be
        // 0 or negative.
        out.insert(QStringLiteral("resourceFee"),
                   QString::number(tx->getSorobanData() ? tx->getSorobanData()->resourceFee
                                                        : tx->getFee() - feeBefore));
        out.insert(QStringLiteral("minResourceFee"), sim.getMinResourceFee());
        out.insert(QStringLiteral("latestLedger"), sim.getLatestLedger());
        // Said out loud because it is the thing that catches people: any
        // signature made before this point is now over a different hash.
        out.insert(QStringLiteral("signaturesCarried"), signaturesBefore);
        out.insert(QStringLiteral("hashChanged"), true);

        const auto& results = sim.getResults();
        if (!results.isEmpty()) {
            out.insert(QStringLiteral("returnValueXdr"), results.first().xdr);
            stellar::SCVal value;
            if (decodeXdr(results.first().xdr, value)) {
                out.insert(QStringLiteral("returnValue"), describeScVal(value));
                // A deploy answers with an address, and that address is the
                // contract id every later call needs. Pulled out by name so a
                // caller does not have to parse the description.
                if (value.type == stellar::SCValType::SCV_ADDRESS) {
                    const QString address = describeScVal(value);
                    if (address.startsWith(QLatin1Char('C')))
                        out.insert(QStringLiteral("contractId"), address);
                }
            }
            out.insert(QStringLiteral("authEntries"), results.first().auth.size());
        }
        return out;
    }

    /**
     * The endpoint's network passphrase, as its getNetwork answers it: asked
     * once per endpoint, and again after every endpoint change (back to the
     * same URL included), an answer that contradicted it, or a given
     * passphrase that differs from it (send()). Failing to learn it is
     * NotSent, with the lookup's own kind. `asked` says whether it was asked
     * just now, rather than remembered from an earlier request.
     */
    QString endpointPassphrase(SorobanWorkerServer* server, const Request& request,
                               bool& asked)
    {
        asked = false;
        if (server->networkLearnedAt == request.endpointChange
            && !server->networkPassphrase.isEmpty())
            return server->networkPassphrase;
        auto notRead = [](const QString& kind, const QString& why) {
            return NotSent(kind, QStringLiteral("not sent: the endpoint's network could not be "
                                                "read (getNetwork), and the transaction's hash "
                                                "depends on it: %1").arg(why));
        };
        QString passphrase;
        try {
            passphrase = server->getNetwork().getPassphrase();
        } catch (const std::exception& e) {
            const QString why = QString::fromUtf8(e.what());
            throw notRead(classify(why), why);
        } catch (...) {
            throw notRead(QStringLiteral("unknown"), QStringLiteral("it failed in an unknown way"));
        }
        server->posted = false;         // the lookup is not the request
        server->networkPassphrase = passphrase;
        server->networkLearnedAt = request.endpointChange;
        asked = true;
        return passphrase;
    }

    /**
     * Sends the envelope, knowing first which transaction it is on the
     * endpoint's network, so that a send whose outcome is unknown can say
     * what to look up.
     *
     * The hash depends on the network passphrase, which an envelope does not
     * carry, and a network applies a transaction under the hash its own
     * passphrase gives it. So the endpoint's passphrase is learned first
     * (endpointPassphrase) and the hash worked out with it: a passphrase the
     * caller gave that is not the endpoint's is refused — the hash worked out
     * from it is one that network never gives the transaction — and without
     * one the endpoint's is used. The given passphrase used to be trusted, so
     * a send to another network than meant was reported under a hash that
     * could never be found there; and the one-argument send reported none.
     *
     * Nothing is posted until all of that is known: an envelope that does not
     * decode exactly, a network that cannot be read, a passphrase that is not
     * the endpoint's, or a cancel meanwhile, and the transaction is not sent.
     * An answer with another hash than the one worked out is refused, and
     * carries both: the RPC's own is the one the transaction can land under.
     *
     * A given passphrase is refused on a fresh answer only. One remembered
     * from an earlier send can be stale (the RPC behind the same URL, a local
     * one restarted on another network, may serve another network since), so
     * a passphrase that differs from it has the endpoint asked once more
     * first.
     */
    QVariantMap send(SorobanWorkerServer* server, const Request& request,
                     QString& hash, QString& rpcHash)
    {
        const QString& envelope = request.argument;
        const QString& given = request.context;     // empty for send(envelope)
        // The envelope as the SDK writes it back: what is posted.
        QString written;
        auto hashOn = [&envelope, &written](const QString& passphrase) -> QString {
            Network network(passphrase);
            try {
                QScopedPointer<AbstractTransaction> parsed(
                    AbstractTransaction::fromEnvelopeXdr(envelope, &network));
                written = parsed->toEnvelopeXdrBase64();
                return parsed->hashHex();
            } catch (const std::exception& e) {
                throw RefusedBeforeSending(
                    QStringLiteral("not sent: its hash cannot be worked out, so a send whose "
                                   "outcome is unknown could not be looked up (%1)")
                        .arg(QString::fromUtf8(e.what())).toStdString());
            }
        };
        // Before any request, given its network or not: an envelope that does
        // not decode exactly (and write back byte for byte) costs none. Only
        // the hash depends on the network, not the decoding, so without one
        // it is read on a stand-in passphrase and hashed once the endpoint's
        // network is known. It used to be read only after that lookup, which
        // cost a request, and against an endpoint that could not be reached
        // failed as "connection" instead of "validation".
        if (given.isEmpty())
            (void)hashOn(QStringLiteral("stand-in: decoding only, no hash is kept"));
        else
            hash = hashOn(given);

        // The endpoint's network, and whether it was asked for this send.
        bool asked = false;
        auto learn = [&]() -> QString {
            const QString passphrase = endpointPassphrase(server, request, asked);
            // A cancel that came while the lookup was ending — delivered in
            // the batch of events that finished it, when there was nothing
            // left to abort — or that is still on its way. Without this the
            // send went out after it, and the cancel had been spent on the
            // lookup.
            if (m_abortedCurrent || request.id <= cancelledThrough.load())
                throw NotSent(QStringLiteral("cancelled"),
                              QStringLiteral("cancelled before it was sent"));
            return passphrase;
        };
        QString served = learn();

        // Remembered from an earlier send, and not the one given: asked once
        // more before refusing. The RPC behind this URL may serve another
        // network since, and the remembered answer was only dropped when a
        // posted send came back with another hash, which a refused send never
        // is: the right passphrase was refused for as long as the endpoint
        // stayed set.
        if (!given.isEmpty() && given != served && !asked) {
            server->networkPassphrase.clear();
            served = learn();
        }

        if (given.isEmpty()) {
            hash = hashOn(served);
        } else if (given != served) {
            throw RefusedBeforeSending(
                QStringLiteral("not sent: the endpoint serves the network \"%1\", not the one "
                               "given (\"%2\"); a network applies a transaction under the hash "
                               "its own passphrase gives it, so %3 could never be found there")
                    .arg(served, given, hash).toStdString());
        }

        // Posted as the SDK writes it back, which the decoding above checked
        // is the envelope given, byte for byte, without the whitespace it
        // skips (an envelope pasted wrapped over lines). The RPC takes no
        // space inside one ("invalid_xdr"): such an envelope was hashed and
        // taken here, posted as given, refused there, and reported as one
        // that may have reached the server, under a hash never found.
        const SendTransactionResponse response = server->sendTransactionXdr(written);
        if (response.getHash().compare(hash, Qt::CaseInsensitive) != 0) {
            // An honest RPC hashes with the passphrase it names, so it has
            // changed since it was learned (or it is not honest): learned
            // again by the next send.
            server->networkPassphrase.clear();
            rpcHash = response.getHash().toLower();
            throw std::runtime_error(
                QStringLiteral("the RPC answered with hash %1 (rpcHash), not %2 (hash), the "
                               "transaction's hash on the network it names. %1 is the one it "
                               "can land under: a network applies a transaction under the hash "
                               "its own passphrase gives it, and that is the hash the RPC works "
                               "out. It may have been received: look %1 up before sending it "
                               "again or reusing its sequence number")
                    .arg(rpcHash, hash).toStdString());
        }
        // TRY_AGAIN_LATER and ERROR come here only as the answer to the one
        // send made: to a send Qt made again, by itself, SorobanServer
        // refuses them (SorobanTransportException, resent), and they fail in
        // execute(), not final. PENDING and DUPLICATE say the RPC has it,
        // however many sends it took.
        QVariantMap out;
        switch (response.getStatus()) {
        case SendTransactionResponse::Status::PENDING:
            out.insert(QStringLiteral("status"), QStringLiteral("pending")); break;
        case SendTransactionResponse::Status::DUPLICATE:
            out.insert(QStringLiteral("status"), QStringLiteral("duplicate")); break;
        case SendTransactionResponse::Status::TRY_AGAIN_LATER:
            out.insert(QStringLiteral("status"), QStringLiteral("tryAgainLater")); break;
        case SendTransactionResponse::Status::ERROR_:
            out.insert(QStringLiteral("status"), QStringLiteral("error")); break;
        }
        out.insert(QStringLiteral("hash"), response.getHash());
        out.insert(QStringLiteral("latestLedger"), response.getLatestLedger());
        out.insert(QStringLiteral("errorResultXdr"), response.getErrorResultXdr());
        out.insert(QStringLiteral("events"), QVariant(response.getDiagnosticEventsXdr()));
        return out;
    }

    static QVariantMap health(SorobanServer* server)
    {
        const GetHealthResponse response = server->getHealth();
        QVariantMap out;
        out.insert(QStringLiteral("status"), response.getStatus());
        out.insert(QStringLiteral("healthy"),
                   response.getStatus().compare(QLatin1String("healthy"),
                                                Qt::CaseInsensitive) == 0);
        out.insert(QStringLiteral("latestLedger"), response.getLatestLedger());
        out.insert(QStringLiteral("oldestLedger"), response.getOldestLedger());
        return out;
    }

    SorobanWorkerServer* m_server = nullptr;
    QString m_endpoint;
    QList<Request> m_queue;
    bool m_running = false;
    int m_current = 0;                  // the request in flight, 0 for none
    bool m_abortedCurrent = false;      // cancel() aborted it
};

// ─── The facade ──────────────────────────────────────────────────────

SorobanQml::SorobanQml(QObject* parent)
    : QObject(parent)
    , m_endpoint(QString::fromLatin1(TESTNET_RPC))
{
    m_thread = new QThread(this);
    m_thread->setObjectName(QStringLiteral("soroban-rpc"));

    m_worker = new SorobanWorker();     // no parent: moved to the thread below
    m_worker->moveToThread(m_thread);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &SorobanWorker::finished,
            this, &SorobanQml::onWorkerFinished, Qt::QueuedConnection);
    m_thread->start();
}

SorobanQml::~SorobanQml()
{
    // cancelAll() aborts the request on the wire, and quit() ends every event
    // loop the worker waits in (one it starts afterwards returns at once, and
    // SorobanServer then aborts its request), so the worker normally finishes
    // at once. The wait stays bounded all the same: a hung exit is worse
    // than a worker left to finish.
    cancelAll();
    // Connected before quit(), so the thread's finished() cannot come first.
    // When the thread ends within the wait, it is deleted below with this
    // object instead, as its child, and the queued deleteLater goes with it.
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);
    m_thread->quit();
    if (!m_thread->wait(2000)) {
        // Still busy (in a step that waits in no event loop, or starved of
        // CPU): ABANDONED, never terminated. Terminating it tore its
        // QNetworkAccessManager and replies down from this thread, which a
        // Qt 6 debug build can abort on ("Cannot send events to objects
        // owned by a different thread") and a release build may crash on. It
        // now finishes on its own thread, which deletes the worker as it
        // ends (finished() -> deleteLater, see the constructor), and the
        // thread object follows, here, on a later pass of the event loop.
        qWarning("SorobanQml: the RPC worker did not stop within 2 s; it is left to finish on its own thread");
        m_thread->setParent(nullptr);
    }
}

QString SorobanQml::rpcUrlFor(const QString& network) const
{
    if (network == QLatin1String("testnet"))
        return QString::fromLatin1(TESTNET_RPC);
    if (network == QLatin1String("public"))
        return QString::fromLatin1(PUBLIC_RPC);
    return QString();
}

void SorobanQml::setEndpoint(const QString& endpoint)
{
    if (endpoint == m_endpoint)
        return;
    m_endpoint = endpoint;
    // What a send learned of the old endpoint's network is not taken for the
    // new one's, even when this changes back to a URL already asked.
    ++m_endpointChanges;
    // Every request not yet answered was started against the old endpoint
    // (an earlier change abandoned the ones before), so all of them are:
    // they stay pending until the worker reports each as cancelled. The one on
    // the wire used to run to completion, and an endpoint that never answered
    // held up every request to the new one forever.
    cancelStarted();
    emit endpointChanged();
}

void SorobanQml::cancelStarted()
{
    // The atomic first, so a request still queued is dropped even if the
    // worker reaches it before the queued call below; that call aborts the
    // one on the wire, delivered by the nested loop it waits in.
    m_worker->cancelledThrough.store(m_lastRequestId);
    QMetaObject::invokeMethod(m_worker, "cancel", Qt::QueuedConnection,
                              Q_ARG(int, m_lastRequestId));
}

void SorobanQml::cancelAll()
{
    // A request still waiting behind the one in flight is dropped before it
    // starts, so a cancelled send is never sent, and the one in flight is
    // aborted. It used to run to completion — nothing interrupted the nested
    // event loop inside the SDK's RPC call — so a stalled endpoint held up
    // every request after it.
    cancelStarted();
    // Forgotten at once, all but the sends: only the worker knows whether it
    // had started one, and one it had may have reached the server, so it is
    // kept until the worker says. (The sends were forgotten too, and the
    // worker's report of one aborted on the wire was dropped as no longer
    // pending: a wallet that cancelled never heard that its transaction may
    // land.)
    bool forgot = false;
    for (auto it = m_pending.begin(); it != m_pending.end();) {
        if (it.value()) {
            m_cancelledSends.insert(it.key());
            ++it;
        } else {
            it = m_pending.erase(it);
            forgot = true;
        }
    }
    if (forgot)
        emit pendingChanged();
}

void SorobanQml::finish(int requestId)
{
    if (m_pending.remove(requestId))
        emit pendingChanged();
}

int SorobanQml::start(const QString& method, const QString& argument,
                      const QString& context)
{
    const int id = ++m_lastRequestId;
    m_pending.insert(id, method == QLatin1String("send"));
    emit pendingChanged();

    QMetaObject::invokeMethod(m_worker, "run", Qt::QueuedConnection,
                              Q_ARG(int, id), Q_ARG(QString, m_endpoint),
                              Q_ARG(QString, method), Q_ARG(QString, argument),
                              Q_ARG(QString, context), Q_ARG(int, m_endpointChanges));
    return id;
}

int SorobanQml::prepare(const QString& envelopeXdrBase64,
                        const QString& networkPassphrase)
{
    return start(QStringLiteral("prepare"), envelopeXdrBase64, networkPassphrase);
}

int SorobanQml::simulate(const QString& envelopeXdrBase64)
{
    if (envelopeXdrBase64.trimmed().isEmpty()) {
        const int id = ++m_lastRequestId;
        QVariantMap error;
        error.insert(QStringLiteral("kind"), QStringLiteral("validation"));
        error.insert(QStringLiteral("message"),
                     QStringLiteral("there is no transaction to simulate"));
        error.insert(QStringLiteral("mayHaveReachedServer"), false);
        // Queued, for the reason the other two facades already are: this
        // runs inside simulate(), which has not returned `id` yet, so a caller
        // filtering on the id it is about to be handed would drop this.
        QMetaObject::invokeMethod(this, [this, id, error]() {
            emit failed(id, error);
        }, Qt::QueuedConnection);
        return id;
    }
    return start(QStringLiteral("simulate"), envelopeXdrBase64);
}

int SorobanQml::send(const QString& envelopeXdrBase64)
{
    return start(QStringLiteral("send"), envelopeXdrBase64);
}

int SorobanQml::send(const QString& envelopeXdrBase64, const QString& networkPassphrase)
{
    // An empty passphrase would quietly be the one-argument send, which takes
    // whatever network the endpoint serves: refused rather than taken as that.
    if (networkPassphrase.trimmed().isEmpty()) {
        const int id = ++m_lastRequestId;
        QVariantMap error;
        error.insert(QStringLiteral("kind"), QStringLiteral("validation"));
        error.insert(QStringLiteral("message"),
                     QStringLiteral("not sent: the network passphrase is empty, and the "
                                    "transaction's hash depends on it"));
        error.insert(QStringLiteral("mayHaveReachedServer"), false);
        // Queued, as simulate()'s refusal is: the caller has not been handed
        // `id` yet.
        QMetaObject::invokeMethod(this, [this, id, error]() {
            emit failed(id, error);
        }, Qt::QueuedConnection);
        return id;
    }
    return start(QStringLiteral("send"), envelopeXdrBase64, networkPassphrase);
}

int SorobanQml::checkHealth()
{
    return start(QStringLiteral("health"), QString());
}

QVariantMap SorobanQml::readContract(const QUrl& fileUrl) const
{
    return readContractPath(fileUrl.isLocalFile() ? fileUrl.toLocalFile()
                                                  : fileUrl.toString());
}

QVariantMap SorobanQml::readContractPath(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QVariantMap out;
        out.insert(QStringLiteral("isWasm"), false);
        out.insert(QStringLiteral("problem"),
                   QStringLiteral("Could not open ") + path);
        return out;
    }
    // A contract is tens of kilobytes; a cap keeps a mistaken pick of a disk
    // image from being read into memory in full.
    const QByteArray wasm = file.read(16 * 1024 * 1024);
    file.close();

    return contractToMap(ContractSpec::read(wasm), wasm);
}

QVariantMap SorobanQml::readContractHex(const QString& hex) const
{
    const QByteArray wasm = QByteArray::fromHex(hex.trimmed().toLatin1());
    if (wasm.isEmpty() && !hex.trimmed().isEmpty()) {
        QVariantMap out;
        out.insert(QStringLiteral("isWasm"), false);
        out.insert(QStringLiteral("problem"),
                   QStringLiteral("That is not hex. Paste the bytes of the .wasm "
                                  "as hex, or open the file instead."));
        return out;
    }
    return contractToMap(ContractSpec::read(wasm), wasm);
}

QString SorobanQml::argumentTemplate(const QVariantMap& function) const
{
    QJsonArray args;
    for (const QVariant& entry : function.value(QStringLiteral("inputs")).toList()) {
        const QVariantMap input = entry.toMap();
        QJsonObject argument;
        const QString scvType = input.value(QStringLiteral("scvType")).toString();
        // When the contract wants something with no editor, the template keeps
        // the DECLARED type rather than substituting one that would build and
        // then be rejected by the host.
        argument.insert(QStringLiteral("type"),
                        scvType.isEmpty() ? input.value(QStringLiteral("type")).toString()
                                          : scvType);
        // null until the user fills it in: the builder refuses an argument
        // with no value, so an untouched skeleton cannot be signed as an
        // empty string or an empty container that nobody chose.
        argument.insert(QStringLiteral("value"), QJsonValue(QJsonValue::Null));
        args.append(argument);
    }
    return QString::fromUtf8(QJsonDocument(args).toJson(QJsonDocument::Compact));
}

void SorobanQml::onWorkerFinished(int requestId, const QString& kind,
                                  const QVariantMap& payload, bool started)
{
    if (!m_pending.contains(requestId))
        return;                         // not a send, and cancelAll() forgot it
    const bool cancelledSend = m_cancelledSends.remove(requestId);
    finish(requestId);
    // A send cancelAll() cancelled before the worker took it up was never
    // sent, and is dropped as that promises. One it had taken up is reported
    // whatever became of it: a cancel on the wire, before posting, or an end
    // of its own that came first.
    if (cancelledSend && !started)
        return;

    if (!kind.isEmpty()) {
        QVariantMap error = payload;
        error.insert(QStringLiteral("kind"), kind);
        emit failed(requestId, error);
        return;
    }

    // Checked first: a prepared envelope carries a resourceFee just as a
    // simulation does, so shape alone cannot tell them apart.
    if (payload.contains(QStringLiteral("preparedXdr")))
        emit prepared(requestId, payload);
    else if (payload.contains(QStringLiteral("healthy")))
        emit health(requestId, payload);
    else if (payload.contains(QStringLiteral("status")))
        emit sent(requestId, payload);
    else
        emit simulated(requestId, payload);
}

QSTELLAR_END_NS

#include "sorobanqml.moc"
