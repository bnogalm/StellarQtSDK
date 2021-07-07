#ifndef CLAIMABLEBALANCESPONSORSHIPCREATEDEFFECTRESPONSE_H
#define CLAIMABLEBALANCESPONSORSHIPCREATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
/**
 * Represents claimable_balance_sponsorship_created effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class ClaimableBalanceSponsorshipCreatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString sponsor READ getSponsor WRITE setSponsor NOTIFY sponsorChanged)
    Q_PROPERTY(QString balance_id READ getBalanceID WRITE setBalanceID NOTIFY balanceIDChanged)

    QString m_sponsor;
    QString m_balanceID;

public:
    ClaimableBalanceSponsorshipCreatedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~ClaimableBalanceSponsorshipCreatedEffectResponse();
    QString getSponsor() const;
    QString getBalanceID() const;

public slots:
    void setSponsor(QString sponsor);
    void setBalanceID(QString balanceID);

signals:
    void sponsorChanged();
    void balanceIDChanged();
};

#endif // CLAIMABLEBALANCESPONSORSHIPCREATEDEFFECTRESPONSE_H
