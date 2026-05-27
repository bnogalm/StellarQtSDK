#include "sorobanserver.h"

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

SorobanServer::SorobanServer(const QUrl& endpoint, QObject* parent)
    : QObject(parent), m_endpoint(endpoint), m_http(new QNetworkAccessManager(this))
{
    if (!endpoint.isValid()) {
        throw std::runtime_error("invalid Soroban RPC endpoint URL");
    }
}

SorobanServer::~SorobanServer() = default;

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

    // Block on reply via a local event loop. Java SDK is sync; the Qt sync
    // wrapper keeps the API ergonomic for typical Soroban scripted flows.
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const QByteArray payload = reply->readAll();
    const QNetworkReply::NetworkError netErr = reply->error();
    const QString netErrString = reply->errorString();
    reply->deleteLater();

    if (netErr != QNetworkReply::NoError) {
        throw std::runtime_error(
            QString("Soroban RPC HTTP error: %1").arg(netErrString).toStdString());
    }
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
    return GetTransactionResponse::fromJson(sendRequest("getTransaction", params).toObject());
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
    QJsonObject params;
    params.insert("transaction", envelopeXdrBase64);
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

    // contract = the SAC contract id for `asset` on this network
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

    QByteArray buf;
    QDataStream s(&buf, QIODevice::WriteOnly);
    s << key;
    return QString::fromLatin1(buf.toBase64(XDR_BASE64ENCODING));
}

SACBalance SorobanServer::parseSACBalanceFromLedgerEntryXdr(const QString& base64LedgerEntry)
{
    QByteArray raw = QByteArray::fromBase64(base64LedgerEntry.toLatin1(), XDR_BASE64ENCODING);
    QDataStream s(&raw, QIODevice::ReadOnly);

    // LedgerEntry: { lastModifiedLedgerSeq:uint32, data:union, ext:union }
    quint32 lastModified = 0;
    s >> lastModified;
    qint32 entryType = 0;
    s >> entryType;
    if (static_cast<stellar::LedgerEntryType>(entryType) != stellar::LedgerEntryType::CONTRACT_DATA) {
        throw std::runtime_error("parseSACBalanceFromLedgerEntryXdr: not a CONTRACT_DATA entry");
    }

    // ContractDataEntry: { contract:SCAddress, key:SCVal, durability:i32, val:SCVal }
    stellar::SCAddress contract;
    stellar::SCVal scKey;
    qint32 durability = 0;
    stellar::SCVal scVal;
    s >> contract;
    s >> scKey;
    s >> durability;
    s >> scVal;

    // The SAC balance entry stores `val` as SCV_MAP with three keys:
    //   "amount" (i128), "authorized" (bool), "clawback" (bool)
    QList<stellar::SCMapEntry> entries = Scv::fromMap(scVal);
    SACBalance out;
    out.exists = true;
    for (const auto& e : entries) {
        if (e.key.type != stellar::SCValType::SCV_SYMBOL) continue;
        const QString k = QString::fromUtf8(e.key.symbol);
        if (k == QLatin1String("amount")) {
            out.amount = Scv::fromInt128(e.val);
        } else if (k == QLatin1String("authorized")) {
            out.authorized = (e.val.type == stellar::SCValType::SCV_BOOL) ? e.val.b : true;
        } else if (k == QLatin1String("clawback")) {
            out.clawbackEnabled = (e.val.type == stellar::SCValType::SCV_BOOL) ? e.val.b : false;
        }
    }
    return out;
}

SACBalance SorobanServer::getSACBalance(const QString& accountId,
                                        const stellar::Asset& asset,
                                        Network* network)
{
    if (!network) throw std::runtime_error("getSACBalance: null network");
    const QString key = buildSACBalanceLedgerKey(accountId, asset, network->getNetworkPassphrase());
    GetLedgerEntriesResponse resp = getLedgerEntries(QStringList{key});
    if (resp.getEntries().isEmpty()) {
        SACBalance empty;
        empty.amount = QByteArray(16, '\0');
        return empty;  // exists = false
    }
    return parseSACBalanceFromLedgerEntryXdr(resp.getEntries().first().xdr);
}

void SorobanServer::applyTransactionSimulation(Transaction* tx, const SimulateTransactionResponse& sim)
{
    if (!tx) throw std::runtime_error("applyTransactionSimulation: null transaction");

    // Decode the simulated SorobanTransactionData and attach it to the tx.
    if (!sim.getTransactionData().isEmpty()) {
        QByteArray raw = QByteArray::fromBase64(sim.getTransactionData().toLatin1(), XDR_BASE64ENCODING);
        QDataStream s(&raw, QIODevice::ReadOnly);
        auto data = QSharedPointer<stellar::SorobanTransactionData>::create();
        s >> *data;
        tx->m_sorobanData = data;
    }

    // Fill auth on the first InvokeHostFunctionOperation that has none, if
    // the simulation produced any. Stellar core derives a SOURCE_ACCOUNT
    // credentials entry implicitly, so only auto-fill when the user left
    // it empty.
    if (!sim.getResults().isEmpty()) {
        const auto& authBase64 = sim.getResults().first().auth;
        if (!authBase64.isEmpty()) {
            for (Operation* op : tx->m_operations) {
                auto* invoke = dynamic_cast<InvokeHostFunctionOperation*>(op);
                if (!invoke) continue;
                if (!invoke->getAuth().isEmpty()) break;
                QList<stellar::SorobanAuthorizationEntry> entries;
                for (const QString& enc : authBase64) {
                    QByteArray raw = QByteArray::fromBase64(enc.toLatin1(), XDR_BASE64ENCODING);
                    QDataStream s(&raw, QIODevice::ReadOnly);
                    stellar::SorobanAuthorizationEntry e;
                    s >> e;
                    entries.append(e);
                }
                invoke->setAuth(entries);
                break;
            }
        }
    }

    // Bump the fee by the minResourceFee returned by the simulation.
    bool ok = false;
    const qint64 resFee = sim.getMinResourceFee().toLongLong(&ok);
    if (ok) tx->m_fee += resFee;
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

GetTransactionResponse SorobanServer::pollTransaction(const QString& hash, int timeoutMs, int intervalMs)
{
    if (intervalMs < 0) intervalMs = 0;
    QElapsedTimer clock;
    clock.start();
    while (true) {
        GetTransactionResponse r = fetchTransaction(hash);
        if (r.getStatus() != GetTransactionResponse::Status::NOT_FOUND) {
            return r;
        }
        if (clock.elapsed() >= timeoutMs) {
            throw std::runtime_error(
                QString("pollTransaction: timed out after %1 ms waiting for %2")
                    .arg(timeoutMs).arg(hash).toStdString());
        }
        // Yield without sleeping the GUI thread by spinning a tiny event
        // loop terminated by a single-shot timer.
        QEventLoop wait;
        QTimer::singleShot(intervalMs, &wait, &QEventLoop::quit);
        wait.exec();
    }
}
