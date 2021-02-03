#include "signersponsorshipremovedeffectresponse.h"

SignerSponsorshipRemovedEffectResponse::SignerSponsorshipRemovedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply)
{

}

SignerSponsorshipRemovedEffectResponse::~SignerSponsorshipRemovedEffectResponse(){}

QString SignerSponsorshipRemovedEffectResponse::getFormerSponsor() const
{
    return m_formerSponsor;
}

QString SignerSponsorshipRemovedEffectResponse::getSigner() const
{
    return m_signer;
}

void SignerSponsorshipRemovedEffectResponse::setFormerSponsor(QString formerSponsor)
{
    if (m_formerSponsor == formerSponsor)
        return;

    m_formerSponsor = formerSponsor;
    emit formerSponsorChanged();
}

void SignerSponsorshipRemovedEffectResponse::setSigner(QString signer)
{
    if (m_signer == signer)
        return;

    m_signer = signer;
    emit signerChanged();
}
