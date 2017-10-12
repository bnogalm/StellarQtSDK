#ifndef TRADEEFFECTRESPONSE_H
#define TRADEEFFECTRESPONSE_H

#include "effectresponse.h"


class Asset;
class KeyPair;
/**
 * Represents trade effect response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/effect.html" target="_blank">Effect documentation</a>
 * @see org.stellar.sdk.requests.EffectsRequestBuilder
 * @see org.stellar.sdk.Server#effects()
 */
class TradeEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString seller READ seller WRITE setSeller)
    Q_PROPERTY(qint64 offer_id MEMBER m_offerId)
    Q_PROPERTY(QString sold_amount MEMBER m_soldAmount)
    Q_PROPERTY(QString sold_asset_type READ soldAssetType WRITE setSoldAssetType)
    Q_PROPERTY(QString sold_asset_code READ soldAssetCode WRITE setSoldAssetCode)
    Q_PROPERTY(QString sold_asset_issuer READ soldAssetIssuer WRITE setSoldAssetIssuer)
    Q_PROPERTY(QString bought_amount MEMBER m_boughtAmount)

    Q_PROPERTY(QString bought_asset_type READ boughtAssetType WRITE setBoughtAssetType)
    Q_PROPERTY(QString bought_asset_code READ boughtAssetCode WRITE setBoughtAssetCode)
    Q_PROPERTY(QString bought_asset_issuer READ boughtAssetIssuer WRITE setBoughtAssetIssuer)

    QString m_seller;
    KeyPair* m_sellerKeypair;
    qint64 m_offerId;

    QString m_soldAmount;
    QString m_soldAssetType;
    QString m_soldAssetCode;
    QString m_soldAssetIssuer;
    Asset* m_soldAsset;

    QString m_boughtAmount;
    QString m_boughtAssetType;
    QString m_boughtAssetCode;
    QString m_boughtAssetIssuer;
    Asset* m_boughtAsset;


public:
    TradeEffectResponse(QNetworkReply *reply = nullptr);
    virtual ~TradeEffectResponse();


    KeyPair& getSeller();
    qint64 getOfferId() const;
    QString getSoldAmount() const;
    QString getBoughtAmount() const;
    Asset* getSoldAsset();
    Asset* getBoughtAsset();

    QString seller() const;
    QString soldAssetCode() const;
    QString soldAssetIssuer() const;
    QString boughtAssetType() const;
    QString boughtAssetCode() const;
    QString boughtAssetIssuer() const;
    QString soldAssetType() const;

public slots:
    void setSeller(QString seller);

    void setSoldAssetCode(QString soldAssetCode);
    void setSoldAssetIssuer(QString soldAssetIssuer);
    void setSoldAssetType(QString soldAssetType);

    void setBoughtAssetType(QString boughtAssetType);
    void setBoughtAssetCode(QString boughtAssetCode);
    void setBoughtAssetIssuer(QString boughtAssetIssuer);

};
Q_DECLARE_METATYPE(TradeEffectResponse*)
#endif // TRADEEFFECTRESPONSE_H



