#ifndef ACCOUNTSPONSORSHIPREMOVEDEFFECTRESPONSE_H
#define ACCOUNTSPONSORSHIPREMOVEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
/**
 * Represents account_sponsorship_removed effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class AccountSponsorshipRemovedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString former_sponsor READ getFormerSponsor WRITE setFormerSponsor NOTIFY formerSponsorChanged)
    QString m_formerSponsor;

public:
    AccountSponsorshipRemovedEffectResponse(QNetworkReply* reply=nullptr);
    ~AccountSponsorshipRemovedEffectResponse();
    QString getFormerSponsor() const;
public slots:
    void setFormerSponsor(QString former_sponsor);
signals:
    void formerSponsorChanged();
};

#endif // ACCOUNTSPONSORSHIPREMOVEDEFFECTRESPONSE_H
