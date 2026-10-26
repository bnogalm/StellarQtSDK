#include "sorobanserver.h"
#include "restorefootprintoperation.h"
#include "extendfootprintttloperation.h"
#include <limits>

#include <QElapsedTimer>
#include <QEventLoop>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <cstring>
#include <stdexcept>

#include "exception/sorobanrpcexception.h"
#include "abstracttransaction.h"
#include "transaction.h"
#include "invokehostfunctionoperation.h"
#include "network.h"
#include "stellaraddress.h"
#include "asset.h"
#include "util.h"
#include "scval/scv.h"
#include "xdr/sorobanops.h"
#include "xdr/sorobantxdata.h"

#ifndef STELLAR_QT_REPLY_TIMEOUT
#define STELLAR_QT_REPLY_TIMEOUT 30000
#endif

QSTELLAR_BEGIN_NS

namespace {
using qstellar::exception::SorobanTransportException;

/** Set on a reply cancelRequests() aborted, so its caller can tell it apart
 *  from one the network or the timeout ended. */
const char* const CANCELLED_PROPERTY = "qstellarCancelled";

/** Base64 as XDR travels, decoded strictly: a character that is not base64
 *  is an error, where QByteArray::fromBase64 skipped it and decoded the rest. */
QByteArray strictBase64(const QString& text, const char* what,
                        const char* who = "applyTransactionSimulation")
{
    const auto decoded = QByteArray::fromBase64Encoding(
        text.toLatin1(), XDR_BASE64ENCODING | QByteArray::AbortOnBase64DecodingErrors);
    if (!decoded)
        throw std::runtime_error(std::string(who) + ": " + what + " is not base64");
    return *decoded;
}

/**
 * The network errors of a connection that never opened: nothing of the
 * request can have been sent on one. Every other error may come after the
 * request was written (a connection lost midway, an HTTP error status).
 */
bool neverConnected(QNetworkReply::NetworkError error)
{
    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::ProxyConnectionRefusedError:
    case QNetworkReply::ProxyNotFoundError:
    case QNetworkReply::SslHandshakeFailedError:
    case QNetworkReply::ProtocolUnknownError:
        return true;
    default:
        return false;
    }
}

/**
 * The answer to a sendTransaction Qt sent again, by itself, after it had
 * written it whole once (see SorobanServer::sendRequest): the answer to the
 * later send. It is taken only when it says the RPC has the transaction
 * (PENDING, DUPLICATE), which holds of the first send as well. Anything else
 * answers the later send only, while the RPC may have taken in, and applied,
 * the first: ERROR (a txBAD_SEQ most likely means the first was applied),
 * TRY_AGAIN_LATER, an error envelope, an answer that does not parse. That
 * throws SorobanTransportException, resent, which may have reached the server.
 */
QJsonValue resentSendAnswer(const QByteArray& payload)
{
    QString answered;
    try {
        const QJsonValue result = SorobanServer::parseRpcResult(payload);
        const SendTransactionResponse response = SendTransactionResponse::fromJson(result.toObject());
        switch (response.getStatus()) {
        case SendTransactionResponse::Status::PENDING:
        case SendTransactionResponse::Status::DUPLICATE:
            return result;
        case SendTransactionResponse::Status::TRY_AGAIN_LATER:
            answered = QStringLiteral("TRY_AGAIN_LATER (hash %1)").arg(response.getHash());
            break;
        case SendTransactionResponse::Status::ERROR_:
            answered = QStringLiteral("ERROR (hash %1, errorResultXdr %2)")
                           .arg(response.getHash(), response.getErrorResultXdr());
            break;
        }
    } catch (const std::exception& e) {
        answered = QStringLiteral("what is not an acknowledgement (%1)").arg(QString::fromUtf8(e.what()));
    }
    throw SorobanTransportException(
        QStringLiteral("Soroban RPC: sendTransaction was sent again by Qt, by itself, after the "
                       "connection that carried it whole closed with no answer, and the RPC answered "
                       "that later send with %1. That answers the later send only: the RPC may have "
                       "taken in, and applied, the first, so the outcome is not known; look the "
                       "transaction up by its hash before sending it again or reusing its sequence "
                       "number").arg(answered),
        true, true);
}

/** A ContractDataEntry as it stands in a LedgerEntryData. */
struct ContractDataEntry
{
    stellar::SCAddress contract;
    stellar::SCVal key;
    stellar::ContractDataDurability durability = stellar::ContractDataDurability::TEMPORARY;
    stellar::SCVal val;
};

/**
 * Decodes the base64 LedgerEntryData getLedgerEntries answers with, which
 * must hold a ContractDataEntry, exactly as stellar-xdr lays it out:
 *
 *   LedgerEntryType type (CONTRACT_DATA = 6)
 *   ContractDataEntry { ExtensionPoint ext (0); SCAddress contract;
 *                       SCVal key; ContractDataDurability durability;
 *                       SCVal val; }
 *
 * Strict base64, every byte read, or it throws.
 */
