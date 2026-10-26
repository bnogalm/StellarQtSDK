#ifndef TRADERESPONSE_H
#define TRADERESPONSE_H

#include <QObject>
#include "response.h"
#include "link.h"
#include "price.h"

#include "../keypair.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

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
    bool operator ==(const Links& links) const
    {
        return m_base == links.m_base
                && m_counter == links.m_counter
                && m_operation == links.m_operation;
    }
    bool operator !=(const Links& links) const
    {
        return !(*this == links);
    }
};
}
class Asset;/**
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

    // Horizon sends n and d as int64 strings; a Price gadget wrapped them
    // mod 2^32 (4294967297/4294967296 read as 1/0).
    Q_PROPERTY(QVariantMap price MEMBER m_priceR)
    Q_PROPERTY(QString trade_type MEMBER m_tradeType)
    Q_PROPERTY(qint32 liquidity_pool_fee_bp MEMBER m_liquidityPoolFeeBp)
    Q_PROPERTY(QString base_liquidity_pool_id MEMBER m_baseLiquidityPoolId)
    Q_PROPERTY(QString counter_liquidity_pool_id MEMBER m_counterLiquidityPoolId)

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
    QVariantMap m_priceR;
    QString m_tradeType;
    qint32 m_liquidityPoolFeeBp;
    QString m_baseLiquidityPoolId;
    QString m_counterLiquidityPoolId;
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

    /**
     * The trade price as an int32 Price: n/d as Horizon sent them when both
     * fit, otherwise reduced by their gcd. Liquidity-pool trades often have
     * int64 terms that do not fit even then, and this throws
     * std::runtime_error for those rather than return a different price:
     * getPriceN()/getPriceD() always hold the exact value.
     */
    Price getPrice() const;
    /** Numerator of the trade price, exactly as Horizon sent it (int64). */
    qint64 getPriceN() const;
    /** Denominator of the trade price, exactly as Horizon sent it (int64). */
    qint64 getPriceD() const;
    QString getTradeType() const;
    /** Liquidity-pool fee in basis points for LP trades (0 for orderbook trades). */
    qint32 getLiquidityPoolFeeBp() const;
    QString getBaseLiquidityPoolId() const;
    QString getCounterLiquidityPoolId() const;
    TradeResponseAttach::Links& getLinks();
    QString seller() const;
    QString buyer() const;

    QString baseAccount() const;

    QString counterAccount() const;

    Asset* getBaseAsset();
    Asset* getCounterAsset();

    /**
     * One int64 term of a Horizon price object ({"n": "...", "d": "..."}),
     * as text (exact over the whole int64 range, as Horizon sends it) or as a
     * JSON number (exact only below 2^53, refused from there). Throws
     * std::runtime_error when it is missing or not a whole number read
     * exactly: never rounded, never wrapped. Shared with
     * TradeAggregationResponse.
     */
    static qint64 priceTerm(const QVariantMap& price, const char* key);
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

QSTELLAR_END_NS

QSTELLAR_ALIAS(TradeResponse)




QSTELLAR_NS_ALIAS(TradeResponseAttach)
Q_DECLARE_METATYPE(qstellar::TradeResponseAttach::Links)
Q_DECLARE_METATYPE(qstellar::TradeResponse*)
#endif // TRADERESPONSE_H

