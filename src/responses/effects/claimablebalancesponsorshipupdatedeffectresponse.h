#ifndef CLAIMABLEBALANCESPONSORSHIPUPDATEDEFFECTRESPONSE_H
#define CLAIMABLEBALANCESPONSORSHIPUPDATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
/**
 * Represents claimable_balance_sponsorship_updated effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class ClaimableBalanceSponsorshipUpdatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString former_sponsor READ getFormerSponsor WRITE setFormerSponsor NOTIFY formerSponsorChanged)
    Q_PROPERTY(QString new_sponsor READ getNewSponsor WRITE setNewSponsor NOTIFY newSponsorChanged)
    Q_PROPERTY(QString balance_id READ getBalanceID WRITE setBalanceID NOTIFY balanceIDChanged)

    QString m_formerSponsor;
    QString m_newSponsor;
    QString m_balanceID;

public:
    ClaimableBalanceSponsorshipUpdatedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~ClaimableBalanceSponsorshipUpdatedEffectResponse();;
    QString getFormerSponsor() const;
    QString getNewSponsor() const;
    QString getBalanceID() const;

public slots:
    void setFormerSponsor(QString formerSponsor);
    void setNewSponsor(QString newSponsor);
    void setBalanceID(QString balanceID);

signals:
    void formerSponsorChanged();
    void newSponsorChanged();
    void balanceIDChanged();
};

#endif // CLAIMABLEBALANCESPONSORSHIPUPDATEDEFFECTRESPONSE_H
