#include "clawbackoperationresponse.h"
#include "keypair.h"
#include "asset.h"

ClawbackOperationResponse::ClawbackOperationResponse(QNetworkReply *reply):OperationResponse(reply), m_asset(nullptr)
{

}

ClawbackOperationResponse::~ClawbackOperationResponse()
{

    if(m_asset)
        delete m_asset;
}

const QString ClawbackOperationResponse::getAssetType() const
{
    return m_assetType;
}

void ClawbackOperationResponse::setAssetType(const QString newAsset_type)
{
    if (m_assetType == newAsset_type)
        return;
    m_assetType = newAsset_type;
    emit assetTypeChanged();
}

const QString ClawbackOperationResponse::getAssetCode() const
{
    return m_assetCode;
}

void ClawbackOperationResponse::setAssetCode(const QString newAsset_code)
{
    if (m_assetCode == newAsset_code)
        return;
    m_assetCode = newAsset_code;
    emit assetCodeChanged();
}

const QString ClawbackOperationResponse::getAssetIssuer() const
{
    return m_assetIssuer;
}

void ClawbackOperationResponse::setAssetIssuer(const QString newAsset_issuer)
{
    if (m_assetIssuer == newAsset_issuer)
        return;
    m_assetIssuer = newAsset_issuer;
    emit assetIssuerChanged();
}

const QString ClawbackOperationResponse::getAmount() const
{
    return m_amount;
}

void ClawbackOperationResponse::setAmount(const QString newAmount)
{
    if (m_amount == newAmount)
        return;
    m_amount = newAmount;
    emit amountChanged();
}

const QString ClawbackOperationResponse::getFrom() const
{
    return m_from;
}

void ClawbackOperationResponse::setFrom(const QString newFrom)
{
    if (m_from == newFrom)
        return;
    m_from = newFrom;
    emit fromChanged();
}

Asset *ClawbackOperationResponse::getAsset()
{
    if(!m_asset){
        KeyPair* issuer = KeyPair::fromAccountId(m_assetIssuer);
        m_asset= Asset::createNonNativeAsset(m_assetCode, issuer);
    }
    return m_asset;
}
