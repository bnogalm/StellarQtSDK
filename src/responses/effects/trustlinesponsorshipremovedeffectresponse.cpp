#include "trustlinesponsorshipremovedeffectresponse.h"

TrustlineSponsorshipRemovedEffectResponse::TrustlineSponsorshipRemovedEffectResponse(QNetworkReply* reply)
    :EffectResponse(reply),m_asset(nullptr)
{

}

TrustlineSponsorshipRemovedEffectResponse::~TrustlineSponsorshipRemovedEffectResponse()
{
    if(m_asset)
        delete m_asset;
}

QString TrustlineSponsorshipRemovedEffectResponse::asset() const
{
    return m_assetString;
}

Asset *TrustlineSponsorshipRemovedEffectResponse::getAsset(){
    if(!m_asset)
        m_asset = Asset::create(m_assetString);
    return m_asset;
}

QString TrustlineSponsorshipRemovedEffectResponse::getFormerSponsor() const
{
    return m_formerSponsor;
}

void TrustlineSponsorshipRemovedEffectResponse::setAsset(QString assetString)
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

void TrustlineSponsorshipRemovedEffectResponse::setFormerSponsor(QString formerSponsor)
{
    if (m_formerSponsor == formerSponsor)
        return;

    m_formerSponsor = formerSponsor;
    emit formerSponsorChanged();
}
