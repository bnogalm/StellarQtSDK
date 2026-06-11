#ifndef CLAWBACKOPERATIONRESPONSE_H
#define CLAWBACKOPERATIONRESPONSE_H

#include "operationresponse.h"
#include "qstellar_namespace.h"


QSTELLAR_BEGIN_NS

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
    Q_PROPERTY(QString from_muxed MEMBER m_fromMuxed)
    Q_PROPERTY(QString from_muxed_id MEMBER m_fromMuxedId)


    QString m_assetType;

    QString m_assetCode;

    QString m_assetIssuer;

    QString m_amount;

    QString m_from;
    QString m_fromMuxed;
    QString m_fromMuxedId;

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
    QString getFromMuxed() const { return m_fromMuxed; }
    QString getFromMuxedId() const { return m_fromMuxedId; }

    Asset* getAsset();
signals:
    void assetTypeChanged();
    void assetCodeChanged();
    void assetIssuerChanged();
    void amountChanged();
    void fromChanged();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(ClawbackOperationResponse)




#endif // CLAWBACKOPERATIONRESPONSE_H
