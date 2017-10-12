#include "trustlinecudresponse.h"
#include "../../keypair.h"
#include "../../assettypenative.h"
#include "../../assettypecreditalphanum.h"

TrustlineCUDResponse::TrustlineCUDResponse(QNetworkReply *reply)
    :EffectResponse(reply), m_asset(nullptr)
{

}

TrustlineCUDResponse::~TrustlineCUDResponse()
{
    if(m_asset)
        delete m_asset;
}

QString TrustlineCUDResponse::getLimit() const {
    return m_limit;
}

Asset *TrustlineCUDResponse::getAsset() {
    if(!m_asset){
        if (m_assetType=="native") {
            m_asset= new AssetTypeNative();
        } else {
            KeyPair* issuer = KeyPair::fromAccountId(m_assetIssuer);
            m_asset= Asset::createNonNativeAsset(m_assetCode, issuer);
        }
    }
    return m_asset;
}

QString TrustlineCUDResponse::assetType() const
{
    return m_assetType;
}

QString TrustlineCUDResponse::assetCode() const
{
    return m_assetCode;
}

QString TrustlineCUDResponse::assetIssuer() const
{
    return m_assetIssuer;
}

void TrustlineCUDResponse::setAssetType(QString assetType)
{
    if(m_asset){
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetType = assetType;
}

void TrustlineCUDResponse::setAssetCode(QString assetCode)
{
    if(m_asset){
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetCode = assetCode;
}

void TrustlineCUDResponse::setAssetIssuer(QString assetIssuer)
{
    if(m_asset){
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetIssuer = assetIssuer;
}
