#include "claimablebalancesponsorshipremovedeffectresponse.h"

ClaimableBalanceSponsorshipRemovedEffectResponse::ClaimableBalanceSponsorshipRemovedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply)
{

}

ClaimableBalanceSponsorshipRemovedEffectResponse::~ClaimableBalanceSponsorshipRemovedEffectResponse(){}

QString ClaimableBalanceSponsorshipRemovedEffectResponse::getFormerSponsor() const
{
    return m_formerSponsor;
}

QString ClaimableBalanceSponsorshipRemovedEffectResponse::getBalanceID() const
{
    return m_balanceID;
}

void ClaimableBalanceSponsorshipRemovedEffectResponse::setFormerSponsor(QString formerSponsor)
{
    if (m_formerSponsor == formerSponsor)
        return;

    m_formerSponsor = formerSponsor;
    emit formerSponsorChanged();
}

void ClaimableBalanceSponsorshipRemovedEffectResponse::setBalanceID(QString balanceID)
{
    if (m_balanceID == balanceID)
        return;

    m_balanceID = balanceID;
    emit balanceIDChanged(m_balanceID);
}
