#include "operationresponse.h"
#include "../../keypair.h"

OperationResponse::OperationResponse(QNetworkReply* reply)
    : Response(reply)
  ,m_id(0)
  ,m_sourceAccountKeypair(0)
{

}

OperationResponse::~OperationResponse()
{
    if(m_sourceAccountKeypair)
        delete m_sourceAccountKeypair;
}

qint64 OperationResponse::getId() const
{
    return m_id;
}

KeyPair &OperationResponse::getSourceAccount() {
    if(!m_sourceAccountKeypair){
        m_sourceAccountKeypair = KeyPair::fromAccountId(m_sourceAccount);
    }
    return *m_sourceAccountKeypair;
}

QString OperationResponse::getPagingToken() const
{
    return m_pagingToken;
}

QString OperationResponse::getCreatedAt() const
{
    return m_createdAt;
}

QString OperationResponse::getTransactionHash() const
{
    return m_transactionHash;
}

QString OperationResponse::getType() const
{
    return m_type;
}

OperationResponseAttach::Links &OperationResponse::getLinks()
{
    return m_links;
}

QString OperationResponse::sourceAccount() const
{
    return m_sourceAccount;
}

void OperationResponse::setSourceAccount(QString sourceAccount)
{
    if(m_sourceAccountKeypair){
        delete m_sourceAccountKeypair;
        m_sourceAccountKeypair=0;
    }
    m_sourceAccount = sourceAccount;

}
