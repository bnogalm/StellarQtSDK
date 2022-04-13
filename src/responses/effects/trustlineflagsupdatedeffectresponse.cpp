#include "trustlineflagsupdatedeffectresponse.h"
#include "asset.h"
#include "keypair.h"


TrustlineFlagsUpdatedEffectResponse::TrustlineFlagsUpdatedEffectResponse(QNetworkReply *reply):EffectResponse(reply)
  ,m_authorizedFlag(false)
  ,m_authorizedToMaintainLiabilitiesFlag(false)
  ,m_clawbackEnabledFlag(false)
  ,m_asset(nullptr)
{

}

TrustlineFlagsUpdatedEffectResponse::~TrustlineFlagsUpdatedEffectResponse()
{
    if(m_asset)
        delete m_asset;
}

Asset *TrustlineFlagsUpdatedEffectResponse::getAsset() {
    if(!m_asset){
        KeyPair* issuer = KeyPair::fromAccountId(m_assetIssuer);
        m_asset= Asset::createNonNativeAsset(m_assetCode, issuer);
    }
    return m_asset;
}

const QString TrustlineFlagsUpdatedEffectResponse::getTrustor() const
{
    return m_trustor;
}

void TrustlineFlagsUpdatedEffectResponse::setTrustor(const QString newTrustor)
{
    if (m_trustor == newTrustor)
        return;
    m_trustor = newTrustor;
    emit trustorChanged();
}

const QString TrustlineFlagsUpdatedEffectResponse::getAssetType() const
{
    return m_assetType;
}

void TrustlineFlagsUpdatedEffectResponse::setAssetType(const QString newAsset_type)
{
    if (m_assetType == newAsset_type)
        return;
    m_assetType = newAsset_type;
    emit assetTypeChanged();
}

const QString TrustlineFlagsUpdatedEffectResponse::getAssetCode() const
{
    return m_assetCode;
}

void TrustlineFlagsUpdatedEffectResponse::setAssetCode(const QString newAsset_code)
{
    if (m_assetCode == newAsset_code)
        return;
    m_assetCode = newAsset_code;
    emit assetCodeChanged();
}

const QString TrustlineFlagsUpdatedEffectResponse::getAssetIssuer() const
{
    return m_assetIssuer;
}

void TrustlineFlagsUpdatedEffectResponse::setAssetIssuer(const QString newAsset_issuer)
{
    if (m_assetIssuer == newAsset_issuer)
        return;
    m_assetIssuer = newAsset_issuer;
    emit assetIssuerChanged();
}

bool TrustlineFlagsUpdatedEffectResponse::getAuthorized() const
{
    return m_authorizedFlag;
}

void TrustlineFlagsUpdatedEffectResponse::setAuthorized(bool newAuthorized_flag)
{
    if (m_authorizedFlag == newAuthorized_flag)
        return;
    m_authorizedFlag = newAuthorized_flag;
    emit authorizedChanged();
}

bool TrustlineFlagsUpdatedEffectResponse::getAuthorizedToMaintainLiabilities() const
{
    return m_authorizedToMaintainLiabilitiesFlag;
}

void TrustlineFlagsUpdatedEffectResponse::setAuthorizedToMaintainLiabilities(bool newAuthorized_to_maintain_liabilities_flag)
{
    if (m_authorizedToMaintainLiabilitiesFlag == newAuthorized_to_maintain_liabilities_flag)
        return;
    m_authorizedToMaintainLiabilitiesFlag = newAuthorized_to_maintain_liabilities_flag;
    emit authorizedToMaintainLiabilitiesChanged();
}

bool TrustlineFlagsUpdatedEffectResponse::getClawbackEnabled() const
{
    return m_clawbackEnabledFlag;
}

void TrustlineFlagsUpdatedEffectResponse::setClawbackEnabled(bool newClawback_enabled_flag)
{
    if (m_clawbackEnabledFlag == newClawback_enabled_flag)
        return;
    m_clawbackEnabledFlag = newClawback_enabled_flag;
    emit clawbackEnabledChanged();
}
