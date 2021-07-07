#include "trustlinesponsorshipupdatedeffectresponse.h"

TrustlineSponsorshipUpdatedEffectResponse::TrustlineSponsorshipUpdatedEffectResponse(QNetworkReply* reply)
    :EffectResponse(reply),m_asset(nullptr)
{

}

TrustlineSponsorshipUpdatedEffectResponse::~TrustlineSponsorshipUpdatedEffectResponse()
{
    if(m_asset)
        delete m_asset;
}

QString TrustlineSponsorshipUpdatedEffectResponse::asset() const
{
    return m_assetString;
}

Asset *TrustlineSponsorshipUpdatedEffectResponse::getAsset()
{
    if(!m_asset)
        m_asset= Asset::create(m_assetString);
    return m_asset;
}

QString TrustlineSponsorshipUpdatedEffectResponse::getFormerSponsor() const
{
    return m_formerSponsor;
}

QString TrustlineSponsorshipUpdatedEffectResponse::getNewSponsor() const
{
    return m_newSponsor;
}

void TrustlineSponsorshipUpdatedEffectResponse::setAsset(QString assetString)
{
    if (m_assetString == assetString)
        return;
    if(m_asset)
    {
        delete m_asset;
        m_asset= nullptr;
    }
    m_assetString = assetString;
    emit assetChanged();
}

void TrustlineSponsorshipUpdatedEffectResponse::setFormerSponsor(QString formerSponsor)
{
    if (m_formerSponsor == formerSponsor)
        return;

    m_formerSponsor = formerSponsor;
    emit formerSponsorChanged();
}

void TrustlineSponsorshipUpdatedEffectResponse::setNewSponsor(QString newSponsor)
{
    if (m_newSponsor == newSponsor)
        return;

    m_newSponsor = newSponsor;
    emit newSponsorChanged();
}
