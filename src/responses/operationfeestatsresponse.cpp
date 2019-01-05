#include "operationfeestatsresponse.h"

OperationFeeStatsResponse::OperationFeeStatsResponse(QNetworkReply* reply)
    :Response(reply)
    ,m_min(0)
    ,m_mode(0)
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

qint64 OperationFeeStatsResponse::getLastLedgerBaseFee() const
{
    return m_lastLedgerBaseFee;
}

qint64 OperationFeeStatsResponse::getLastLedger() const
{
    return m_lastLedger;
}
