#include "ledgerresponse.h"

LedgerResponse::LedgerResponse(QNetworkReply *reply)
    :Response(reply)
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

qint32 LedgerResponse::getMaxTxSetSize() const{
    return m_maxTxSetSize;
}

LedgerResponseAttach::Links& LedgerResponse::getLinks(){
    return m_links;
}

bool LedgerResponseAttach::Links::operator !=(LedgerResponseAttach::Links &links)
{
    Q_UNUSED(links)
    return true;
}
