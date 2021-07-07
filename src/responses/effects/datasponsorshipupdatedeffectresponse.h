#ifndef DATASPONSORSHIPUPDATEDEFFECTRESPONSE_H
#define DATASPONSORSHIPUPDATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
/**
 * Represents data_sponsorship_updated effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class DataSponsorshipUpdatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString former_sponsor READ getFormerSponsor WRITE setFormerSponsor NOTIFY formerSponsorChanged)
    Q_PROPERTY(QString new_sponsor READ getNewSponsor WRITE setNewSponsor NOTIFY newSponsorChanged)
    Q_PROPERTY(QString data_name READ getDataName WRITE setDataName NOTIFY dataNameChanged)

    QString m_formerSponsor;
    QString m_newSponsor;
    QString m_dataName;

public:
    DataSponsorshipUpdatedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~DataSponsorshipUpdatedEffectResponse();;
    QString getFormerSponsor() const;
    QString getNewSponsor() const;

    QString getDataName() const;

public slots:
    void setFormerSponsor(QString formerSponsor);
    void setNewSponsor(QString newSponsor);

    void setDataName(QString dataName);

signals:
    void formerSponsorChanged();
    void newSponsorChanged();
    void dataNameChanged();
};

#endif // DATASPONSORSHIPUPDATEDEFFECTRESPONSE_H
