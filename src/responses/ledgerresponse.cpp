#include "ledgerresponse.h"

LedgerResponse::LedgerResponse(QNetworkReply *reply)
    :Response(reply)
    ,m_sequence(0)
    ,m_transactionCount(0)
    ,m_successfulTransactionCount(0)
    ,m_failedTransactionCount(0)
    ,m_operationCount(0)
    ,m_baseFee(0)
    ,m_baseFeeInStroops(0)
    ,m_baseReserveInStroops(0)
    ,m_maxTxSetSize(0)
    ,m_protocolVersion(0)
{

}

qint64 LedgerResponse::getSequence() const{
    return m_sequence;
}

QString LedgerResponse::getHash() const{
    return m_hash;
}

QString LedgerResponse::getPagingToken() const{
    return m_pagingToken;
}

QString LedgerResponse::getPrevHash() const{
    return m_prevHash;
}

qint32 LedgerResponse::getTransactionCount() const{
    return m_transactionCount;
}

qint32 LedgerResponse::getSuccessfulTransactionCount() const
{
    return m_successfulTransactionCount;
}

qint32 LedgerResponse::getFailedTransactionCount() const
{
    return m_failedTransactionCount;
}

qint32 LedgerResponse::getOperationCount() const{
    return m_operationCount;
}

QString LedgerResponse::getClosedAt() const{
    return m_closedAt;
}

QString LedgerResponse::getTotalCoins() const{
    return m_totalCoins;
}

QString LedgerResponse::getFeePool() const{
    return m_feePool;
}

qint64 LedgerResponse::getBaseFee() const{
    return m_baseFee;
}

QString LedgerResponse::getBaseReserve() const{
    return m_baseReserve;
}

QString LedgerResponse::getBaseFeeInStroops() const
{
    return QString::number(m_baseFeeInStroops);
}

QString LedgerResponse::getBaseReserveInStroops() const
{
    return QString::number(m_baseReserveInStroops);
}

qint64 LedgerResponse::baseFeeInStroops() const
{
    return m_baseFeeInStroops;
}

qint64 LedgerResponse::baseReserveInStroops() const
{
    return m_baseReserveInStroops;
}

qint32 LedgerResponse::getMaxTxSetSize() const{
    return m_maxTxSetSize;
}

qint32 LedgerResponse::getProtocolVersion() const
{
    return m_protocolVersion;
}

QString LedgerResponse::getHeaderXdr() const
{
    return m_headerXdr;
}

LedgerResponseAttach::Links& LedgerResponse::getLinks(){
    return m_links;
}

bool LedgerResponseAttach::Links::operator !=(LedgerResponseAttach::Links &links)
{
    return (m_effects!=links.m_effects)
            || (m_operations!=links.m_operations)
            || (m_self!=links.m_self)
            || (m_transactions!=links.m_transactions);
}

bool LedgerResponseAttach::Links::operator ==(LedgerResponseAttach::Links &links)
{
    return (m_effects==links.m_effects)
            && (m_operations==links.m_operations)
            && (m_self==links.m_self)
            && (m_transactions==links.m_transactions);
}
