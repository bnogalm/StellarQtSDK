#include "operationresponse.h"
#include "../../keypair.h"

OperationResponse::OperationResponse(QNetworkReply* reply)
    : Response(reply)
  ,m_id(0)
  ,m_sourceAccountKeypair(nullptr)
  ,m_transactionSuccessful(QVariant::Bool)// we have to indicate the type or it will not be filled, it will stay returning isNull as true if it is not initialized
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

Boolean OperationResponse::isTransactionSuccessful() const{
    return Boolean(m_transactionSuccessful);
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
        m_sourceAccountKeypair=nullptr;
    }
    m_sourceAccount = sourceAccount;

}

void OperationResponse::setTransactionSuccessful(QVariant transactionSuccessful)
{
    if(transactionSuccessful.type()==QVariant::Bool)
    {
        m_transactionSuccessful = transactionSuccessful;
    }
}

