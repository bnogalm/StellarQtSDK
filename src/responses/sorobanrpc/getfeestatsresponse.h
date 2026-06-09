#ifndef GETFEESTATSRESPONSE_H
#define GETFEESTATSRESPONSE_H

#include <QJsonObject>
#include <QString>
#include <QtGlobal>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Soroban RPC `getFeeStats` result.
 *
 * Fee distribution stats are returned for two buckets:
 *   - sorobanInclusionFee: stats for Soroban transactions only
 *   - inclusionFee: stats for non-Soroban transactions
 * Both share the same shape (SorobanFeeDistribution).
 */
class SorobanFeeDistribution
{
public:
    QString max;
    QString min;
    QString mode;
    QString p10, p20, p30, p40, p50, p60, p70, p80, p90, p95, p99;
    quint32 transactionCount = 0;
    quint32 ledgerCount = 0;

    static SorobanFeeDistribution fromJson(const QJsonObject& obj);
};

class GetFeeStatsResponse
{
    SorobanFeeDistribution m_sorobanInclusionFee;
    SorobanFeeDistribution m_inclusionFee;
    quint32 m_latestLedger = 0;
public:
    static GetFeeStatsResponse fromJson(const QJsonObject& result);
    const SorobanFeeDistribution& getSorobanInclusionFee() const { return m_sorobanInclusionFee; }
    const SorobanFeeDistribution& getInclusionFee() const { return m_inclusionFee; }
    quint32 getLatestLedger() const { return m_latestLedger; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(SorobanFeeDistribution)
QSTELLAR_ALIAS(GetFeeStatsResponse)




#endif // GETFEESTATSRESPONSE_H
