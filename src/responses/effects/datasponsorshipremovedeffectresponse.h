#ifndef DATASPONSORSHIPREMOVEDEFFECTRESPONSE_H
#define DATASPONSORSHIPREMOVEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
/**
 * Represents data_sponsorship_removed effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class DataSponsorshipRemovedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString former_sponsor READ getFormerSponsor WRITE setFormerSponsor NOTIFY formerSponsorChanged)
    Q_PROPERTY(QString data_name READ getDataName WRITE setDataName NOTIFY dataNameChanged)


    QString m_formerSponsor;
    QString m_dataName;

public:
    DataSponsorshipRemovedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~DataSponsorshipRemovedEffectResponse();;
    QString getFormerSponsor() const;
    QString getDataName() const;

public slots:
    void setFormerSponsor(QString formerSponsor);
    void setDataName(QString dataName);

signals:
    void formerSponsorChanged();
    void dataNameChanged();
};

#endif // DATASPONSORSHIPREMOVEDEFFECTRESPONSE_H