ContractDataEntry decodeContractData(const QString& base64, const char* who)
{
    QByteArray raw = strictBase64(base64, "the ledger entry", who);
    QDataStream s(&raw, QIODevice::ReadOnly);
    qint32 type = -1;
    s >> type;
    if (s.status() != QDataStream::Ok
            || type != static_cast<qint32>(stellar::LedgerEntryType::CONTRACT_DATA)) {
        throw std::runtime_error(std::string(who) + ": the ledger entry is not a "
                                 "CONTRACT_DATA LedgerEntryData");
    }
    ContractDataEntry entry;
    qint32 durability = -1;
    try {
        xdr::Reserved ext;              // ExtensionPoint: 0 is its only arm
        s >> ext >> entry.contract >> entry.key >> durability >> entry.val;
    } catch (const std::exception&) {
        // The SCVal and SCAddress readers throw on an arm they do not know.
        s.setStatus(QDataStream::ReadCorruptData);
    }
    if (s.status() != QDataStream::Ok || !s.atEnd()
            || (durability != static_cast<qint32>(stellar::ContractDataDurability::TEMPORARY)
                && durability != static_cast<qint32>(stellar::ContractDataDurability::PERSISTENT))) {
        throw std::runtime_error(std::string(who) + ": the ledger entry is not a "
                                 "ContractDataEntry");
    }
    entry.durability = static_cast<stellar::ContractDataDurability>(durability);
    return entry;
}

/** The symbol `name`, and nothing else. */
bool isSymbol(const stellar::SCVal& v, const char* name)
{
    return v.type == stellar::SCValType::SCV_SYMBOL && v.symbol == QByteArray(name);
}

/**
 * The SAC's balance entry: PERSISTENT, keyed ["Balance", address], valued
 * with its BalanceValue {amount: i128, authorized: bool, clawback: bool} —
 * a map of exactly those three, keys in that (sorted) order. Anything else is
 * not a balance, and throws: a flag that was missing or not a bool used to be
 * taken as its default, and entries under other keys were skipped.
 */
SACBalance sacBalanceFrom(const ContractDataEntry& entry, const char* who)
{
    auto refuse = [who](const char* what) {
        throw std::runtime_error(std::string(who) + ": " + what);
    };
    if (entry.durability != stellar::ContractDataDurability::PERSISTENT)
        refuse("the entry is not PERSISTENT, as a balance is");
    if (entry.key.type != stellar::SCValType::SCV_VEC || !entry.key.vec
            || entry.key.vec->size() != 2 || !isSymbol(entry.key.vec->at(0), "Balance")
            || entry.key.vec->at(1).type != stellar::SCValType::SCV_ADDRESS) {
        refuse("the entry is not keyed [\"Balance\", address]");
    }
    const stellar::SCVal& val = entry.val;
    if (val.type != stellar::SCValType::SCV_MAP || !val.map || val.map->size() != 3
            || !isSymbol(val.map->at(0).key, "amount")
            || !isSymbol(val.map->at(1).key, "authorized")
            || !isSymbol(val.map->at(2).key, "clawback")
            || val.map->at(0).val.type != stellar::SCValType::SCV_I128
            || val.map->at(1).val.type != stellar::SCValType::SCV_BOOL
            || val.map->at(2).val.type != stellar::SCValType::SCV_BOOL) {
        refuse("the entry's value is not the balance map {amount: i128, authorized: bool, "
               "clawback: bool}");
    }
    if (val.map->at(0).val.i128.hi < 0)
        refuse("the entry holds a negative amount, which no balance is");

    SACBalance out;
    out.amount = Scv::fromInt128(val.map->at(0).val);
    out.authorized = val.map->at(1).val.b;
    out.clawbackEnabled = val.map->at(2).val.b;
    out.exists = true;
    return out;
}

/**
 * The SAC balance in a getLedgerEntries answer to `key` (base64): none, or
 * exactly the one entry asked for, under that key, holding that balance.
 */
SACBalance sacBalanceFromAnswer(const GetLedgerEntriesResponse& resp, const QString& key,
                                const char* who)
{
    if (resp.getEntries().isEmpty()) {
        SACBalance empty;
        empty.amount = QByteArray(16, '\0');
        return empty;  // exists = false
    }

    // The one entry asked for, or the answer is not about this balance. The
    // first entry was taken, whatever it was and however many came.
    if (resp.getEntries().size() != 1)
        throw std::runtime_error(std::string(who) + ": the RPC answered with more than the one entry asked for");
    const LedgerEntryResult& found = resp.getEntries().first();
    const QByteArray asked = strictBase64(key, "the ledger key", who);
    if (strictBase64(found.key, "the answered key", who) != asked)
        throw std::runtime_error(std::string(who) + ": the RPC answered for another key than the one asked for");
    const ContractDataEntry entry = decodeContractData(found.xdr, who);
    stellar::SorobanLedgerKey entryKey;
    entryKey.type = stellar::LedgerEntryType::CONTRACT_DATA;
    entryKey.contractData.contract = entry.contract;
    entryKey.contractData.key = entry.key;
    entryKey.contractData.durability = entry.durability;
    if (xdr::encodeOrThrow(entryKey, (std::string(who) + ": the entry's key").c_str()) != asked)
        throw std::runtime_error(std::string(who) + ": the entry answered is not the one asked for");
    return sacBalanceFrom(entry, who);
}

/**
 * Whether `code` is an asset code a ledger can hold, as Core's isAssetValid
 * reads it: at least `minChars` ASCII letters or digits, then only zero bytes
 * to the end. Any other code names no asset, and looking it up would read as
 * no balance at all.
 */
bool isAssetCode(const qint8* code, int size, int minChars)
{
    int chars = 0;
    bool padding = false;
    for (int i = 0; i < size; ++i) {
        const char c = static_cast<char>(code[i]);
        if (c == '\0') {
            padding = true;
            continue;
        }
        const bool alnum = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
        if (padding || !alnum)
            return false;
        ++chars;
    }
    return chars >= minChars;
}
} // namespace

SorobanServer::SorobanServer(const QUrl& endpoint, QObject* parent)
    : QObject(parent), m_endpoint(endpoint), m_http(new QNetworkAccessManager(this)),
      m_requestTimeout(STELLAR_QT_REPLY_TIMEOUT)
{
    if (!endpoint.isValid()) {
        throw std::runtime_error("invalid Soroban RPC endpoint URL");
    }
}

