#ifndef PAYMENTOPERATIONRESPONSE_H
#define PAYMENTOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

class Asset;/**
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
    Q_PROPERTY(QString from_muxed MEMBER m_fromMuxed)
    Q_PROPERTY(QString from_muxed_id MEMBER m_fromMuxedId)
    Q_PROPERTY(QString to READ to WRITE setTo)
    Q_PROPERTY(QString to_muxed MEMBER m_toMuxed)
    Q_PROPERTY(QString to_muxed_id MEMBER m_toMuxedId)

    QString m_amount;
    QString m_assetType;
    QString m_assetCode;
    QString m_assetIssuer;
    QString m_from;
    QString m_to;
    QString m_fromMuxed;
    QString m_fromMuxedId;
    QString m_toMuxed;
    QString m_toMuxedId;
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
    /** M-strkey of `from`/`to` if muxed; empty otherwise. */
    QString getFromMuxed() const { return m_fromMuxed; }
    QString getFromMuxedId() const { return m_fromMuxedId; }
    QString getToMuxed() const { return m_toMuxed; }
    QString getToMuxedId() const { return m_toMuxedId; }

public slots:
    void setFrom(QString from);
    void setTo(QString to);
    void setAssetType(QString assetType);
    void setAssetCode(QString assetCode);
    void setAssetIssuer(QString assetIssuer);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(PaymentOperationResponse)




Q_DECLARE_METATYPE(qstellar::PaymentOperationResponse*)
#endif // PAYMENTOPERATIONRESPONSE_H

