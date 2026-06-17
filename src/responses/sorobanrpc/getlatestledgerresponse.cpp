#include "getlatestledgerresponse.h"
#include "qtcompat.h"

QSTELLAR_BEGIN_NS


GetLatestLedgerResponse GetLatestLedgerResponse::fromJson(const QJsonObject& r)
{
    GetLatestLedgerResponse out;
    out.m_id = r.value("id").toString();
    QJsonValue pv = r.value("protocolVersion");
    out.m_protocolVersion = pv.isString() ? pv.toString().toInt()
                                          : static_cast<qint32>(jsonToInt64(pv));
    QJsonValue sq = r.value("sequence");
    out.m_sequence = sq.isString() ? sq.toString().toUInt()
                                   : static_cast<quint32>(jsonToInt64(sq));
    return out;
}
QSTELLAR_END_NS
