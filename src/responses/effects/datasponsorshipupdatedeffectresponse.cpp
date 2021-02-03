#include "datasponsorshipupdatedeffectresponse.h"

DataSponsorshipUpdatedEffectResponse::DataSponsorshipUpdatedEffectResponse(QNetworkReply* reply)
    :EffectResponse(reply)
{

}

DataSponsorshipUpdatedEffectResponse::~DataSponsorshipUpdatedEffectResponse(){}

QString DataSponsorshipUpdatedEffectResponse::getFormerSponsor() const
{
    return m_formerSponsor;
}

QString DataSponsorshipUpdatedEffectResponse::getNewSponsor() const
{
    return m_newSponsor;
}

QString DataSponsorshipUpdatedEffectResponse::getDataName() const
{
    return m_dataName;
}

void DataSponsorshipUpdatedEffectResponse::setFormerSponsor(QString formerSponsor)
{
    if (m_formerSponsor == formerSponsor)
        return;

    m_formerSponsor = formerSponsor;
    emit formerSponsorChanged();
}

void DataSponsorshipUpdatedEffectResponse::setNewSponsor(QString newSponsor)
{
    if (m_newSponsor == newSponsor)
        return;

    m_newSponsor = newSponsor;
    emit newSponsorChanged();
}

void DataSponsorshipUpdatedEffectResponse::setDataName(QString dataName)
{
    if (m_dataName == dataName)
        return;

    m_dataName = dataName;
    emit dataNameChanged();
}
