#include "accountsponsorshipupdatedeffectresponse.h"



AccountSponsorshipUpdatedEffectResponse::AccountSponsorshipUpdatedEffectResponse(QNetworkReply *reply):EffectResponse(reply)
{

}

AccountSponsorshipUpdatedEffectResponse::~AccountSponsorshipUpdatedEffectResponse(){}

QString AccountSponsorshipUpdatedEffectResponse::getFormerSponsor() const
{
    return m_formerSponsor;
}

QString AccountSponsorshipUpdatedEffectResponse::getNewSponsor() const
{
    return m_newSponsor;
}

void AccountSponsorshipUpdatedEffectResponse::setFormerSponsor(QString former_sponsor)
{
    if (m_formerSponsor == former_sponsor)
        return;

    m_formerSponsor = former_sponsor;
    emit formerSponsorChanged();
}

void AccountSponsorshipUpdatedEffectResponse::setNewSponsor(QString new_sponsor)
{
    if (m_newSponsor == new_sponsor)
        return;

    m_newSponsor = new_sponsor;
    emit newSponsorChanged();
}
