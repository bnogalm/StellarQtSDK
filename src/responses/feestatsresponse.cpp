#include "feestatsresponse.h"

FeeStatsResponse::FeeStatsResponse(QNetworkReply* reply)
    :Response(reply)
    ,m_ledgerCapacityUsage(0)
    ,m_lastLedgerBaseFee(0)
    ,m_lastLedger(0)
{

}


FeeStatsResponse::~FeeStatsResponse(){}



float FeeStatsResponse::getLedgerCapacityUsage() const {
    return m_ledgerCapacityUsage;
}

qint64 FeeStatsResponse::getLastLedgerBaseFee() const
{
    return m_lastLedgerBaseFee;
}

qint64 FeeStatsResponse::getLastLedger() const
{
    return m_lastLedger;
}

FeeDistribution FeeStatsResponse::getFeeCharged() const
{
    return m_feeCharged;
}

FeeDistribution FeeStatsResponse::getMaxFee() const{
    return m_maxFee;
}

FeeDistribution::FeeDistribution()
    :m_min(0)
    ,m_max(0)
    ,m_mode(0)
    ,m_p10(0)
    ,m_p20(0)
    ,m_p30(0)
    ,m_p40(0)
    ,m_p50(0)
    ,m_p60(0)
    ,m_p70(0)
    ,m_p80(0)
    ,m_p90(0)
    ,m_p95(0)
    ,m_p99(0)
{

}


qint64 FeeDistribution::getMin() const
{
    return m_min;
}

qint64 FeeDistribution::getMax() const
{
    return m_max;
}

qint64 FeeDistribution::getMode() const
{
    return m_mode;
}

qint64 FeeDistribution::getP10() const{
    return m_p10;
}

qint64 FeeDistribution::getP20() const{
    return m_p20;
}

qint64 FeeDistribution::getP30() const{
    return m_p30;
}

qint64 FeeDistribution::getP40() const{
    return m_p40;
}

qint64 FeeDistribution::getP50() const{
    return m_p50;
}

qint64 FeeDistribution::getP60() const{
    return m_p60;
}

qint64 FeeDistribution::getP70() const{
    return m_p70;
}

qint64 FeeDistribution::getP80() const{
    return m_p80;
}

qint64 FeeDistribution::getP90() const{
    return m_p90;
}

qint64 FeeDistribution::getP95() const{
    return m_p95;
}

qint64 FeeDistribution::getP99() const{
    return m_p99;
}

bool FeeDistribution::operator !=(FeeDistribution &feeDistribution)
{
    return (m_min != feeDistribution.m_min) ||
           (m_max != feeDistribution.m_max) ||
           (m_mode != feeDistribution.m_mode) ||
           (m_p10 != feeDistribution.m_p10) ||
                (m_p20 != feeDistribution.m_p20) ||
                (m_p30 != feeDistribution.m_p30) ||
                (m_p40 != feeDistribution.m_p40) ||
                (m_p50 != feeDistribution.m_p50) ||
                (m_p60 != feeDistribution.m_p60) ||
                (m_p70 != feeDistribution.m_p70) ||
                (m_p80 != feeDistribution.m_p80) ||
                (m_p90 != feeDistribution.m_p90) ||
                (m_p95 != feeDistribution.m_p95) ||
                (m_p99 != feeDistribution.m_p99);
}

bool FeeDistribution::operator ==(FeeDistribution &feeDistribution)
{
    return (m_min == feeDistribution.m_min) &&
           (m_max == feeDistribution.m_max) &&
           (m_mode == feeDistribution.m_mode) &&
           (m_p10 == feeDistribution.m_p10) &&
                (m_p20 == feeDistribution.m_p20) &&
                (m_p30 == feeDistribution.m_p30) &&
                (m_p40 == feeDistribution.m_p40) &&
                (m_p50 == feeDistribution.m_p50) &&
                (m_p60 == feeDistribution.m_p60) &&
                (m_p70 == feeDistribution.m_p70) &&
                (m_p80 == feeDistribution.m_p80) &&
                (m_p90 == feeDistribution.m_p90) &&
                (m_p95 == feeDistribution.m_p95) &&
                (m_p99 == feeDistribution.m_p99);
}
