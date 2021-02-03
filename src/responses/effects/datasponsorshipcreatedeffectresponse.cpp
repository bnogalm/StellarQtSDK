#include "datasponsorshipcreatedeffectresponse.h"

DataSponsorshipCreatedEffectResponse::DataSponsorshipCreatedEffectResponse(QNetworkReply* reply)
    :EffectResponse(reply)
{

}

DataSponsorshipCreatedEffectResponse::~DataSponsorshipCreatedEffectResponse(){}

QString DataSponsorshipCreatedEffectResponse::getSponsor() const
{
    return m_sponsor;
}

QString DataSponsorshipCreatedEffectResponse::getDataName() const
{
    return m_dataName;
}

void DataSponsorshipCreatedEffectResponse::setSponsor(QString sponsor)
{
    if (m_sponsor == sponsor)
        return;

    m_sponsor = sponsor;
    emit sponsorChanged();
}

void DataSponsorshipCreatedEffectResponse::setDataName(QString dataName)
{
    if (m_dataName == dataName)
        return;

    m_dataName = dataName;
    emit dataNameChanged();
}
