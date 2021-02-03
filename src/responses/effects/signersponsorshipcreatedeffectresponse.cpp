#include "signersponsorshipcreatedeffectresponse.h"

SignerSponsorshipCreatedEffectResponse::SignerSponsorshipCreatedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply)
{

}

SignerSponsorshipCreatedEffectResponse::~SignerSponsorshipCreatedEffectResponse(){}

QString SignerSponsorshipCreatedEffectResponse::getSponsor() const
{
    return m_sponsor;
}

QString SignerSponsorshipCreatedEffectResponse::getSigner() const
{
    return m_signer;
}

void SignerSponsorshipCreatedEffectResponse::setSponsor(QString sponsor)
{
    if (m_sponsor == sponsor)
        return;

    m_sponsor = sponsor;
    emit sponsorChanged();
}

void SignerSponsorshipCreatedEffectResponse::setSigner(QString signer)
{
    if (m_signer == signer)
        return;

    m_signer = signer;
    emit signerChanged();
}
