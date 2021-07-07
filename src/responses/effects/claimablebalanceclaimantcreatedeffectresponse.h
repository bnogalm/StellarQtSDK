#ifndef CLAIMABLEBALANCECLAIMANTCREATEDEFFECTRESPONSE_H
#define CLAIMABLEBALANCECLAIMANTCREATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include <QObject>
#include "predicate.h"
#include "asset.h"
/**
 * Represents claimable_balance_claimed effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */

class ClaimableBalanceClaimantCreatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString asset READ asset WRITE setAsset NOTIFY assetChanged)
    Q_PROPERTY(QString amount READ getAmount WRITE setAmount NOTIFY amountChanged)
    Q_PROPERTY(QString balance_id READ getBalanceID WRITE setBalanceID NOTIFY balanceIDChanged)
    Q_PROPERTY(Predicate* predicate READ predicate WRITE setPredicate NOTIFY predicateChanged)
    QString m_assetString;
    Asset* m_asset;
    QString m_amount;
    QString m_balanceID;
    Predicate* m_predicate;

public:
    ClaimableBalanceClaimantCreatedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~ClaimableBalanceClaimantCreatedEffectResponse();
    QString asset() const;
    Asset* getAsset();
    QString getAmount() const;

    QString getBalanceID() const;

    Predicate* predicate() const;

public slots:
    void setAsset(QString asset);
    void setAmount(QString amount);
    void setBalanceID(QString balance_id);
    void setPredicate(Predicate* predicate);

signals:
    void assetChanged();
    void amountChanged();
    void balanceIDChanged();
    void predicateChanged();
};

#endif // CLAIMABLEBALANCECLAIMANTCREATEDEFFECTRESPONSE_H