SorobanServer::~SorobanServer() = default;

void SorobanServer::setRequestTimeout(int milliseconds)
{
    // 0 has no reading that is not a guess: no time at all, or no limit.
    if (milliseconds <= 0)
        throw std::invalid_argument("SorobanServer: the request timeout must be a positive "
                                    "number of milliseconds");
    m_requestTimeout = milliseconds;
}

void SorobanServer::cancelRequests()
{
    // Counted, so a pollTransaction under way sees it even when it lands
    // while no wait of its own is running: in the batch of events that ends
    // a poll's request, after its answer had already arrived.
    ++m_cancels;
    // A copy: aborting emits finished(), and the caller waiting on that reply
    // takes it out of m_inFlight as it wakes.
    const QList<QNetworkReply*> waiting = m_inFlight;
    for (QNetworkReply* reply : waiting) {
        reply->setProperty(CANCELLED_PROPERTY, true);
        reply->abort();
    }
    // And the waits between polls, which used to run to their end.
    for (QEventLoop* wait : m_pollWaits)
        wait->quit();
}

QByteArray SorobanServer::buildRpcRequest(const QString& method, const QJsonValue& params)
{
    QJsonObject envelope;
    envelope.insert("jsonrpc", QStringLiteral("2.0"));
    envelope.insert("id", QString::number(m_nextId++));
    envelope.insert("method", method);
    if (!params.isUndefined() && !params.isNull()) {
        envelope.insert("params", params);
    }
    return QJsonDocument(envelope).toJson(QJsonDocument::Compact);
}

QJsonValue SorobanServer::parseRpcResult(const QByteArray& jsonBytes)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        throw std::runtime_error(
            QString("Soroban RPC: invalid JSON envelope (%1)").arg(err.errorString()).toStdString());
    }
    QJsonObject obj = doc.object();
    if (obj.value("jsonrpc").toString() != QStringLiteral("2.0")) {
        throw std::runtime_error("Soroban RPC: missing or wrong `jsonrpc` field");
    }
    if (obj.contains("error")) {
        QJsonObject e = obj.value("error").toObject();
        const int code = e.value("code").toInt();
        const QString message = e.value("message").toString();
        const QJsonValue dataVal = e.value("data");
        QVariant data;
        if (!dataVal.isNull() && !dataVal.isUndefined()) {
            data = dataVal.toVariant();
        }
        throw qstellar::exception::SorobanRpcException(code, message, data);
    }
    if (!obj.contains("result")) {
        throw std::runtime_error("Soroban RPC: envelope has neither `result` nor `error`");
    }
    return obj.value("result");
}

