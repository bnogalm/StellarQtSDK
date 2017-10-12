#include "effectresponse.h"
#include "../../keypair.h"

EffectResponse::EffectResponse(QNetworkReply *reply)
    :Response(reply),m_accountKeypair(nullptr)
{

}

EffectResponse::~EffectResponse()
{
    if(!m_accountKeypair){
        delete m_accountKeypair;
    }
}

QString EffectResponse::getId() {
    return m_id;
}

KeyPair &EffectResponse::getAccount() {
    if(!m_accountKeypair){
        m_accountKeypair = KeyPair::fromAccountId(m_account);
    }
    return *m_accountKeypair;
}

QString EffectResponse::getType() {
    return m_type;
}

QString EffectResponse::getPagingToken() {
    return m_pagingToken;
}

QString EffectResponse::account() const
{
    return m_account;
}

void EffectResponse::setAccount(QString account)
{
    if(m_accountKeypair){
        delete m_accountKeypair;
        m_accountKeypair=nullptr;
    }
    m_account = account;
}

bool EffectResponseAttach::Links::operator !=(EffectResponseAttach::Links &links)
{
    Q_UNUSED(links)
    return true;
}
