#ifndef CLAIMABLEBALANCECLAIMEDEFFECTRESPONSE_H
#define CLAIMABLEBALANCECLAIMEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "asset.h"
#include <QObject>

/**
 * Represents claimable_balance_claimed effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class ClaimableBalanceClaimedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString asset READ asset WRITE setAsset NOTIFY assetChanged)
    Q_PROPERTY(QString amount READ getAmount WRITE setAmount NOTIFY amountChanged)
    Q_PROPERTY(QString balance_id READ getBalanceID WRITE setBalanceID NOTIFY balanceIDChanged)

    QString m_assetString;

    QString m_amount;

    QString m_balanceID;

    Asset* m_asset;
public:
    ClaimableBalanceClaimedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~ClaimableBalanceClaimedEffectResponse();;
    QString asset() const;
    Asset* getAsset();
    QString getAmount() const;

    QString getBalanceID() const;

public slots:
    void setAsset(QString asset);
    void setAmount(QString amount);
    void setBalanceID(QString balance_id);

signals:
    void assetChanged();
    void amountChanged();
    void balanceIDChanged();
};

#endif // CLAIMABLEBALANCECLAIMEDEFFECTRESPONSE_H
