#include "accountsponsorshipremovedeffectresponse.h"

AccountSponsorshipRemovedEffectResponse::AccountSponsorshipRemovedEffectResponse(QNetworkReply* reply):EffectResponse(reply)
{

}

AccountSponsorshipRemovedEffectResponse::~AccountSponsorshipRemovedEffectResponse(){}

QString AccountSponsorshipRemovedEffectResponse::getFormerSponsor() const
{
    return m_formerSponsor;
}

void AccountSponsorshipRemovedEffectResponse::setFormerSponsor(QString former_sponsor)
{
    if (m_formerSponsor == former_sponsor)
        return;

    m_formerSponsor = former_sponsor;
    emit formerSponsorChanged();
}
