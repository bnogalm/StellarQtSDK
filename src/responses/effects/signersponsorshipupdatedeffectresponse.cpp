#include "signersponsorshipupdatedeffectresponse.h"

SignerSponsorshipUpdatedEffectResponse::SignerSponsorshipUpdatedEffectResponse(QNetworkReply* reply)
    :EffectResponse(reply)
{

}

SignerSponsorshipUpdatedEffectResponse::~SignerSponsorshipUpdatedEffectResponse(){}

QString SignerSponsorshipUpdatedEffectResponse::getFormerSponsor() const
{
    return m_formerSponsor;
}

QString SignerSponsorshipUpdatedEffectResponse::getNewSponsor() const
{
    return m_newSponsor;
}

QString SignerSponsorshipUpdatedEffectResponse::getSigner() const
{
    return m_signer;
}

void SignerSponsorshipUpdatedEffectResponse::setFormerSponsor(QString formerSponsor)
{
    if (m_formerSponsor == formerSponsor)
        return;

    m_formerSponsor = formerSponsor;
    emit formerSponsorChanged();
}

void SignerSponsorshipUpdatedEffectResponse::setNewSponsor(QString newSponsor)
{
    if (m_newSponsor == newSponsor)
        return;

    m_newSponsor = newSponsor;
    emit newSponsorChanged();
}

void SignerSponsorshipUpdatedEffectResponse::setSigner(QString signer)
{
    if (m_signer == signer)
        return;

    m_signer = signer;
    emit signerChanged();
}
