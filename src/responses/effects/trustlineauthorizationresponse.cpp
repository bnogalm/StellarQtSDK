#include "trustlineauthorizationresponse.h"
#include "../../keypair.h"

TrustlineAuthorizationResponse::TrustlineAuthorizationResponse(QNetworkReply *reply )
    :EffectResponse(reply),m_trustorKeypair(nullptr)
{

}

TrustlineAuthorizationResponse::~TrustlineAuthorizationResponse()
{
    if(m_trustorKeypair)
        delete m_trustorKeypair;
}

KeyPair &TrustlineAuthorizationResponse::getTrustor() {
    if(!m_trustorKeypair)
    {
        m_trustorKeypair  = KeyPair::fromAccountId(m_trustor);
    }
    return *m_trustorKeypair;
}

QString TrustlineAuthorizationResponse::getAssetType() {
    return m_assetType;
}

QString TrustlineAuthorizationResponse::getAssetCode() {
    return m_assetCode;
}

QString TrustlineAuthorizationResponse::trustor() const
{
    return m_trustor;
}

void TrustlineAuthorizationResponse::setTrustor(QString trustor)
{
    if(m_trustorKeypair)
    {
        delete m_trustorKeypair;
        m_trustorKeypair =nullptr;
    }
    m_trustor = trustor;
}
