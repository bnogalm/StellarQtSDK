#include "getfeestatsresponse.h"

QSTELLAR_BEGIN_NS


namespace {
QString readStr(const QJsonObject& o, const QString& k) {
    return o.value(k).toString();
}
quint32 readU32(const QJsonObject& o, const QString& k) {
    QJsonValue v = o.value(k);
    return v.isString() ? v.toString().toUInt()
                        : static_cast<quint32>(v.toInteger(0));
}
}

SorobanFeeDistribution SorobanFeeDistribution::fromJson(const QJsonObject& o)
{
    SorobanFeeDistribution fd;
    fd.max  = readStr(o, "max");
    fd.min  = readStr(o, "min");
    fd.mode = readStr(o, "mode");
    fd.p10  = readStr(o, "p10");
    fd.p20  = readStr(o, "p20");
    fd.p30  = readStr(o, "p30");
    fd.p40  = readStr(o, "p40");
    fd.p50  = readStr(o, "p50");
    fd.p60  = readStr(o, "p60");
    fd.p70  = readStr(o, "p70");
    fd.p80  = readStr(o, "p80");
    fd.p90  = readStr(o, "p90");
    fd.p95  = readStr(o, "p95");
    fd.p99  = readStr(o, "p99");
    fd.transactionCount = readU32(o, "transactionCount");
    fd.ledgerCount      = readU32(o, "ledgerCount");
    return fd;
}

GetFeeStatsResponse GetFeeStatsResponse::fromJson(const QJsonObject& r)
{
    GetFeeStatsResponse out;
    out.m_sorobanInclusionFee = SorobanFeeDistribution::fromJson(r.value("sorobanInclusionFee").toObject());
    out.m_inclusionFee        = SorobanFeeDistribution::fromJson(r.value("inclusionFee").toObject());
    out.m_latestLedger        = readU32(r, "latestLedger");
    return out;
}
QSTELLAR_END_NS
