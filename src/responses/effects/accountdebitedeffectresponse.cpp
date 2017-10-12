#include "accountdebitedeffectresponse.h"
#include "../../keypair.h"
#include "../../assettypenative.h"
#include "../../assettypecreditalphanum.h"


AccountDebitedEffectResponse::AccountDebitedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply),m_asset(nullptr)
{

}

AccountDebitedEffectResponse::~AccountDebitedEffectResponse()
{
    if(m_asset)
        delete m_asset;
}

QString AccountDebitedEffectResponse::getAmount() const {
    return m_amount;
}

Asset *AccountDebitedEffectResponse::getAsset() {
    if(!m_asset)
    {
        if(m_assetType == "native")
        {
            m_asset = new AssetTypeNative();
        }
        else
        {
            KeyPair* issuer = KeyPair::fromAccountId(m_assetIssuer);
            m_asset= Asset::createNonNativeAsset(m_assetCode, issuer);
        }
    }
    return m_asset;
}

QString AccountDebitedEffectResponse::assetType() const
{
    return m_assetType;
}

QString AccountDebitedEffectResponse::assetCode() const
{
    return m_assetCode;
}

QString AccountDebitedEffectResponse::assetIssuer() const
{
    return m_assetIssuer;
}

void AccountDebitedEffectResponse::setAssetType(QString assetType)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetType = assetType;
}

void AccountDebitedEffectResponse::setAssetCode(QString assetCode)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetCode = assetCode;
}

void AccountDebitedEffectResponse::setAssetIssuer(QString assetIssuer)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetIssuer = assetIssuer;
}
