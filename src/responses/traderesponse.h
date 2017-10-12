#ifndef TRADERESPONSE_H
#define TRADERESPONSE_H

#include <QObject>
#include "response.h"
#include "link.h"

namespace TradeResponseAttach
{
/**
 * Links connected to a trade.
 */
class Links {
    Q_GADGET
    Q_PROPERTY(Link self MEMBER m_self)
    Q_PROPERTY(Link seller MEMBER m_seller)
    Q_PROPERTY(Link buyer MEMBER m_buyer)
    Link m_self;
    Link m_seller;
    Link m_buyer;
public:
    Link& getSelf() {
        return m_self;
    }
    Link& getSeller() {
        return m_seller;
    }
    Link& getBuyer() {
        return m_buyer;
    }
    bool operator !=(Links& links)
    {
        Q_UNUSED(links)
        return true;
    }
};
}
class KeyPair;
/**
 * Represents trades response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/endpoints/trades-for-orderbook.html" target="_blank">Trades for Orderbook documentation</a>
 * @see org.stellar.sdk.requests.TradesRequestBuilder
 * @see org.stellar.sdk.Server#trades()
 */

class TradeResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(QString id MEMBER m_id)
    Q_PROPERTY(QString paging_token MEMBER m_pagingToken)
    Q_PROPERTY(QString created_at MEMBER m_createdAt)
    Q_PROPERTY(QString seller READ seller WRITE setSeller)

    Q_PROPERTY(QString sold_amount MEMBER m_soldAmount)
    Q_PROPERTY(QString sold_asset_type MEMBER m_soldAssetType)
    Q_PROPERTY(QString sold_asset_code MEMBER m_soldAssetCode)
    Q_PROPERTY(QString sold_asset_issuer MEMBER m_soldAssetIssuer)

    Q_PROPERTY(QString buyer READ buyer WRITE setBuyer)

    Q_PROPERTY(QString bought_amount MEMBER m_boughtAmount)
    Q_PROPERTY(QString bought_asset_type MEMBER m_boughtAssetType)
    Q_PROPERTY(QString bought_asset_code MEMBER m_boughtAssetCode)
    Q_PROPERTY(QString bought_asset_issuer MEMBER m_boughtAssetIssuer)

    Q_PROPERTY(TradeResponseAttach::Links _links MEMBER m_links)

    QString m_id;
    QString m_pagingToken;
    QString m_createdAt;
    QString m_seller;
    KeyPair *m_sellerKeypair;

    QString m_soldAmount;
    QString m_soldAssetType;
    QString m_soldAssetCode;
    QString m_soldAssetIssuer;

    QString m_buyer;
    KeyPair *m_buyerKeypair;

    QString m_boughtAmount;
    QString m_boughtAssetType;
    QString m_boughtAssetCode;
    QString m_boughtAssetIssuer;
    TradeResponseAttach::Links m_links;

public:
    Q_INVOKABLE explicit TradeResponse(QNetworkReply *reply=nullptr);
    virtual ~TradeResponse();

    QString getId() const;

    QString getPagingToken() const;

    QString getCreatedAt() const;

    KeyPair& getSeller();

    QString getSoldAmount() const;

    QString getSoldAssetType() const;

    QString getSoldAssetCode() const;

    QString getSoldAssetIssuer() const;

    KeyPair& getBuyer();

    QString getBoughtAmount() const;

    QString getBoughtAssetType() const;

    QString getBoughtAssetCode() const;

    QString getBoughtAssetIssuer() const;

    TradeResponseAttach::Links& getLinks();
    QString seller() const;
    QString buyer() const;

public slots:
    void setSeller(QString seller);
    void setBuyer(QString buyer);
};

#endif // TRADERESPONSE_H
Q_DECLARE_METATYPE(TradeResponseAttach::Links)
Q_DECLARE_METATYPE(TradeResponse*)
