#ifndef CLAWBACKOPERATIONRESPONSE_H
#define CLAWBACKOPERATIONRESPONSE_H

#include "operationresponse.h"


class Asset;

/**
 * Represents a Clawback operation response.
 *
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class ClawbackOperationResponse : public OperationResponse
{
    Q_OBJECT

    Q_PROPERTY(QString asset_type READ getAssetType WRITE setAssetType NOTIFY assetTypeChanged)
    Q_PROPERTY(QString asset_code READ getAssetCode WRITE setAssetCode NOTIFY assetCodeChanged)
    Q_PROPERTY(QString asset_issuer READ getAssetIssuer WRITE setAssetIssuer NOTIFY assetIssuerChanged)
    Q_PROPERTY(QString amount READ getAmount WRITE setAmount NOTIFY amountChanged)
    Q_PROPERTY(QString from READ getFrom WRITE setFrom NOTIFY fromChanged)


    QString m_assetType;

    QString m_assetCode;

    QString m_assetIssuer;

    QString m_amount;

    QString m_from;

    Asset* m_asset;

public:
    ClawbackOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~ClawbackOperationResponse();
    const QString getAssetType() const;
    void setAssetType(const QString newAsset_type);
    const QString getAssetCode() const;
    void setAssetCode(const QString newAsset_code);

    const QString getAssetIssuer() const;
    void setAssetIssuer(const QString newAsset_issuer);

    const QString getAmount() const;
    void setAmount(const QString newAmount);

    const QString getFrom() const;
    void setFrom(const QString newFrom);

    Asset* getAsset();
signals:
    void assetTypeChanged();
    void assetCodeChanged();
    void assetIssuerChanged();
    void amountChanged();
    void fromChanged();
};

#endif // CLAWBACKOPERATIONRESPONSE_H
