#include "claimablebalancesponsorshipcreatedeffectresponse.h"

ClaimableBalanceSponsorshipCreatedEffectResponse::ClaimableBalanceSponsorshipCreatedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply)
{

}

ClaimableBalanceSponsorshipCreatedEffectResponse::~ClaimableBalanceSponsorshipCreatedEffectResponse(){}

QString ClaimableBalanceSponsorshipCreatedEffectResponse::getSponsor() const
{
    return m_sponsor;
}

QString ClaimableBalanceSponsorshipCreatedEffectResponse::getBalanceID() const
{
    return m_balanceID;
}

void ClaimableBalanceSponsorshipCreatedEffectResponse::setSponsor(QString sponsor)
{
    if (m_sponsor == sponsor)
        return;

    m_sponsor = sponsor;
    emit sponsorChanged();
}

void ClaimableBalanceSponsorshipCreatedEffectResponse::setBalanceID(QString balanceID)
{
    if (m_balanceID == balanceID)
        return;

    m_balanceID = balanceID;
    emit balanceIDChanged();
}
