#ifndef TRADESREQUESTBUILDER_H
#define TRADESREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/traderesponse.h"
#include "../responses/page.h"
#include "../qstellar_namespace.h"
QSTELLAR_FWD(Asset)
QSTELLAR_BEGIN_NS

/**
 * Builds requests connected to trades.
 */
class TradesRequestBuilder : public RequestBuilder
{
public:
    TradesRequestBuilder(Server* server);

    TradesRequestBuilder& baseAsset(Asset* asset);

    TradesRequestBuilder& counterAsset(Asset* asset);
    TradesRequestBuilder& offerId(QString offerId);

    /**
    * Builds request to <code>GET /accounts/{account}/trades</code>
    * @see <a href="https://www.stellar.org/developers/horizon/reference/endpoints/trades-for-account.html">Trades for Account</a>
    * @param account Account for which to get trades
    */
    TradesRequestBuilder& forAccount(KeyPair *account);
    /** String overload for forAccount. */
    TradesRequestBuilder& forAccount(const QString& accountId);

    /**
     * Builds request to <code>GET /liquidity_pools/{pool_id}/trades</code>.
     */
    TradesRequestBuilder& forLiquidityPool(const QString& liquidityPoolId);

    /**
     * Adds a `trade_type` filter (e.g. "orderbook" / "liquidity_pool" / "all").
     */
    TradesRequestBuilder& tradeType(const QString& type);

    /**
     * Requests specific <code>uri</code> and returns {@link TradeResponse}.
     * This method is helpful for getting the links.
     */
    Page<TradeResponse>* execute(QUrl uri);

    Page<TradeResponse>* execute();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TradesRequestBuilder)

#endif // TRADESREQUESTBUILDER_H
