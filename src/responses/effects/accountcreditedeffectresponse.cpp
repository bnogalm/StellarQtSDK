#include "accountcreditedeffectresponse.h"
#include "../../keypair.h"
#include "../../assettypenative.h"
#include "../../assettypecreditalphanum.h"

AccountCreditedEffectResponse::AccountCreditedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply),m_asset(nullptr)
{

}

AccountCreditedEffectResponse::~AccountCreditedEffectResponse()
{
    if(m_asset)
        delete m_asset;
}

QString AccountCreditedEffectResponse::getAmount() const {
    return m_amount;
}

Asset *AccountCreditedEffectResponse::getAsset() {
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

QString AccountCreditedEffectResponse::assetType() const
{
    return m_assetType;
}

QString AccountCreditedEffectResponse::assetCode() const
{
    return m_assetCode;
}

QString AccountCreditedEffectResponse::assetIssuer() const
{
    return m_assetIssuer;
}

void AccountCreditedEffectResponse::setAssetType(QString assetType)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetType = assetType;
}

void AccountCreditedEffectResponse::setAssetCode(QString assetCode)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetCode = assetCode;
}

void AccountCreditedEffectResponse::setAssetIssuer(QString assetIssuer)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetIssuer = assetIssuer;
}
