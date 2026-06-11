#include "contractdebitedeffectresponse.h"
#include "../../keypair.h"
#include "../../assettypenative.h"
#include "../../assettypecreditalphanum.h"

QSTELLAR_BEGIN_NS


ContractDebitedEffectResponse::ContractDebitedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply),m_asset(nullptr)
{

}

ContractDebitedEffectResponse::~ContractDebitedEffectResponse()
{
    if(m_asset)
        delete m_asset;
}

QString ContractDebitedEffectResponse::getAmount() const {
    return m_amount;
}

Asset *ContractDebitedEffectResponse::getAsset() {
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

QString ContractDebitedEffectResponse::assetType() const
{
    return m_assetType;
}

QString ContractDebitedEffectResponse::assetCode() const
{
    return m_assetCode;
}

QString ContractDebitedEffectResponse::assetIssuer() const
{
    return m_assetIssuer;
}

void ContractDebitedEffectResponse::setAssetType(QString assetType)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetType = assetType;
}

void ContractDebitedEffectResponse::setAssetCode(QString assetCode)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetCode = assetCode;
}

void ContractDebitedEffectResponse::setAssetIssuer(QString assetIssuer)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetIssuer = assetIssuer;
}
QSTELLAR_END_NS
