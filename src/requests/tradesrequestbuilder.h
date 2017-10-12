#ifndef TRADESREQUESTBUILDER_H
#define TRADESREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/traderesponse.h"
#include "../responses/page.h"
class Asset;
/**
 * Builds requests connected to trades.
 */
class TradesRequestBuilder : public RequestBuilder
{
public:
    TradesRequestBuilder(Server* server);


    TradesRequestBuilder& buyingAsset(Asset* asset);

    TradesRequestBuilder& sellingAsset(Asset* asset);
    /**
     * Requests specific <code>uri</code> and returns {@link TradeResponse}.
     * This method is helpful for getting the links.
     */
    Page<TradeResponse>* execute(QUrl uri);

    Page<TradeResponse>* execute();
};

#endif // TRADESREQUESTBUILDER_H
