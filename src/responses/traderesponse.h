#ifndef TRADERESPONSE_H
#define TRADERESPONSE_H

#include <QObject>
#include "response.h"
#include "link.h"
#include "price.h"

namespace TradeResponseAttach
{
/**
 * Links connected to a trade.
 */
class Links {
    Q_GADGET
    Q_PROPERTY(Link base MEMBER m_base)
    Q_PROPERTY(Link counter MEMBER m_counter)
    Q_PROPERTY(Link operation MEMBER m_operation)
    Link m_base;
    Link m_counter;
    Link m_operation;
public:
    Link& getBase();
    Link& getCounter();
    Link& getOperation();
    bool operator !=(Links& links)
    {
        Q_UNUSED(links)
        return true;
    }
};
}
class KeyPair;
class Asset;
/**
 * Represents trades response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/endpoints/trades.html" target="_blank">Trades documentation</a>
 * @see org.stellar.sdk.requests.TradesRequestBuilder
 * @see org.stellar.sdk.Server#trades()
 */

class TradeResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(QString id MEMBER m_id)
    Q_PROPERTY(QString paging_token MEMBER m_pagingToken)
    Q_PROPERTY(QString ledger_close_time MEMBER m_ledgerCloseTime READ getLedgerCloseTime)
    Q_PROPERTY(QString offer_id MEMBER m_offerId READ getOfferId)
    Q_PROPERTY(bool base_is_seller MEMBER m_baseIsSeller READ isBaseSeller)


    Q_PROPERTY(QString base_account READ baseAccount WRITE setBaseAccount)
    Q_PROPERTY(QString base_offer_id MEMBER m_baseOfferId)

    Q_PROPERTY(QString base_amount MEMBER m_baseAmount)
    Q_PROPERTY(QString base_asset_type MEMBER m_baseAssetType WRITE setBaseAssetType)
    Q_PROPERTY(QString base_asset_code MEMBER m_baseAssetCode WRITE setBaseAssetCode)
    Q_PROPERTY(QString base_asset_issuer MEMBER m_baseAssetIssuer WRITE setBaseAssetIssuer)

    Q_PROPERTY(QString counter_account READ counterAccount WRITE setCounterAccount)
    Q_PROPERTY(QString counter_offer_id MEMBER m_counterOfferId)

    Q_PROPERTY(QString counter_amount MEMBER m_counterAmount)
    Q_PROPERTY(QString counter_asset_type MEMBER m_counterAssetType WRITE setCounterAssetType)
    Q_PROPERTY(QString counter_asset_code MEMBER m_counterAssetCode WRITE setCounterAssetCode)
    Q_PROPERTY(QString counter_asset_issuer MEMBER m_counterAssetIssuer WRITE setCounterAssetIssuer)

    Q_PROPERTY(Price price MEMBER m_price)

    Q_PROPERTY(TradeResponseAttach::Links _links MEMBER m_links)

    QString m_id;
    QString m_pagingToken;
    QString m_ledgerCloseTime;
    QString m_offerId;
    bool m_baseIsSeller;



    QString m_baseAccount;
    QString m_baseOfferId;
    KeyPair *m_baseAccountKeypair;

    QString m_baseAmount;
    QString m_baseAssetType;
    QString m_baseAssetCode;
    QString m_baseAssetIssuer;

    QString m_counterAccount;
    QString m_counterOfferId;
    KeyPair *m_counterAccountKeypair;

    QString m_counterAmount;
    QString m_counterAssetType;
    QString m_counterAssetCode;
    QString m_counterAssetIssuer;
    Price m_price;
    TradeResponseAttach::Links m_links;
    Asset *m_baseAsset;
    Asset *m_counterAsset;

public:
    Q_INVOKABLE explicit TradeResponse(QNetworkReply *reply=nullptr);
    virtual ~TradeResponse();

    QString getId() const;

    QString getPagingToken() const;

    QString getLedgerCloseTime() const;

    QString getOfferId() const;

    bool isBaseSeller() const;

    QString getBaseOfferId() const;

    KeyPair& getBaseAccount();

    QString getBaseAmount() const;

    QString getBaseAssetType() const;

    QString getBaseAssetCode() const;

    QString getBaseAssetIssuer() const;

    QString getCounterOfferId() const;

    KeyPair& getCounterAccount();

    QString getCounterAmount() const;

    QString getCounterAssetType() const;

    QString getCounterAssetCode() const;

    QString getCounterAssetIssuer() const;

    Price getPrice() const;
    TradeResponseAttach::Links& getLinks();
    QString seller() const;
    QString buyer() const;

    QString baseAccount() const;

    QString counterAccount() const;

    Asset* getBaseAsset();
    Asset* getCounterAsset();
public slots:
    void setBaseAccount(QString base_account);
    void setCounterAccount(QString counter_account);
    void setBaseAssetType(QString base_asset_type);
    void setBaseAssetCode(QString base_asset_code);
    void setBaseAssetIssuer(QString base_asset_issuer);
    void setCounterAssetType(QString counter_asset_type);
    void setCounterAssetCode(QString counter_asset_code);
    void setCounterAssetIssuer(QString counter_asset_issuer);
};

Q_DECLARE_METATYPE(TradeResponseAttach::Links)
Q_DECLARE_METATYPE(TradeResponse*)
#endif // TRADERESPONSE_H