QJsonValue SorobanServer::sendRequest(const QString& method, const QJsonValue& params)
{
    QNetworkRequest request(m_endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    QByteArray body = buildRpcRequest(method, params);

    QNetworkReply* reply = m_http->post(request, body);
    m_inFlight.append(reply);

    // Block on reply via a local event loop. Java SDK is sync; the Qt sync
    // wrapper keeps the API ergonomic for typical Soroban scripted flows.
    //
    // Bounded: the timer aborts a request that has had no complete answer
    // within m_requestTimeout, and cancelRequests() — which this loop delivers
    // when it is queued from another thread — aborts it at once. It used to
    // wait forever, and one stalled endpoint blocked SorobanQml's worker, and
    // every request queued behind it, for good.
    bool timedOut = false;
    // The most of the body the connection has reported sending. Reported in
    // order with the reply's error, so a connection error that arrives with
    // none sent is one that never carried the request.
    qint64 bytesSent = 0;
    // Whether the connection has written the body whole, and whether it then
    // wrote some of it again: Qt 5 sends a request again, by itself, on a new
    // connection, when the one it was written on closes before an answer,
    // and writes the body from the start. Whatever ends the request then came
    // of a later send, while the server may have read the first whole. Qt
    // says a body was written whole once per send, as (total, total),
    // however it throttles the steps before; the body is a byte array, so
    // the total is known. Whatever has Qt write the body again counts (a
    // redirect it follows with the body would too): on the safe side, as an
    // answer may then not be the first send's. It could not be told: an
    // answer to the later send was taken as the answer to the only one.
    bool writtenWhole = false;
    bool resent = false;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, reply, [reply, &timedOut]() {
        timedOut = true;
        reply->abort();
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    // `loop` as context: the connection ends with this call, the reply lives on.
    QObject::connect(reply, &QNetworkReply::uploadProgress, &loop,
                     [&bytesSent, &writtenWhole, &resent](qint64 sent, qint64 total) {
        bytesSent = qMax(bytesSent, sent);
        // A reply that ends with nothing written says so too, as (0, 0).
        if (sent <= 0)
            return;
        if (writtenWhole)
            resent = true;
        if (sent == total)
            writtenWhole = true;
    });
    if (!reply->isFinished()) {
        timer.start(m_requestTimeout);
        loop.exec();
    }
    timer.stop();
    m_inFlight.removeOne(reply);

    // Not finished means the loop was ended from outside (its thread is
    // quitting): whatever arrived so far is not an answer.
    const bool finished = reply->isFinished();
    if (!finished)
        reply->abort();
    const QNetworkReply::NetworkError netErr = reply->error();
    const QString netErrString = reply->errorString();
    // Only an abort that really cut the reply short counts: one that came
    // after the answer had arrived in full found nothing to abort.
    const bool aborted = netErr == QNetworkReply::OperationCanceledError;
    const bool cancelled = !finished || (aborted && reply->property(CANCELLED_PROPERTY).toBool());
    // An aborted reply is closed, and there is nothing in it to read.
    const QByteArray payload = aborted ? QByteArray() : reply->readAll();
    reply->deleteLater();

    // A cancel or a timeout may have come after the request was written, and
    // which it was cannot be told at the moment of the abort. Only a
    // connection that never opened, with nothing sent, is known not to have
    // reached the server. These were all plain runtime_errors, so a caller
    // could not tell a send that never left from one that may be applied.
    // Each says, too, when Qt had sent the request again (resent()).
    const QString again = resent
        ? QStringLiteral("; Qt had sent it again, by itself, after the connection that carried "
                         "it whole closed with no answer")
        : QString();
    if (cancelled) {
        throw SorobanTransportException(
            QString("Soroban RPC request cancelled before its answer arrived (%1); the "
                    "server may still have received it%2").arg(method, again), true, resent);
    }
    if (aborted && timedOut) {
        throw SorobanTransportException(
            QString("Soroban RPC HTTP error: no answer within %1 ms (%2); the server may "
                    "still have received it%3").arg(m_requestTimeout).arg(method, again),
            true, resent);
    }
    if (netErr != QNetworkReply::NoError) {
        const bool notSent = neverConnected(netErr) && bytesSent <= 0;
        throw SorobanTransportException(
            QString("Soroban RPC HTTP error: %1 (%2); %3%4").arg(netErrString, method,
                    notSent ? QStringLiteral("the request was not sent")
                            : QStringLiteral("the server may have received it"), again),
            !notSent, resent);
    }
    // The answer to a later send. A sendTransaction's is not its answer
    // unless it says the RPC has the transaction (resentSendAnswer()).
    if (resent && method == QLatin1String("sendTransaction"))
        return resentSendAnswer(payload);
    return parseRpcResult(payload);
}

GetHealthResponse SorobanServer::getHealth()
{
    return GetHealthResponse::fromJson(sendRequest("getHealth").toObject());
}

GetNetworkResponse SorobanServer::getNetwork()
{
    return GetNetworkResponse::fromJson(sendRequest("getNetwork").toObject());
}

GetVersionInfoResponse SorobanServer::getVersionInfo()
{
    return GetVersionInfoResponse::fromJson(sendRequest("getVersionInfo").toObject());
}

GetLatestLedgerResponse SorobanServer::getLatestLedger()
{
    return GetLatestLedgerResponse::fromJson(sendRequest("getLatestLedger").toObject());
}

GetFeeStatsResponse SorobanServer::getFeeStats()
{
    return GetFeeStatsResponse::fromJson(sendRequest("getFeeStats").toObject());
}

GetTransactionResponse SorobanServer::getTransaction(const QString& hash)
{
    QJsonObject params;
    params.insert("hash", hash);
    const GetTransactionResponse r =
        GetTransactionResponse::fromJson(sendRequest("getTransaction", params).toObject());
    // The RPC answers about the hash asked for, and echoes it, found or not:
    // an answer about another transaction used to be taken for this one's,
    // and pollTransaction returned its outcome.
    if (r.getTxHash().compare(hash, Qt::CaseInsensitive) != 0) {
        throw std::runtime_error(QStringLiteral("Soroban RPC: getTransaction answered about %1, "
                                                "not about the %2 asked for")
                                     .arg(r.getTxHash(), hash).toStdString());
    }
    return r;
}

GetLedgerEntriesResponse SorobanServer::getLedgerEntries(const QStringList& base64Keys)
{
    QJsonObject params;
    QJsonArray keys;
    for (const QString& k : base64Keys) keys.append(k);
    params.insert("keys", keys);
    return GetLedgerEntriesResponse::fromJson(sendRequest("getLedgerEntries", params).toObject());
}

GetLedgersResponse SorobanServer::getLedgers(quint32 startLedger, const QString& cursor, qint32 limit)
{
    QJsonObject params;
    if (startLedger > 0) params.insert("startLedger", static_cast<qint64>(startLedger));
    QJsonObject pagination;
    if (!cursor.isEmpty()) pagination.insert("cursor", cursor);
    if (limit > 0)         pagination.insert("limit", limit);
    if (!pagination.isEmpty()) params.insert("pagination", pagination);
    return GetLedgersResponse::fromJson(sendRequest("getLedgers", params).toObject());
}

GetTransactionsResponse SorobanServer::getTransactions(quint32 startLedger, const QString& cursor, qint32 limit)
{
    QJsonObject params;
    if (startLedger > 0) params.insert("startLedger", static_cast<qint64>(startLedger));
    QJsonObject pagination;
    if (!cursor.isEmpty()) pagination.insert("cursor", cursor);
    if (limit > 0)         pagination.insert("limit", limit);
    if (!pagination.isEmpty()) params.insert("pagination", pagination);
    return GetTransactionsResponse::fromJson(sendRequest("getTransactions", params).toObject());
}

GetEventsResponse SorobanServer::getEvents(quint32 startLedger, quint32 endLedger,
                                           const QJsonArray& filters,
                                           const QString& cursor, qint32 limit)
{
    QJsonObject params;
    if (startLedger > 0) params.insert("startLedger", static_cast<qint64>(startLedger));
    if (endLedger > 0)   params.insert("endLedger",   static_cast<qint64>(endLedger));
    if (!filters.isEmpty()) params.insert("filters", filters);
    QJsonObject pagination;
    if (!cursor.isEmpty()) pagination.insert("cursor", cursor);
    if (limit > 0)         pagination.insert("limit", limit);
    if (!pagination.isEmpty()) params.insert("pagination", pagination);
    return GetEventsResponse::fromJson(sendRequest("getEvents", params).toObject());
}

SendTransactionResponse SorobanServer::sendTransaction(AbstractTransaction* tx)
{
    if (!tx) throw std::runtime_error("sendTransaction: null transaction");
    return sendTransactionXdr(tx->toEnvelopeXdrBase64());
}

SendTransactionResponse SorobanServer::sendTransactionXdr(const QString& envelopeXdrBase64)
{
    QJsonObject params;
    params.insert("transaction", envelopeXdrBase64);
    return SendTransactionResponse::fromJson(sendRequest("sendTransaction", params).toObject());
}

SimulateTransactionResponse SorobanServer::simulateTransaction(AbstractTransaction* tx)
{
    if (!tx) throw std::runtime_error("simulateTransaction: null transaction");
    return simulateTransactionXdr(tx->toEnvelopeXdrBase64());
}

SimulateTransactionResponse SorobanServer::simulateTransactionXdr(const QString& envelopeXdrBase64)
{
    return simulateTransactionXdr(envelopeXdrBase64, SimulateTransactionOptions());
}

SimulateTransactionResponse SorobanServer::simulateTransaction(AbstractTransaction* tx,
                                                               const SimulateTransactionOptions& options)
{
    if (!tx) throw std::runtime_error("simulateTransaction: null transaction");
    return simulateTransactionXdr(tx->toEnvelopeXdrBase64(), options);
}

SimulateTransactionResponse SorobanServer::simulateTransactionXdr(const QString& envelopeXdrBase64,
                                                                  const SimulateTransactionOptions& options)
{
    QJsonObject params;
    params.insert("transaction", envelopeXdrBase64);
    // Each one only when asked for, under js's names. There was no way to
    // send any of them: a call that costs more on chain than simulated could
    // not be padded, nor non-root authorization recorded.
    if (options.instructionLeeway > 0) {
        QJsonObject resourceConfig;
        resourceConfig.insert("instructionLeeway", static_cast<qint64>(options.instructionLeeway));
        params.insert("resourceConfig", resourceConfig);
    }
    switch (options.authMode) {
    case SimulateTransactionOptions::AuthMode::Default:
        break;
    case SimulateTransactionOptions::AuthMode::Enforce:
        params.insert("authMode", QStringLiteral("enforce"));
        break;
    case SimulateTransactionOptions::AuthMode::Record:
        params.insert("authMode", QStringLiteral("record"));
        break;
    case SimulateTransactionOptions::AuthMode::RecordAllowNonroot:
        params.insert("authMode", QStringLiteral("record_allow_nonroot"));
        break;
    }
    if (options.useUpgradedAuth != SimulateTransactionOptions::UpgradedAuth::NotSent) {
        params.insert("useUpgradedAuth",
                      options.useUpgradedAuth == SimulateTransactionOptions::UpgradedAuth::Requested);
    }
    return SimulateTransactionResponse::fromJson(sendRequest("simulateTransaction", params).toObject());
}

// ── Iter 4 helpers ─────────────────────────────────────────────────────

QString SorobanServer::buildSACBalanceLedgerKey(const QString& accountId,
                                                const stellar::Asset& asset,
                                                const QString& networkPassphrase)
{
    // key = SCV_VEC([ SCV_SYMBOL("Balance"), SCV_ADDRESS(accountId) ])
    QList<stellar::SCVal> elems;
    elems.append(Scv::toSymbol(QStringLiteral("Balance")));
    elems.append(Scv::toAddress(accountId));
    stellar::SCVal vecKey = Scv::toVec(elems);

    // contract = the SAC contract id for `asset` on this network. It is the
    // hash of the asset's XDR, so an asset XDR has no encoding for (a pool
    // share is not an Asset) is refused here: its partial encoding used to be
    // hashed into the id of a contract that cannot exist, and the balance of
    // anything there read as zero.
    xdr::encodeOrThrow(asset, "buildSACBalanceLedgerKey: the asset");
    const QByteArray sacId = Util::getContractAddressFromAsset(networkPassphrase, asset);
    if (sacId.size() != 32) {
        throw std::runtime_error("buildSACBalanceLedgerKey: SAC contract id length != 32");
    }
    stellar::SCAddress contract;
    contract.type = stellar::SCAddressType::SC_ADDRESS_TYPE_CONTRACT;
    std::memcpy(contract.contractId, sacId.constData(), 32);

    stellar::SorobanLedgerKey key;
    key.type = stellar::LedgerEntryType::CONTRACT_DATA;
    key.contractData.contract = contract;
    key.contractData.key = vecKey;
    key.contractData.durability = stellar::ContractDataDurability::PERSISTENT;

    // Sent as the key to look up: whole, or not sent.
    const QByteArray buf = xdr::encodeOrThrow(key, "buildSACBalanceLedgerKey: the ledger key");
    return QString::fromLatin1(buf.toBase64(XDR_BASE64ENCODING));
}

SACBalance SorobanServer::parseSACBalanceFromLedgerEntryXdr(const QString& base64LedgerEntry)
{
    // A LedgerEntryData, as getLedgerEntries answers: this read a LedgerEntry
    // prefix (lastModifiedLedgerSeq) the answer does not have and skipped the
    // ContractDataEntry's ext, so every real entry was refused as not
    // contract data — and a buffer shaped the wrong way was read as one.
    const char* const who = "parseSACBalanceFromLedgerEntryXdr";
    return sacBalanceFrom(decodeContractData(base64LedgerEntry, who), who);
}

SACBalance SorobanServer::getSACBalance(const QString& accountId,
                                        const stellar::Asset& asset,
                                        Network* network)
{
    if (!network) throw std::runtime_error("getSACBalance: null network");
    // An account's balance of an asset contract lives in its trustline (its
    // XLM in the account entry), never under this key: looking it up read
    // every account's balance as zero.
    if (StellarAddress::fromString(accountId).getType() != StellarAddress::Type::CONTRACT) {
        throw std::runtime_error("getSACBalance: only a contract (C...) keeps an asset contract "
                                 "balance in contract data; an account's is its trustline (or, "
                                 "for XLM, its account) balance, which this does not read");
    }
    const QString key = buildSACBalanceLedgerKey(accountId, asset, network->getNetworkPassphrase());
    return sacBalanceFromAnswer(getLedgerEntries(QStringList{key}), key, "getSACBalance");
}

AssetBalance SorobanServer::getAssetBalance(const QString& address,
                                            const stellar::Asset& asset,
                                            Network* network)
{
    const char* const who = "getAssetBalance";
    auto refuse = [who](const char* what) {
        throw std::runtime_error(std::string(who) + ": " + what);
    };

    // An asset no ledger can hold is refused: looked up, it would read as no
    // balance at all rather than as the mistake it is.
    switch (asset.type) {
    case stellar::AssetType::ASSET_TYPE_NATIVE:
        break;
    case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
        if (!isAssetCode(asset.alphaNum4.assetCode, 4, 1))
            refuse("the asset code is not 1 to 4 letters or digits padded with zero bytes");
        break;
    case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
        if (!isAssetCode(asset.alphaNum12.assetCode, 12, 5))
            refuse("the asset code is not 5 to 12 letters or digits padded with zero bytes");
        break;
    default:
        refuse("the asset is neither XLM nor an issued asset (a pool share is not an asset balance)");
    }

    const StellarAddress holder = StellarAddress::fromString(address);
    switch (holder.getType()) {
    case StellarAddress::Type::ACCOUNT:
        break;
    case StellarAddress::Type::CONTRACT: {
        if (!network)
            refuse("null network: a contract's balance is kept by the asset's contract, whose id "
                   "depends on the network");
        const QString key = buildSACBalanceLedgerKey(address, asset, network->getNetworkPassphrase());
        const GetLedgerEntriesResponse resp = getLedgerEntries(QStringList{key});
        const SACBalance sac = sacBalanceFromAnswer(resp, key, who);
        AssetBalance out;
        out.amount = sac.amount;
        out.latestLedger = resp.getLatestLedger();
        if (sac.exists) {
            out.exists = true;
            out.authorized = sac.authorized;
            out.clawbackEnabled = sac.clawbackEnabled;
        }
        return out;
    }
    case StellarAddress::Type::MUXED_ACCOUNT:
        // A muxed address is its account plus an id the ledger keeps nothing
        // under: no entry of its own, and no balance but its account's.
        refuse("a muxed account (M...) has no balance of its own: ask for the balance of its "
               "account (G...), which holds it");
        break;
    default:
        refuse("only an account (G...) or a contract (C...) holds an asset balance");
    }

    // The account entry for XLM, the trustline for an issued asset.
    const bool native = asset.type == stellar::AssetType::ASSET_TYPE_NATIVE;
    const stellar::AccountID holderId = stellar::PublicKey::from(holder.getBytes());
    stellar::LedgerKey key;
    if (native) {
        key.fillAccount().accountID = holderId;
    } else {
        stellar::LedgerKey::TrustLine& line = key.fillTrustLine();
        line.accountID = holderId;
        line.asset = stellar::TrustLineAsset(asset);
    }
    const QByteArray asked = xdr::encodeOrThrow(key, "getAssetBalance: the ledger key");
    const GetLedgerEntriesResponse resp =
        getLedgerEntries(QStringList{ QString::fromLatin1(asked.toBase64(XDR_BASE64ENCODING)) });

    AssetBalance out;
    out.amount = QByteArray(16, '\0');
    out.latestLedger = resp.getLatestLedger();
    if (resp.getEntries().isEmpty())
        return out;                     // exists = false

    // The one entry asked for, under the key asked for, decoded whole.
    if (resp.getEntries().size() != 1)
        refuse("the RPC answered with more than the one entry asked for");
    const LedgerEntryResult& found = resp.getEntries().first();
    if (strictBase64(found.key, "the answered key", who) != asked)
        refuse("the RPC answered for another key than the one asked for");
    QByteArray raw = strictBase64(found.xdr, "the ledger entry", who);
    QDataStream s(&raw, QIODevice::ReadOnly);
    // LedgerEntryData: the entry type, then that entry.
    qint32 type = -1;
    s >> type;
    stellar::AccountEntry account;
    stellar::TrustLineEntry trustLine;
    stellar::LedgerKey entryKey;
    try {
        if (native && type == static_cast<qint32>(stellar::LedgerEntryType::ACCOUNT)) {
            s >> account;
            entryKey.fillAccount().accountID = account.accountID;
        } else if (!native && type == static_cast<qint32>(stellar::LedgerEntryType::TRUSTLINE)) {
            s >> trustLine;
            stellar::LedgerKey::TrustLine& line = entryKey.fillTrustLine();
            line.accountID = trustLine.accountID;
            line.asset = trustLine.asset;
        } else {
            s.setStatus(QDataStream::ReadCorruptData);
        }
    } catch (const std::exception&) {
        // The array readers throw on a length or padding that is not XDR's.
        s.setStatus(QDataStream::ReadCorruptData);
    }
    if (s.status() != QDataStream::Ok || !s.atEnd()) {
        refuse(native ? "the ledger entry is not an AccountEntry LedgerEntryData"
                      : "the ledger entry is not a TrustLineEntry LedgerEntryData");
    }
    QByteArray entryKeyXdr;
    try {
        entryKeyXdr = xdr::encodeOrThrow(entryKey, "getAssetBalance: the entry's key");
    } catch (const std::exception&) {
        refuse("the entry answered is not the one asked for");
    }
    if (entryKeyXdr != asked)
        refuse("the entry answered is not the one asked for");

    qint64 balance = 0;
    stellar::Liabilities liabilities = {};
    if (native) {
        balance = account.balance;
        if (account.ext.reserved == 1)
            liabilities = account.v1.liabilities;
        // XLM has no trustline: nobody can deauthorize or claw it back.
        out.authorized = true;
    } else {
        balance = trustLine.balance;
        if (trustLine.ext.reserved == 1)
            liabilities = trustLine.v1.liabilities;
        const quint32 flags = trustLine.flags;
        const quint32 authorized = quint32(stellar::TrustLineFlags::AUTHORIZED_FLAG);
        const quint32 maintain = quint32(stellar::TrustLineFlags::AUTHORIZED_TO_MAINTAIN_LIABILITIES_FLAG);
        const quint32 clawback = quint32(stellar::TrustLineFlags::TRUSTLINE_CLAWBACK_ENABLED_FLAG);
        // Only the flags Core defines (MASK_TRUSTLINE_FLAGS_V17), and never
        // both authorizations, which Core does not allow: anything else would
        // be reported as a meaning nobody gave it.
        if (flags & ~(authorized | maintain | clawback))
            refuse("the trustline carries a flag Core does not define");
        if ((flags & authorized) && (flags & maintain))
            refuse("the trustline is both authorized and authorized to maintain liabilities, "
                   "which Core does not allow");
        out.authorized = (flags & authorized) != 0;
        out.authorizedToMaintainLiabilities = (flags & maintain) != 0;
        out.clawbackEnabled = (flags & clawback) != 0;
    }
    if (balance < 0)
        refuse("the entry holds a negative balance, which no balance is");
    if (liabilities.buying < 0 || liabilities.selling < 0)
        refuse("the entry holds negative liabilities, which no liabilities are");
    out.amount = Scv::fromInt128(Scv::toInt128(balance));
    out.buyingLiabilities = liabilities.buying;
    out.sellingLiabilities = liabilities.selling;
    out.exists = true;
    return out;
}

void SorobanServer::applyTransactionSimulation(Transaction* tx, const SimulateTransactionResponse& sim)
{
    if (!tx) throw std::runtime_error("applyTransactionSimulation: null transaction");
    // A Soroban transaction is one invokeHostFunction, extendFootprintTtl or
    // restoreFootprint operation, as js assembleTransaction requires: soroban
    // data on anything else was signed, and core refuses the transaction.
    if (tx->getOperations().size() != 1
            || (!dynamic_cast<InvokeHostFunctionOperation*>(tx->getOperations().first())
                && !dynamic_cast<ExtendFootprintTTLOperation*>(tx->getOperations().first())
                && !dynamic_cast<RestoreFootprintOperation*>(tx->getOperations().first()))) {
        throw std::runtime_error("applyTransactionSimulation: a Soroban transaction has exactly one "
                                 "invokeHostFunction, extendFootprintTtl or restoreFootprint operation");
    }

    // Everything is decoded into locals and committed together at the end, so
    // a failure anywhere leaves the transaction exactly as it was.

    // The transaction's own fee and footprint first: the bid is recovered from
    // them below, and a value that makes the recovered bid wrong is refused
    // before any arithmetic. A negative resource fee was subtracted as it
    // came: fee 100 with -4000000000 recovered a bid of 4000000100 and was
    // signed as 4000000400 stroops, and INT64_MIN overflowed.
    if (tx->m_fee < 0 || tx->m_fee > static_cast<qint64>(std::numeric_limits<quint32>::max())) {
        throw std::runtime_error("applyTransactionSimulation: the transaction's fee is not a "
                                 "transaction fee (0 to 4294967295 stroops)");
    }
    if (tx->m_sorobanData && tx->m_sorobanData->resourceFee < 0) {
        throw std::runtime_error("applyTransactionSimulation: the transaction's own soroban "
                                 "data carries a negative resource fee");
    }

    // The simulated SorobanTransactionData, decoded in full or refused: a
    // stream that fails or has bytes left over is not a footprint anybody
    // computed.
    QSharedPointer<stellar::SorobanTransactionData> newData;
    if (!sim.getTransactionData().isEmpty()) {
        // Strictly, as AssembledTransaction::result() decodes: a character
        // that is not base64 used to be skipped and the rest decoded.
        QByteArray raw = strictBase64(sim.getTransactionData(), "transactionData");
        QDataStream s(&raw, QIODevice::ReadOnly);
        newData = QSharedPointer<stellar::SorobanTransactionData>::create();
        s >> *newData;
        if (s.status() != QDataStream::Ok || !s.atEnd()) {
            throw std::runtime_error("applyTransactionSimulation: transactionData is not a "
                                     "SorobanTransactionData");
        }
        if (newData->resourceFee < 0) {
            throw std::runtime_error("applyTransactionSimulation: transactionData carries a "
                                     "negative resource fee");
        }
    }

    // minResourceFee is a number from a remote server: present means a
    // non-negative whole number, or the response is malformed. It is checked,
    // not charged — see below.
    const QString minText = sim.getMinResourceFee().trimmed();
    if (!minText.isEmpty()) {
        bool ok = false;
        const qint64 minFee = minText.toLongLong(&ok);
        if (!ok || minFee < 0) {
            throw std::runtime_error("applyTransactionSimulation: minResourceFee is not a "
                                     "non-negative whole number");
        }
    }

    // The fee is the inclusion fee the caller bid plus the resource fee of the
    // SorobanTransactionData the transaction carries once this returns, as
    // js-stellar-sdk's assembleTransaction computes it: the one resource fee
    // that is added is the one that goes into the envelope, so taking it out
    // again recovers the bid exactly, however often this runs. It used to add
    // minResourceFee and take out the footprint's resourceFee: an RPC that
    // sent the two differently moved the bid on every re-simulation. With no
    // new footprint the transaction keeps its own, and its fee.
    qint64 newFee = tx->m_fee;
    if (newData) {
        qint64 inclusionFee = tx->m_fee;
        if (tx->m_sorobanData) {
            // A fee below the resource fee its own footprint declares has no
            // inclusion fee to recover: refused rather than guessed at.
            if (tx->m_fee < tx->m_sorobanData->resourceFee) {
                throw std::runtime_error("applyTransactionSimulation: the transaction's fee is "
                                         "less than the resource fee of its own soroban data");
            }
            inclusionFee -= tx->m_sorobanData->resourceFee;
        }
        // Never a total the envelope's uint32 fee field cannot hold: toV1Xdr
        // used to truncate one, signing one fee while getFee() reported another.
        if (newData->resourceFee > static_cast<qint64>(std::numeric_limits<quint32>::max()) - inclusionFee) {
            throw std::runtime_error("applyTransactionSimulation: the simulated resource fee "
                                     "does not fit a transaction fee (4294967295 stroops at most)");
        }
        newFee = inclusionFee + newData->resourceFee;
    }

    // Fill auth on the first InvokeHostFunctionOperation that has none, if
    // the simulation produced any. Stellar core derives a SOURCE_ACCOUNT
    // credentials entry implicitly, so only auto-fill when the user left
    // it empty.
    InvokeHostFunctionOperation* authTarget = nullptr;
    QList<stellar::SorobanAuthorizationEntry> entries;
    if (!sim.getResults().isEmpty()) {
        const auto& authBase64 = sim.getResults().first().auth;
        if (!authBase64.isEmpty()) {
            for (Operation* op : tx->m_operations) {
                auto* invoke = dynamic_cast<InvokeHostFunctionOperation*>(op);
                if (!invoke) continue;
                if (!invoke->getAuth().isEmpty()) break;
                for (const QString& enc : authBase64) {
                    QByteArray raw = strictBase64(enc, "an auth entry");
                    QDataStream s(&raw, QIODevice::ReadOnly);
                    stellar::SorobanAuthorizationEntry e;
                    s >> e;
                    // A truncated entry decodes as a default-filled one, and it
                    // would be installed on the operation and signed.
                    if (s.status() != QDataStream::Ok || !s.atEnd()) {
                        throw std::runtime_error("applyTransactionSimulation: an auth entry "
                                                 "is not a SorobanAuthorizationEntry");
                    }
                    entries.append(e);
                }
                authTarget = invoke;
                break;
            }
        }
    }

    // Commit: nothing above has touched the transaction.
    if (newData)
        tx->m_sorobanData = newData;
    if (authTarget)
        authTarget->setAuth(entries);
    tx->m_fee = newFee;
}

void SorobanServer::prepareTransaction(Transaction* tx)
{
    if (!tx) throw std::runtime_error("prepareTransaction: null transaction");
    SimulateTransactionResponse sim = simulateTransaction(tx);
    if (sim.isError()) {
        throw std::runtime_error(
            QString("prepareTransaction: simulation failed: %1").arg(sim.getError()).toStdString());
    }
    applyTransactionSimulation(tx, sim);
}

void SorobanServer::prepareTransaction(Transaction* tx, const SimulateTransactionOptions& options)
{
    if (!tx) throw std::runtime_error("prepareTransaction: null transaction");
    SimulateTransactionResponse sim = simulateTransaction(tx, options);
    if (sim.isError()) {
        throw std::runtime_error(
            QString("prepareTransaction: simulation failed: %1").arg(sim.getError()).toStdString());
    }
    applyTransactionSimulation(tx, sim);
}

GetTransactionResponse SorobanServer::pollTransaction(const QString& hash, int timeoutMs, int intervalMs)
{
    if (intervalMs < 0) intervalMs = 0;
    // A cancelRequests() made from here on ends the poll, wherever it lands:
    // in a poll's request (which then throws), in the wait between two, or
    // after a poll's answer had arrived. The wait used to run to its end
    // whatever was cancelled, and the caller waited out the whole timeout.
    const quint64 cancelsBefore = m_cancels;
    auto throwIfCancelled = [&]() {
        if (m_cancels != cancelsBefore) {
            throw qstellar::exception::SorobanTransportException(
                QString("pollTransaction: cancelled while waiting for %1; the transaction "
                        "may still be applied: look it up by its hash").arg(hash), true);
        }
    };
    QElapsedTimer clock;
    clock.start();
    while (true) {
        GetTransactionResponse r = fetchTransaction(hash);
        if (r.getStatus() != GetTransactionResponse::Status::NOT_FOUND) {
            return r;
        }
        throwIfCancelled();
        if (clock.elapsed() >= timeoutMs) {
            throw std::runtime_error(
                QString("pollTransaction: timed out after %1 ms waiting for %2")
                    .arg(timeoutMs).arg(hash).toStdString());
        }
        // Yield without sleeping the GUI thread by spinning a tiny event
        // loop terminated by a single-shot timer, or by cancelRequests().
        QEventLoop wait;
        QTimer::singleShot(intervalMs, &wait, &QEventLoop::quit);
        m_pollWaits.append(&wait);
        wait.exec();
        m_pollWaits.removeOne(&wait);
        throwIfCancelled();
    }
}

QSTELLAR_END_NS
