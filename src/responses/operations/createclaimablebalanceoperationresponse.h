#ifndef CREATECLAIMABLEBALANCEOPERATIONRESPONSE_H
#define CREATECLAIMABLEBALANCEOPERATIONRESPONSE_H

#include "operationresponse.h"
#include <QObject>
#include "claimant.h"
#include "asset.h"

/**
 * Represents CreateClaimableBalance operation response.
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class CreateClaimableBalanceOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString asset READ asset WRITE setAsset NOTIFY assetChanged)
    Q_PROPERTY(QString amount READ getAmount WRITE setAmount NOTIFY amountChanged)
    Q_PROPERTY(QList<Claimant> claimants READ getClaimants WRITE setClaimants NOTIFY claimantsChanged)



    QString m_assetString;
    Asset* m_asset;
    QString m_amount;

    QList<Claimant> m_claimants;

public:
    CreateClaimableBalanceOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~CreateClaimableBalanceOperationResponse();
    QString asset() const;
    Asset* getAsset();
    QString getAmount() const;
    QList<Claimant> getClaimants() const;

public slots:
    void setAsset(QString assetString);
    void setAmount(QString amount);

    void setClaimants(QList<Claimant> claimants);

signals:
    void assetChanged();
    void amountChanged();
    void claimantsChanged();
};

#endif // CREATECLAIMABLEBALANCEOPERATIONRESPONSE_H
