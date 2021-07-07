#include "datasponsorshipremovedeffectresponse.h"

DataSponsorshipRemovedEffectResponse::DataSponsorshipRemovedEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply)
{

}

DataSponsorshipRemovedEffectResponse::~DataSponsorshipRemovedEffectResponse(){}

QString DataSponsorshipRemovedEffectResponse::getFormerSponsor() const
{
    return m_formerSponsor;
}

QString DataSponsorshipRemovedEffectResponse::getDataName() const
{
    return m_dataName;
}

void DataSponsorshipRemovedEffectResponse::setFormerSponsor(QString formerSponsor)
{
    if (m_formerSponsor == formerSponsor)
        return;

    m_formerSponsor = formerSponsor;
    emit formerSponsorChanged();
}

void DataSponsorshipRemovedEffectResponse::setDataName(QString dataName)
{
    if (m_dataName == dataName)
        return;

    m_dataName = dataName;
    emit dataNameChanged();
}
