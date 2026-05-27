#include "getledgersresponse.h"

#include <QJsonArray>

namespace {
quint32 readU32(const QJsonObject& o, const QString& k) {
    QJsonValue v = o.value(k);
    return v.isString() ? v.toString().toUInt()
                        : static_cast<quint32>(v.toInteger(0));
}
}

GetLedgersResponse GetLedgersResponse::fromJson(const QJsonObject& r)
{
    GetLedgersResponse out;
    for (const QJsonValue& v : r.value("ledgers").toArray()) {
        QJsonObject e = v.toObject();
        LedgerInfo li;
        li.hash             = e.value("hash").toString();
        li.sequence         = readU32(e, "sequence");
        li.ledgerCloseTime  = e.value("ledgerCloseTime").toString();
        li.headerXdr        = e.value("headerXdr").toString();
        li.metadataXdr      = e.value("metadataXdr").toString();
        out.m_ledgers.append(li);
    }
    out.m_latestLedger          = readU32(r, "latestLedger");
    out.m_latestLedgerCloseTime = r.value("latestLedgerCloseTime").toString();
    out.m_oldestLedger          = readU32(r, "oldestLedger");
    out.m_oldestLedgerCloseTime = r.value("oldestLedgerCloseTime").toString();
    out.m_cursor                = r.value("cursor").toString();
    return out;
}
