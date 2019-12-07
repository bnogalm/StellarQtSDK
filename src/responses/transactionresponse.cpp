#include "transactionresponse.h"
#include "../keypair.h"
#include "../memo.h"
TransactionResponse::TransactionResponse(QNetworkReply *reply)
    :Response(reply)
    ,m_sourceAccountKeypair(nullptr)
    ,m_successful(QVariant::Bool)// we have to indicate the type or it will not be filled, it will stay returning isNull as true if it is not initialized
    ,m_sourceAccountSequence(0)
    ,m_maxFee(0)
    ,m_feeCharged(0)
    ,m_operationCount(0)
    ,m_memo(nullptr)
{
}

TransactionResponse::~TransactionResponse()
{
    if(m_sourceAccountKeypair)
        delete m_sourceAccountKeypair;
    if(m_memo)
        delete m_memo;
}

QString TransactionResponse::getHash() const{
    return m_hash;
}

qint64 TransactionResponse::getLedger() const{
    return m_ledger;
}

QString TransactionResponse::getCreatedAt() const{
    return m_createdAt;
}

KeyPair *TransactionResponse::getSourceAccount() {
    if(!m_sourceAccountKeypair)
    {
        m_sourceAccountKeypair= KeyPair::fromAccountId(m_sourceAccount);
    }
    return m_sourceAccountKeypair;
}

QString TransactionResponse::getPagingToken() const{
    return m_pagingToken;
}

Boolean TransactionResponse::isSuccessful() const{
    return Boolean(m_successful);
}

qint64 TransactionResponse::getSourceAccountSequence() const{
    return m_sourceAccountSequence;
}

qint64 TransactionResponse::getMaxFee() const{
    return m_maxFee;
}

qint64 TransactionResponse::getFeeCharged() const{
    return m_feeCharged;
}

int TransactionResponse::getOperationCount() const{
    return m_operationCount;
}

QString TransactionResponse::getEnvelopeXdr() const{
    return m_envelopeXdr;
}

QString TransactionResponse::getResultXdr() const{
    return m_resultXdr;
}

QString TransactionResponse::getResultMetaXdr() const{
    return m_resultMetaXdr;
}

Memo *TransactionResponse::getMemo() {
    if(!m_memo)
    {
        m_memo = Memo::parse(m_memoType,m_memoData);
    }
    return m_memo;
}

TransactionResponseAttach::Links &TransactionResponse::getLinks() {
    return m_links;
}

QString TransactionResponse::sourceAccount() const
{
    return m_sourceAccount;
}

QString TransactionResponse::memoType() const
{
    return m_memoType;
}

QByteArray TransactionResponse::memo() const
{
    return m_memoData;
}

void TransactionResponse::setSourceAccount(QString sourceAccount)
{
    if(m_sourceAccountKeypair)
    {
        delete m_sourceAccountKeypair;
        m_sourceAccountKeypair = nullptr;
    }
    m_sourceAccount = sourceAccount;
}

void TransactionResponse::setMemoType(QString memoType)
{
    if(m_memo)
    {
        delete m_memo;
        m_memo=nullptr;
    }
    m_memoType = memoType;
}

void TransactionResponse::setMemo(QByteArray memoData)
{
    if(m_memo)
    {
        delete m_memo;
        m_memo=nullptr;
    }
    m_memoData = memoData;
}

void TransactionResponse::setSuccessful(QVariant successful)
{
    if(successful.type()==QVariant::Bool)
    {
        m_successful = successful;
    }
}
