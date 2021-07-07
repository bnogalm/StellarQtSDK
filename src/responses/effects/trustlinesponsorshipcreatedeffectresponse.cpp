#include "trustlinesponsorshipcreatedeffectresponse.h"

TrustlineSponsorshipCreatedEffectResponse::TrustlineSponsorshipCreatedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply),m_asset(nullptr)
{

}

TrustlineSponsorshipCreatedEffectResponse::~TrustlineSponsorshipCreatedEffectResponse()
{
    if(m_asset)
        delete m_asset;
}

QString TrustlineSponsorshipCreatedEffectResponse::asset() const
{
    return m_assetString;
}

Asset *TrustlineSponsorshipCreatedEffectResponse::getAsset()
{
    if(!m_asset)
        m_asset=Asset::create(m_assetString);
    return m_asset;
}

QString TrustlineSponsorshipCreatedEffectResponse::getSponsor() const
{
    return m_sponsor;
}

void TrustlineSponsorshipCreatedEffectResponse::setAsset(QString assetString)
{
    if (m_assetString == assetString)
        return;
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetString = assetString;
    emit assetChanged();
}

void TrustlineSponsorshipCreatedEffectResponse::setSponsor(QString sponsor)
{
    if (m_sponsor == sponsor)
        return;

    m_sponsor = sponsor;
    emit sponsorChanged();
}
