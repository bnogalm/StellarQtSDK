#ifndef CREATEPASSIVESELLOFFEROPERATIONRESPONSE_H
#define CREATEPASSIVESELLOFFEROPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
#include "qstellar_namespace.h"
#include "../../price.h"
QSTELLAR_BEGIN_NS

class Asset;/**
 * Represents CreatePassiveSellOfferOperationResponse operation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class CreatePassiveSellOfferOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(qint64 offer_id MEMBER m_offerID)
    Q_PROPERTY(QString amount MEMBER m_amount)
    Q_PROPERTY(QString price MEMBER m_price)
    // price_r: exact rational price {n,d}; `price` above is the rounded decimal.
    Q_PROPERTY(QVariantMap price_r MEMBER m_priceR)

    Q_PROPERTY(QString buying_asset_type READ buyingAssetType WRITE setBuyingAssetType)
    Q_PROPERTY(QString buying_asset_code READ buyingAssetCode WRITE setBuyingAssetCode)
    Q_PROPERTY(QString buying_asset_issuer READ buyingAssetIssuer WRITE setBuyingAssetIssuer)

    Q_PROPERTY(QString selling_asset_type READ sellingAssetType WRITE setSellingAssetType)
    Q_PROPERTY(QString selling_asset_code READ sellingAssetCode WRITE setSellingAssetCode)
    Q_PROPERTY(QString selling_asset_issuer READ sellingAssetIssuer WRITE setSellingAssetIssuer)

    qint64 m_offerID;
    QString m_amount;
    QString m_price;
    QVariantMap m_priceR;
    QString m_buyingAssetType;
    QString m_buyingAssetCode;
    QString m_buyingAssetIssuer;
    QString m_sellingAssetType;
    QString m_sellingAssetCode;
    QString m_sellingAssetIssuer;

    Asset* m_buyingAsset;
    Asset* m_sellingAsset;

public:
    CreatePassiveSellOfferOperationResponse(QNetworkReply *reply=nullptr);
    virtual ~CreatePassiveSellOfferOperationResponse();

    qint64 getOfferId() const;
    QString getAmount() const;
    QString getPrice() const;
    /** Exact price as a rational {n,d}. Prefer over getPrice() for fund-critical math. */
    Price getPriceR() const;
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

QSTELLAR_END_NS

QSTELLAR_ALIAS(CreatePassiveSellOfferOperationResponse)



Q_DECLARE_METATYPE(qstellar::CreatePassiveSellOfferOperationResponse*)
#endif // CREATEPASSIVESELLOFFEROPERATIONRESPONSE_H



