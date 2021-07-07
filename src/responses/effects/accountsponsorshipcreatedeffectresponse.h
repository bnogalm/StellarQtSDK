#ifndef ACCOUNTSPONSORSHIPCREATEDEFFECTRESPONSE_H
#define ACCOUNTSPONSORSHIPCREATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
/**
 * Represents account_sponsorship_created effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class AccountSponsorshipCreatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString sponsor READ getSponsor WRITE setSponsor NOTIFY sponsorChanged)
    QString m_sponsor;

public:
    AccountSponsorshipCreatedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~AccountSponsorshipCreatedEffectResponse();
    QString getSponsor() const;
public slots:
    void setSponsor(QString sponsor);
signals:
    void sponsorChanged();
};

#endif // ACCOUNTSPONSORSHIPCREATEDEFFECTRESPONSE_H
