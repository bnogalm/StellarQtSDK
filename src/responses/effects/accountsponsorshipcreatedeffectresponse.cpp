#include "accountsponsorshipcreatedeffectresponse.h"

AccountSponsorshipCreatedEffectResponse::AccountSponsorshipCreatedEffectResponse(QNetworkReply *reply):EffectResponse(reply)
{

}

AccountSponsorshipCreatedEffectResponse::~AccountSponsorshipCreatedEffectResponse()
{

}

QString AccountSponsorshipCreatedEffectResponse::getSponsor() const
{
    return m_sponsor;
}

void AccountSponsorshipCreatedEffectResponse::setSponsor(QString sponsor)
{
    if (m_sponsor == sponsor)
        return;

    m_sponsor = sponsor;
    emit sponsorChanged();
}
