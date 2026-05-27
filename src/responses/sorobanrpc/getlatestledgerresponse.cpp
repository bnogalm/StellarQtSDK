#include "getlatestledgerresponse.h"

GetLatestLedgerResponse GetLatestLedgerResponse::fromJson(const QJsonObject& r)
{
    GetLatestLedgerResponse out;
    out.m_id = r.value("id").toString();
    QJsonValue pv = r.value("protocolVersion");
    out.m_protocolVersion = pv.isString() ? pv.toString().toInt()
                                          : static_cast<qint32>(pv.toInteger(0));
    QJsonValue sq = r.value("sequence");
    out.m_sequence = sq.isString() ? sq.toString().toUInt()
                                   : static_cast<quint32>(sq.toInteger(0));
    return out;
}
