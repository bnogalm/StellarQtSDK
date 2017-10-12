#include "changetrustoperationresponse.h"
#include "../../keypair.h"
#include "../../asset.h"
#include "../../assettypenative.h"
ChangeTrustOperationResponse::ChangeTrustOperationResponse(QNetworkReply *reply)
    :OperationResponse(reply),m_trustorKeypair(0),m_trusteeKeypair(0),m_asset(0)
{

}

ChangeTrustOperationResponse::~ChangeTrustOperationResponse()
{
    if(m_trusteeKeypair)
        delete m_trusteeKeypair;
    if(m_trustorKeypair)
        delete m_trustorKeypair;
    if(m_asset)
        delete m_asset;
}
KeyPair& ChangeTrustOperationResponse::getTrustor() {
    if(!m_trustorKeypair)
        m_trustorKeypair = KeyPair::fromAccountId(m_trustor);
    return *m_trustorKeypair;
}

KeyPair& ChangeTrustOperationResponse::getTrustee() {
    if(!m_trusteeKeypair)
        m_trusteeKeypair = KeyPair::fromAccountId(m_trustee);
    return *m_trusteeKeypair;
}

Asset* ChangeTrustOperationResponse::getAsset() {
    if(m_asset)
        return m_asset;
    if (m_assetType =="native") {
        m_asset= new AssetTypeNative();
    } else {
        KeyPair* issuer = KeyPair::fromAccountId(m_assetIssuer);
        m_asset =Asset::createNonNativeAsset(m_assetCode, issuer);
    }
    return m_asset;
}

QString ChangeTrustOperationResponse::trustor() const
{
    return m_trustor;
}

QString ChangeTrustOperationResponse::trustee() const
{
    return m_trustee;
}

QString ChangeTrustOperationResponse::assetType() const
{
    return m_assetType;
}

QString ChangeTrustOperationResponse::assetCode() const
{
    return m_assetCode;
}

QString ChangeTrustOperationResponse::assetIssuer() const
{
    return m_assetIssuer;
}

QString ChangeTrustOperationResponse::getLimit() const{
    return m_limit;
}

void ChangeTrustOperationResponse::setTrustor(QString trustor)
{
    if(m_trustorKeypair){
        delete m_trustorKeypair;
        m_trustorKeypair=0;
    }
    m_trustor = trustor;
}

void ChangeTrustOperationResponse::setTrustee(QString trustee)
{
    if(m_trusteeKeypair){
        delete m_trusteeKeypair;
        m_trusteeKeypair=0;
    }
    m_trustee = trustee;
}

void ChangeTrustOperationResponse::setAssetType(QString assetType)
{
    if(m_asset){
        delete m_asset;
        m_asset=0;
    }
    m_assetType = assetType;
}

void ChangeTrustOperationResponse::setAssetCode(QString assetCode)
{
    if(m_asset){
        delete m_asset;
        m_asset=0;
    }
    m_assetCode = assetCode;
}

void ChangeTrustOperationResponse::setAssetIssuer(QString assetIssuer)
{
    if(m_asset){
        delete m_asset;
        m_asset=0;
    }
    m_assetIssuer = assetIssuer;
}
