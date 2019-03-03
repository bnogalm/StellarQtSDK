#include "operationfeestatsresponse.h"

OperationFeeStatsResponse::OperationFeeStatsResponse(QNetworkReply* reply)
    :Response(reply)
    ,m_min(0)
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
    ,m_ledgerCapacityUsage(0)
    ,m_lastLedgerBaseFee(0)
    ,m_lastLedger(0)
{

}

OperationFeeStatsResponse::~OperationFeeStatsResponse(){}

qint64 OperationFeeStatsResponse::getMin() const
{
    return m_min;
}

qint64 OperationFeeStatsResponse::getMode() const
{
    return m_mode;
}

qint64 OperationFeeStatsResponse::getP10() const{
    return m_p10;
}

qint64 OperationFeeStatsResponse::getP20() const{
    return m_p20;
}

qint64 OperationFeeStatsResponse::getP30() const{
    return m_p30;
}

qint64 OperationFeeStatsResponse::getP40() const{
    return m_p40;
}

qint64 OperationFeeStatsResponse::getP50() const{
    return m_p50;
}

qint64 OperationFeeStatsResponse::getP60() const{
    return m_p60;
}

qint64 OperationFeeStatsResponse::getP70() const{
    return m_p70;
}

qint64 OperationFeeStatsResponse::getP80() const{
    return m_p80;
}

qint64 OperationFeeStatsResponse::getP90() const{
    return m_p90;
}

qint64 OperationFeeStatsResponse::getP95() const{
    return m_p95;
}

qint64 OperationFeeStatsResponse::getP99() const{
    return m_p99;
}

float OperationFeeStatsResponse::getLedgerCapacityUsage() const {
    return m_ledgerCapacityUsage;
}

qint64 OperationFeeStatsResponse::getLastLedgerBaseFee() const
{
    return m_lastLedgerBaseFee;
}

qint64 OperationFeeStatsResponse::getLastLedger() const
{
    return m_lastLedger;
}
