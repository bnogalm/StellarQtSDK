#include "claimablebalanceclaimedeffectresponse.h"


ClaimableBalanceClaimedEffectResponse::ClaimableBalanceClaimedEffectResponse(QNetworkReply* reply):EffectResponse(reply),m_asset(nullptr)
{

}

ClaimableBalanceClaimedEffectResponse::~ClaimableBalanceClaimedEffectResponse(){
    if(m_asset)
        delete m_asset;
}

QString ClaimableBalanceClaimedEffectResponse::asset() const
{
    return m_assetString;
}

Asset *ClaimableBalanceClaimedEffectResponse::getAsset()
{
    if(!m_asset)
        m_asset= Asset::create(m_assetString);
    return m_asset;
}

QString ClaimableBalanceClaimedEffectResponse::getAmount() const
{
    return m_amount;
}

QString ClaimableBalanceClaimedEffectResponse::getBalanceID() const
{
    return m_balanceID;
}

void ClaimableBalanceClaimedEffectResponse::setAsset(QString asset)
{
    if (m_assetString == asset)
        return;

    if(m_asset){
        delete m_asset;
        m_asset=nullptr;
    }

    m_assetString = asset;
    emit assetChanged();
}

void ClaimableBalanceClaimedEffectResponse::setAmount(QString amount)
{
    if (m_amount == amount)
        return;

    m_amount = amount;
    emit amountChanged();
}

void ClaimableBalanceClaimedEffectResponse::setBalanceID(QString balance_id)
{
    if (m_balanceID == balance_id)
        return;

    m_balanceID = balance_id;
    emit balanceIDChanged();
}
