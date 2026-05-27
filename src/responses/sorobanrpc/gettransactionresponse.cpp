#include "gettransactionresponse.h"

namespace {
quint32 readU32(const QJsonObject& o, const QString& k) {
    QJsonValue v = o.value(k);
    return v.isString() ? v.toString().toUInt()
                        : static_cast<quint32>(v.toInteger(0));
}
}

GetTransactionResponse GetTransactionResponse::fromJson(const QJsonObject& r)
{
    GetTransactionResponse out;
    const QString status = r.value("status").toString();
    if (status == "SUCCESS")        out.m_status = Status::SUCCESS;
    else if (status == "FAILED")    out.m_status = Status::FAILED;
    else                            out.m_status = Status::NOT_FOUND;

    out.m_txHash                 = r.value("txHash").toString();
    out.m_latestLedger           = readU32(r, "latestLedger");
    out.m_latestLedgerCloseTime  = r.value("latestLedgerCloseTime").toString();
    out.m_oldestLedger           = readU32(r, "oldestLedger");
    out.m_oldestLedgerCloseTime  = r.value("oldestLedgerCloseTime").toString();
    out.m_applicationOrder       = static_cast<qint32>(r.value("applicationOrder").toInteger(0));
    out.m_envelopeXdr            = r.value("envelopeXdr").toString();
    out.m_resultXdr              = r.value("resultXdr").toString();
    out.m_resultMetaXdr          = r.value("resultMetaXdr").toString();
    out.m_ledger                 = readU32(r, "ledger");
    out.m_createdAt              = r.value("createdAt").toString();
    return out;
}
