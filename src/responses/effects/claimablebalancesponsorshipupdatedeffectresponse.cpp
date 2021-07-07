#include "claimablebalancesponsorshipupdatedeffectresponse.h"

ClaimableBalanceSponsorshipUpdatedEffectResponse::ClaimableBalanceSponsorshipUpdatedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply)
{

}

ClaimableBalanceSponsorshipUpdatedEffectResponse::~ClaimableBalanceSponsorshipUpdatedEffectResponse(){}

QString ClaimableBalanceSponsorshipUpdatedEffectResponse::getFormerSponsor() const
{
    return m_formerSponsor;
}

QString ClaimableBalanceSponsorshipUpdatedEffectResponse::getNewSponsor() const
{
    return m_newSponsor;
}

QString ClaimableBalanceSponsorshipUpdatedEffectResponse::getBalanceID() const
{
    return m_balanceID;
}

void ClaimableBalanceSponsorshipUpdatedEffectResponse::setFormerSponsor(QString formerSponsor)
{
    if (m_formerSponsor == formerSponsor)
        return;

    m_formerSponsor = formerSponsor;
    emit formerSponsorChanged();
}

void ClaimableBalanceSponsorshipUpdatedEffectResponse::setNewSponsor(QString newSponsor)
{
    if (m_newSponsor == newSponsor)
        return;

    m_newSponsor = newSponsor;
    emit newSponsorChanged();
}

void ClaimableBalanceSponsorshipUpdatedEffectResponse::setBalanceID(QString balanceID)
{
    if (m_balanceID == balanceID)
        return;

    m_balanceID = balanceID;
    emit balanceIDChanged();
}
