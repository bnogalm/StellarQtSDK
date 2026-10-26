#include "getfeestatsresponse.h"
#include "gethealthresponse.h"

#include <limits>

QSTELLAR_BEGIN_NS


namespace {
/**
 * The RPC sends each fee (a uint64) and transactionCount (a uint32) as a
 * decimal string, and ledgerCount as a JSON integer. They used to be taken
 * as they came: a fee that was not a number was kept as its text, a
 * transactionCount that was not one read as 0, and so did a missing one.
 */
SorobanFeeDistribution readDistribution(const sorobanrpcdetail::RpcFields& o)
{
    const quint64 fee = std::numeric_limits<quint64>::max();
    SorobanFeeDistribution fd;
    fd.max  = o.decimal(QStringLiteral("max"), fee);
    fd.min  = o.decimal(QStringLiteral("min"), fee);
    fd.mode = o.decimal(QStringLiteral("mode"), fee);
    fd.p10  = o.decimal(QStringLiteral("p10"), fee);
    fd.p20  = o.decimal(QStringLiteral("p20"), fee);
    fd.p30  = o.decimal(QStringLiteral("p30"), fee);
    fd.p40  = o.decimal(QStringLiteral("p40"), fee);
    fd.p50  = o.decimal(QStringLiteral("p50"), fee);
    fd.p60  = o.decimal(QStringLiteral("p60"), fee);
    fd.p70  = o.decimal(QStringLiteral("p70"), fee);
    fd.p80  = o.decimal(QStringLiteral("p80"), fee);
    fd.p90  = o.decimal(QStringLiteral("p90"), fee);
    fd.p95  = o.decimal(QStringLiteral("p95"), fee);
    fd.p99  = o.decimal(QStringLiteral("p99"), fee);
    fd.transactionCount = o.decimal(QStringLiteral("transactionCount"),
                                    std::numeric_limits<quint32>::max()).toUInt();
    fd.ledgerCount      = o.uint32(QStringLiteral("ledgerCount"));
    return fd;
}
}

SorobanFeeDistribution SorobanFeeDistribution::fromJson(const QJsonObject& o)
{
    return readDistribution(sorobanrpcdetail::RpcFields(o, QStringLiteral("getFeeStats")));
}

GetFeeStatsResponse GetFeeStatsResponse::fromJson(const QJsonObject& r)
{
    const sorobanrpcdetail::RpcFields fields(r, QStringLiteral("getFeeStats"));
    GetFeeStatsResponse out;
    out.m_sorobanInclusionFee = readDistribution(fields.object(QStringLiteral("sorobanInclusionFee")));
    out.m_inclusionFee        = readDistribution(fields.object(QStringLiteral("inclusionFee")));
    out.m_latestLedger        = fields.uint32(QStringLiteral("latestLedger"));
    return out;
}
QSTELLAR_END_NS
