#include "claimablebalancecreatedeffectresponse.h"

ClaimableBalanceCreatedEffectResponse::ClaimableBalanceCreatedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply),m_asset(nullptr)
{

}

ClaimableBalanceCreatedEffectResponse::~ClaimableBalanceCreatedEffectResponse()
{
    if(m_asset)
        delete m_asset;
}

QString ClaimableBalanceCreatedEffectResponse::asset() const
{
    return m_assetString;
}

Asset *ClaimableBalanceCreatedEffectResponse::getAsset()
{
    if(!m_asset)
        m_asset = Asset::create(m_assetString);
    return m_asset;
}

QString ClaimableBalanceCreatedEffectResponse::getAmount() const
{
    return m_amount;
}

QString ClaimableBalanceCreatedEffectResponse::getBalanceID() const
{
    return m_balanceID;
}

void ClaimableBalanceCreatedEffectResponse::setAsset(QString asset)
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

void ClaimableBalanceCreatedEffectResponse::setAmount(QString amount)
{
    if (m_amount == amount)
        return;

    m_amount = amount;
    emit amountChanged();
}

void ClaimableBalanceCreatedEffectResponse::setBalanceID(QString balanceID)
{
    if (m_balanceID == balanceID)
        return;

    m_balanceID = balanceID;
    emit balanceIDChanged();
}
