#ifndef CLAIMABLEBALANCECREATEDEFFECTRESPONSE_H
#define CLAIMABLEBALANCECREATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "asset.h"

#include <QObject>
/**
 * Represents claimable_balance_created effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class ClaimableBalanceCreatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString asset READ asset WRITE setAsset NOTIFY assetChanged)
    Q_PROPERTY(QString amount READ getAmount WRITE setAmount NOTIFY amountChanged)
    Q_PROPERTY(QString balance_id READ getBalanceID WRITE setBalanceID NOTIFY balanceIDChanged)

    QString m_assetString;
    Asset* m_asset;
    QString m_amount;
    QString m_balanceID;

public:
    ClaimableBalanceCreatedEffectResponse(QNetworkReply* reply=nullptr);
    virtual ~ClaimableBalanceCreatedEffectResponse();
    QString asset() const;
    Asset* getAsset();
    QString getAmount() const;

    QString getBalanceID() const;

public slots:
    void setAsset(QString asset);
    void setAmount(QString amount);
    void setBalanceID(QString balanceID);

signals:
    void assetChanged();
    void amountChanged();
    void balanceIDChanged();
};

#endif // CLAIMABLEBALANCECREATEDEFFECTRESPONSE_H
