#ifndef ACCOUNTSPONSORSHIPUPDATEDEFFECTRESPONSE_H
#define ACCOUNTSPONSORSHIPUPDATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
/**
 * Represents account_sponsorship_updated effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class AccountSponsorshipUpdatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString former_sponsor READ getFormerSponsor WRITE setFormerSponsor NOTIFY formerSponsorChanged)
    Q_PROPERTY(QString new_sponsor READ getNewSponsor WRITE setNewSponsor NOTIFY newSponsorChanged)
    QString m_formerSponsor;


    QString m_newSponsor;

public:
    AccountSponsorshipUpdatedEffectResponse(QNetworkReply* reply=nullptr);
    ~AccountSponsorshipUpdatedEffectResponse();
    QString getFormerSponsor() const;
    QString getNewSponsor() const;

public slots:
    void setFormerSponsor(QString former_sponsor);
    void setNewSponsor(QString new_sponsor);

signals:
    void formerSponsorChanged();
    void newSponsorChanged();
};

#endif // ACCOUNTSPONSORSHIPUPDATEDEFFECTRESPONSE_H
