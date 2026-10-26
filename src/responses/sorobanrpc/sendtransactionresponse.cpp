#include "sendtransactionresponse.h"
#include "gethealthresponse.h"

#include <QJsonArray>
#include <limits>

QSTELLAR_BEGIN_NS


SendTransactionResponse SendTransactionResponse::fromJson(const QJsonObject& r)
{
    const sorobanrpcdetail::RpcFields fields(r, QStringLiteral("sendTransaction"));
    SendTransactionResponse out;
    // A missing or unknown status is a malformed answer, not an
    // acknowledgement: it used to read as PENDING, so a caller polled for
    // a transaction the RPC never took in, and kept its sequence number.
    const QString s = fields.oneOf(QStringLiteral("status"),
                                   { QStringLiteral("PENDING"), QStringLiteral("DUPLICATE"),
                                     QStringLiteral("TRY_AGAIN_LATER"), QStringLiteral("ERROR") });
    if (s == QLatin1String("PENDING"))              out.m_status = Status::PENDING;
    else if (s == QLatin1String("DUPLICATE"))       out.m_status = Status::DUPLICATE;
    else if (s == QLatin1String("TRY_AGAIN_LATER")) out.m_status = Status::TRY_AGAIN_LATER;
    else                                            out.m_status = Status::ERROR_;

    // The hash is what the transaction is polled by: 64 hex digits, always.
    // A missing or non-numeric latestLedger used to read as 0.
    out.m_hash                  = fields.hash(QStringLiteral("hash"));
    out.m_latestLedger          = fields.uint32(QStringLiteral("latestLedger"));
    out.m_latestLedgerCloseTime = fields.decimal(QStringLiteral("latestLedgerCloseTime"),
                                                 std::numeric_limits<qint64>::max());
    // Why the network refused the transaction comes with ERROR, and only with
    // it: a result on any other status contradicts the status.
    const QString errorKey = QStringLiteral("errorResultXdr");
    if (out.m_status == Status::ERROR_)
        out.m_errorResultXdr = fields.xdr(errorKey);
    else if (fields.has(errorKey))
        fields.refuse(errorKey, QStringLiteral("comes with a status other than ERROR"));
    out.m_diagnosticEventsXdr   = fields.xdrList(QStringLiteral("diagnosticEventsXdr"));
    return out;
}
QSTELLAR_END_NS
