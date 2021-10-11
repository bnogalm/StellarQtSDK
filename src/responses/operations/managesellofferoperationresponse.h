#ifndef MANAGESELLOFFEROPERATIONRESPONSE_H
#define MANAGESELLOFFEROPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"

class Asset;
/**
 * Represents ManageSellOfferOperationResponse operation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class ManageSellOfferOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(qint64 offer_id MEMBER m_offerID)
    Q_PROPERTY(QString amount MEMBER m_amount)
    // Price is not implemented yet in horizon
    Q_PROPERTY(QString price MEMBER m_price)

    Q_PROPERTY(QString buying_asset_type READ buyingAssetType WRITE setBuyingAssetType)
    Q_PROPERTY(QString buying_asset_code READ buyingAssetCode WRITE setBuyingAssetCode)
    Q_PROPERTY(QString buying_asset_issuer READ buyingAssetIssuer WRITE setBuyingAssetIssuer)

    Q_PROPERTY(QString selling_asset_type READ sellingAssetType WRITE setSellingAssetType)
    Q_PROPERTY(QString selling_asset_code READ sellingAssetCode WRITE setSellingAssetCode)
    Q_PROPERTY(QString selling_asset_issuer READ sellingAssetIssuer WRITE setSellingAssetIssuer)

    qint64 m_offerID;
    QString m_amount;
    QString m_price;

    QString m_buyingAssetType;
    QString m_buyingAssetCode;
    QString m_buyingAssetIssuer;
    Asset * m_buyingAsset;

    QString m_sellingAssetType;
    QString m_sellingAssetCode;
    QString m_sellingAssetIssuer;
    Asset * m_sellingAsset;


public:
    ManageSellOfferOperationResponse(QNetworkReply *reply = nullptr);
    virtual ~ManageSellOfferOperationResponse();
    qint64 getOfferId() const;
    QString getAmount() const;
    QString getPrice() const;
    Asset* getBuyingAsset();
    Asset* getSellingAsset();
    QString buyingAssetType() const;
    QString buyingAssetCode() const;
    QString buyingAssetIssuer() const;
    QString sellingAssetType() const;
    QString sellingAssetCode() const;
    QString sellingAssetIssuer() const;

public slots:
    void setBuyingAssetType(QString buyingAssetType);
    void setBuyingAssetCode(QString buyingAssetCode);
    void setBuyingAssetIssuer(QString buyingAssetIssuer);
    void setSellingAssetType(QString sellingAssetType);
    void setSellingAssetCode(QString sellingAssetCode);
    void setSellingAssetIssuer(QString sellingAssetIssuer);
};
Q_DECLARE_METATYPE(ManageSellOfferOperationResponse*)
#endif // MANAGESELLOFFEROPERATIONRESPONSE_H
