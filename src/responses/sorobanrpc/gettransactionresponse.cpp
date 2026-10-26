#include "gettransactionresponse.h"
#include "gethealthresponse.h"

#include <QJsonArray>
#include <limits>
#include <stdexcept>

QSTELLAR_BEGIN_NS


GetTransactionResponse GetTransactionResponse::fromJson(const QJsonObject& r)
{
    return read(sorobanrpcdetail::RpcFields(r, QStringLiteral("getTransaction")), true);
}

GetTransactionResponse GetTransactionResponse::read(const sorobanrpcdetail::RpcFields& r,
                                                    bool single)
{
    const qint64 closeTimeMax = std::numeric_limits<qint64>::max();
    GetTransactionResponse out;
    // The status the RPC answers with, or the answer is refused. A missing or
    // unknown one used to read as NOT_FOUND, so pollTransaction polled on
    // until its timeout for an answer it could not read. getTransactions
    // lists transactions that are in a ledger: never NOT_FOUND.
    const QString status = single
        ? r.oneOf(QStringLiteral("status"), { QStringLiteral("SUCCESS"), QStringLiteral("NOT_FOUND"),
                                              QStringLiteral("FAILED") })
        : r.oneOf(QStringLiteral("status"), { QStringLiteral("SUCCESS"), QStringLiteral("FAILED") });
    if (status == QLatin1String("SUCCESS"))     out.m_status = Status::SUCCESS;
    else if (status == QLatin1String("FAILED")) out.m_status = Status::FAILED;
    else                                        out.m_status = Status::NOT_FOUND;

    // Every numeric field is always sent (as 0 for a transaction not found),
    // and was read as 0 when it was missing or was not a number.
    out.m_txHash = r.hash(QStringLiteral("txHash"));
    if (single) {
        // getTransactions sends the ledger range once for the page instead.
        out.m_latestLedger          = r.uint32(QStringLiteral("latestLedger"));
        out.m_latestLedgerCloseTime = r.decimal(QStringLiteral("latestLedgerCloseTime"), closeTimeMax);
        out.m_oldestLedger          = r.uint32(QStringLiteral("oldestLedger"));
        out.m_oldestLedgerCloseTime = r.decimal(QStringLiteral("oldestLedgerCloseTime"), closeTimeMax);
    }
    out.m_applicationOrder = r.int32(QStringLiteral("applicationOrder"));
    out.m_feeBump          = r.boolean(QStringLiteral("feeBump"));
    out.m_ledger           = r.uint32(QStringLiteral("ledger"));
    // A decimal string here, a JSON integer in getTransactions (the RPC's
    // own inconsistency): that one used to be read as a string, so every
    // transaction getTransactions listed had an empty createdAt.
    out.m_createdAt = single ? r.decimal(QStringLiteral("createdAt"), closeTimeMax)
                             : r.int64Text(QStringLiteral("createdAt"));

    // The transaction itself, whenever it was found.
    if (out.m_status == Status::NOT_FOUND) {
        out.m_envelopeXdr   = r.optionalXdr(QStringLiteral("envelopeXdr"));
        out.m_resultXdr     = r.optionalXdr(QStringLiteral("resultXdr"));
        out.m_resultMetaXdr = r.optionalXdr(QStringLiteral("resultMetaXdr"));
    } else {
        out.m_envelopeXdr   = r.xdr(QStringLiteral("envelopeXdr"));
        out.m_resultXdr     = r.xdr(QStringLiteral("resultXdr"));
        out.m_resultMetaXdr = r.xdr(QStringLiteral("resultMetaXdr"));
    }

    // Event lists are left out when empty.
    out.m_diagnosticEventsXdr = r.xdrList(QStringLiteral("diagnosticEventsXdr"));

    // Protocol 23+ nests the event streams under an `events` object.
    const QString eventsKey = QStringLiteral("events");
    if (r.has(eventsKey)) {
        const sorobanrpcdetail::RpcFields events = r.object(eventsKey);
        out.m_events.diagnosticEventsXdr  = events.xdrList(QStringLiteral("diagnosticEventsXdr"));
        out.m_events.transactionEventsXdr = events.xdrList(QStringLiteral("transactionEventsXdr"));
        const QString contractKey = QStringLiteral("contractEventsXdr");
        if (events.has(contractKey)) {
            // One list per operation.
            const QJsonArray groups = events.array(contractKey);
            for (int i = 0; i < groups.size(); ++i) {
                out.m_events.contractEventsXdr.append(events.xdrListValue(
                    QStringLiteral("%1[%2]").arg(events.name(contractKey)).arg(i), groups.at(i)));
            }
        }
    }

    // Expose diagnostics uniformly regardless of which shape the server used:
    // if the flat field was empty, fall back to the nested one.
    if (out.m_diagnosticEventsXdr.isEmpty())
        out.m_diagnosticEventsXdr = out.m_events.diagnosticEventsXdr;

    return out;
}
stellar::TransactionMeta GetTransactionResponse::getTransactionMeta() const
{
    if (m_resultMetaXdr.isEmpty())
        throw std::runtime_error("getTransaction: no TransactionMeta (the transaction was not found)");
    const auto decoded = QByteArray::fromBase64Encoding(
        m_resultMetaXdr.toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
    if (!decoded)
        throw std::runtime_error("getTransaction: resultMetaXdr is not base64");
    return stellar::decodeTransactionMeta(*decoded);
}

bool GetTransactionResponse::hasReturnValue() const
{
    return stellar::returnValueOf(getTransactionMeta(), nullptr);
}

stellar::SCVal GetTransactionResponse::getReturnValue() const
{
    stellar::SCVal value;
    if (!stellar::returnValueOf(getTransactionMeta(), &value))
        throw std::runtime_error("getTransaction: the transaction's meta carries no return value "
                                 "(not a Soroban call, extend/restore footprint, or a failed call)");
    return value;
}

QSTELLAR_END_NS
