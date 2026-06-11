#include "contractcreditedeffectresponse.h"
#include "../../keypair.h"
#include "../../assettypenative.h"
#include "../../assettypecreditalphanum.h"

QSTELLAR_BEGIN_NS


ContractCreditedEffectResponse::ContractCreditedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply),m_asset(nullptr)
{

}

ContractCreditedEffectResponse::~ContractCreditedEffectResponse()
{
    if(m_asset)
        delete m_asset;
}

QString ContractCreditedEffectResponse::getAmount() const {
    return m_amount;
}

Asset *ContractCreditedEffectResponse::getAsset() {
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

QString ContractCreditedEffectResponse::assetType() const
{
    return m_assetType;
}

QString ContractCreditedEffectResponse::assetCode() const
{
    return m_assetCode;
}

QString ContractCreditedEffectResponse::assetIssuer() const
{
    return m_assetIssuer;
}

void ContractCreditedEffectResponse::setAssetType(QString assetType)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetType = assetType;
}

void ContractCreditedEffectResponse::setAssetCode(QString assetCode)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetCode = assetCode;
}

void ContractCreditedEffectResponse::setAssetIssuer(QString assetIssuer)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetIssuer = assetIssuer;
}
QSTELLAR_END_NS
