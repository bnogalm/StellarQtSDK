#ifndef DATASPONSORSHIPCREATEDEFFECTRESPONSE_H
#define DATASPONSORSHIPCREATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
/**
 * Represents data_sponsorship_created effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class DataSponsorshipCreatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString sponsor READ getSponsor WRITE setSponsor NOTIFY sponsorChanged)
    Q_PROPERTY(QString data_name READ getDataName WRITE setDataName NOTIFY dataNameChanged)


    QString m_sponsor;
    QString m_dataName;

public:
    DataSponsorshipCreatedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~DataSponsorshipCreatedEffectResponse();;
    QString getSponsor() const;
    QString getDataName() const;

public slots:
    void setSponsor(QString sponsor);
    void setDataName(QString dataName);

signals:
    void sponsorChanged();
    void dataNameChanged();
};

#endif // DATASPONSORSHIPCREATEDEFFECTRESPONSE_H
