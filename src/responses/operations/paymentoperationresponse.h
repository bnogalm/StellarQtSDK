#ifndef PAYMENTOPERATIONRESPONSE_H
#define PAYMENTOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
class Asset;
/**
 * Represents Payment operation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class PaymentOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString amount MEMBER m_amount)
    Q_PROPERTY(QString asset_type READ assetType WRITE setAssetType)
    Q_PROPERTY(QString asset_code READ assetCode WRITE setAssetCode)
    Q_PROPERTY(QString asset_issuer READ assetIssuer WRITE setAssetIssuer)
    Q_PROPERTY(QString from READ from WRITE setFrom)
    Q_PROPERTY(QString to READ to WRITE setTo)

    QString m_amount;
    QString m_assetType;
    QString m_assetCode;
    QString m_assetIssuer;
    QString m_from;
    QString m_to;
    KeyPair* m_fromKeypair;
    KeyPair* m_toKeypair;
    Asset * m_asset;

public:
    PaymentOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~PaymentOperationResponse();
    QString getAmount();

    QString assetType();
    QString assetCode();
    Asset* getAsset();

    KeyPair& getFrom();

    KeyPair& getTo();
    QString from() const;
    QString to() const;
    QString assetIssuer() const;

public slots:
    void setFrom(QString from);
    void setTo(QString to);
    void setAssetType(QString assetType);
    void setAssetCode(QString assetCode);
    void setAssetIssuer(QString assetIssuer);
};

Q_DECLARE_METATYPE(PaymentOperationResponse*)
#endif // PAYMENTOPERATIONRESPONSE_H

