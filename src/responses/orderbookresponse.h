#ifndef ORDERBOOKRESPONSE_H
#define ORDERBOOKRESPONSE_H

#include <QObject>
#include "response.h"
#include "../price.h"

namespace OrderBookResponseAttach
{
/**
  * Represents order book row.
  */
class Row {
    Q_GADGET
    Q_PROPERTY(QString amount MEMBER m_amount)
    Q_PROPERTY(QString price MEMBER m_price)
    Q_PROPERTY(QVariantMap price_r MEMBER m_priceR)

    QString m_amount;
    QString m_price;
    QVariantMap m_priceR;
public:
    QString getAmount();
    QString getPrice();
    Price getPriceR();
    bool operator !=(const Row& row) const;
    bool operator ==(const Row& row) const;
};
}
class Asset;
/**
 * Represents order book response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/orderbook.html" target="_blank">Order book documentation</a>
 * @see org.stellar.sdk.requests.OrderBookRequestBuilder
 * @see org.stellar.sdk.Server#orderBook()
 */
class OrderBookResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap base READ base WRITE setBase)
    Q_PROPERTY(QVariantMap counter READ counter WRITE setCounter)
    Q_PROPERTY(QList<OrderBookResponseAttach::Row> asks MEMBER m_asks)
    Q_PROPERTY(QList<OrderBookResponseAttach::Row> bids MEMBER m_bids)

    QVariantMap m_base;
    QVariantMap m_counter;
    Asset* m_baseAsset;
    Asset* m_counterAsset;
    QList<OrderBookResponseAttach::Row> m_asks;
    QList<OrderBookResponseAttach::Row> m_bids;
public:
    Q_INVOKABLE explicit OrderBookResponse(QNetworkReply* reply=nullptr);
    virtual ~OrderBookResponse();
    Asset* getBase();
    Asset* getCounter();
    QList<OrderBookResponseAttach::Row> getAsks();
    QList<OrderBookResponseAttach::Row> getBids();

    QVariantMap base() const;
    QVariantMap counter() const;

public slots:
    void setBase(QVariantMap base);
    void setCounter(QVariantMap counter);
};
Q_DECLARE_METATYPE(OrderBookResponseAttach::Row)


#endif // ORDERBOOKRESPONSE_H
