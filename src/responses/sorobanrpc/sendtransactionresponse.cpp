#include "sendtransactionresponse.h"

#include <QJsonArray>

namespace {
quint32 readU32(const QJsonObject& o, const QString& k) {
    QJsonValue v = o.value(k);
    return v.isString() ? v.toString().toUInt()
                        : static_cast<quint32>(v.toInteger(0));
}
}

SendTransactionResponse SendTransactionResponse::fromJson(const QJsonObject& r)
{
    SendTransactionResponse out;
    const QString s = r.value("status").toString();
    if (s == "PENDING")               out.m_status = Status::PENDING;
    else if (s == "DUPLICATE")        out.m_status = Status::DUPLICATE;
    else if (s == "TRY_AGAIN_LATER")  out.m_status = Status::TRY_AGAIN_LATER;
    else if (s == "ERROR")            out.m_status = Status::ERROR_;

    out.m_hash                  = r.value("hash").toString();
    out.m_latestLedger          = readU32(r, "latestLedger");
    out.m_latestLedgerCloseTime = r.value("latestLedgerCloseTime").toString();
    out.m_errorResultXdr        = r.value("errorResultXdr").toString();
    for (const QJsonValue& v : r.value("diagnosticEventsXdr").toArray()) {
        out.m_diagnosticEventsXdr.append(v.toString());
    }
    return out;
}
