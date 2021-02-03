#include "claimablebalanceclaimantcreatedeffectresponse.h"

ClaimableBalanceClaimantCreatedEffectResponse::ClaimableBalanceClaimantCreatedEffectResponse(QNetworkReply* reply)
    :EffectResponse(reply),m_asset(nullptr),m_predicate(nullptr)
{

}

ClaimableBalanceClaimantCreatedEffectResponse::~ClaimableBalanceClaimantCreatedEffectResponse()
{
    if(m_asset)
        delete m_asset;
    if(m_predicate)
        delete m_predicate;
}

Asset *ClaimableBalanceClaimantCreatedEffectResponse::getAsset()
{
    if(!m_asset)
        m_asset = Asset::create(m_assetString);
    return m_asset;
}

QString ClaimableBalanceClaimantCreatedEffectResponse::asset() const
{
    return m_assetString;
}

QString ClaimableBalanceClaimantCreatedEffectResponse::getAmount() const
{
    return m_amount;
}

QString ClaimableBalanceClaimantCreatedEffectResponse::getBalanceID() const
{
    return m_balanceID;
}

Predicate *ClaimableBalanceClaimantCreatedEffectResponse::predicate() const
{
    return m_predicate;
}

void ClaimableBalanceClaimantCreatedEffectResponse::setAsset(QString asset)
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

void ClaimableBalanceClaimantCreatedEffectResponse::setAmount(QString amount)
{
    if (m_amount == amount)
        return;

    m_amount = amount;
    emit amountChanged();
}

void ClaimableBalanceClaimantCreatedEffectResponse::setBalanceID(QString balance_id)
{
    if (m_balanceID == balance_id)
        return;

    m_balanceID = balance_id;
    emit balanceIDChanged();
}

void ClaimableBalanceClaimantCreatedEffectResponse::setPredicate(Predicate *predicate)
{
    if (m_predicate == predicate)
        return;

    m_predicate = predicate;
    emit predicateChanged();
}
