#ifndef CLAIMABLEBALANCESPONSORSHIPREMOVEDEFFECTRESPONSE_H
#define CLAIMABLEBALANCESPONSORSHIPREMOVEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
/**
 * Represents claimable_balance_sponsorship_removed effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class ClaimableBalanceSponsorshipRemovedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString former_sponsor READ getFormerSponsor WRITE setFormerSponsor NOTIFY formerSponsorChanged)
    Q_PROPERTY(QString balance_id READ getBalanceID WRITE setBalanceID NOTIFY balanceIDChanged)


    QString m_formerSponsor;
    QString m_balanceID;

public:
    ClaimableBalanceSponsorshipRemovedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~ClaimableBalanceSponsorshipRemovedEffectResponse();;
    QString getFormerSponsor() const;
    QString getBalanceID() const;

public slots:
    void setFormerSponsor(QString formerSponsor);
    void setBalanceID(QString balanceID);

signals:
    void formerSponsorChanged();
    void balanceIDChanged(QString balanceID);
};

#endif // CLAIMABLEBALANCESPONSORSHIPREMOVEDEFFECTRESPONSE_H
