#ifndef PATHPAYMENTOPERATIONRESPONSE_H
#define PATHPAYMENTOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"

class Asset;
/**
 * Represents PathPayment operation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class PathPaymentOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString amount MEMBER m_amount)
    Q_PROPERTY(QString source_amount MEMBER m_sourceAmount)
    Q_PROPERTY(QString source_max MEMBER m_sourceMax)
    Q_PROPERTY(QString from READ from WRITE setFrom)
    Q_PROPERTY(QString to READ to WRITE setTo)
    Q_PROPERTY(QString asset_type READ assetType WRITE setAssetType)
    Q_PROPERTY(QString asset_code READ assetCode WRITE setAssetCode)
    Q_PROPERTY(QString asset_issuer READ assetIssuer WRITE setAssetIssuer)
    Q_PROPERTY(QString source_asset_type READ sourceAssetType WRITE setSourceAssetType)
    Q_PROPERTY(QString source_asset_code READ sourceAssetCode WRITE setSourceAssetCode)
    Q_PROPERTY(QString source_asset_issuer READ sourceAssetIssuer WRITE setSourceAssetIssuer)

    QString m_amount;
    QString m_sourceAmount;
    QString m_sourceMax;

    QString m_from;
    KeyPair* m_fromKeypair;

    QString m_to;
    KeyPair* m_toKeypair;

    QString m_assetType;
    QString m_assetCode;
    QString m_assetIssuer;
    Asset * m_asset;

    QString m_sourceAssetType;
    QString m_sourceAssetCode;
    QString m_sourceAssetIssuer;
    Asset * m_sourceAsset;
public:
    PathPaymentOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~PathPaymentOperationResponse();

    QString getAmount() const;
    QString getSourceAmount() const;
    QString getSourceMax() const;

    KeyPair& getFrom();
    KeyPair& getTo();

    Asset* getAsset();
    Asset* getSourceAsset();
    QString from() const;
    QString to() const;

    QString assetType() const;
    QString assetCode() const;
    QString assetIssuer() const;
    QString sourceAssetType() const;
    QString sourceAssetCode() const;
    QString sourceAssetIssuer() const;

public slots:
    void setFrom(QString from);
    void setTo(QString to);
    void setAssetType(QString assetType);
    void setAssetCode(QString assetCode);
    void setAssetIssuer(QString assetIssuer);
    void setSourceAssetType(QString sourceAssetType);
    void setSourceAssetCode(QString sourceAssetCode);
    void setSourceAssetIssuer(QString sourceAssetIssuer);

};
Q_DECLARE_METATYPE(PathPaymentOperationResponse*)
#endif // PATHPAYMENTOPERATIONRESPONSE_H
