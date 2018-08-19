#include "setoptionsoperationresponse.h"
#include "../../keypair.h"

SetOptionsOperationResponse::SetOptionsOperationResponse(QNetworkReply *reply)
    :OperationResponse(reply),m_inflationDestKeypair(0),m_signerKeypair(0)
{

}

SetOptionsOperationResponse::~SetOptionsOperationResponse()
{
    if(m_inflationDestKeypair)
        delete m_inflationDestKeypair;
}

quint32 SetOptionsOperationResponse::getLowThreshold() const{
    return m_lowThreshold;
}

quint32 SetOptionsOperationResponse::getMedThreshold() const{
    return m_medThreshold;
}

quint32 SetOptionsOperationResponse::getHighThreshold() const{
    return m_highThreshold;
}

KeyPair *SetOptionsOperationResponse::getInflationDestination() {
    if(!m_inflationDestKeypair)
        m_inflationDestKeypair = KeyPair::fromAccountId(m_inflationDest);
    return m_inflationDestKeypair;
}

QString SetOptionsOperationResponse::getHomeDomain() const{
    return m_homeDomain;
}

KeyPair *SetOptionsOperationResponse::getSigner() {
    if(!m_signerKeypair)
        m_signerKeypair = KeyPair::fromAccountId(m_signerKey);
    return m_signerKeypair;
}

QString SetOptionsOperationResponse::getSignerKey() const
{
    return m_signerKey;
}

quint32 SetOptionsOperationResponse::getSignerWeight() const{
    return m_signerWeight;
}

quint32 SetOptionsOperationResponse::getMasterKeyWeight() const{
    return m_masterKeyWeight;
}

QStringList SetOptionsOperationResponse::getClearFlags() const{
    return m_clearFlagsS;
}

QStringList SetOptionsOperationResponse::getSetFlags() const{
    return m_setFlagsS;
}

QString SetOptionsOperationResponse::inflationDest() const
{
    return m_inflationDest;
}

QString SetOptionsOperationResponse::signerKey() const
{
    return m_signerKey;
}

void SetOptionsOperationResponse::setInflationDest(QString inflationDest)
{
    if(m_inflationDestKeypair)
        delete m_inflationDestKeypair;
    m_inflationDest = inflationDest;
}

void SetOptionsOperationResponse::setSignerKey(QString signerKey)
{
    if(m_signerKeypair)
        delete m_signerKeypair;
    m_signerKey = signerKey;
}
