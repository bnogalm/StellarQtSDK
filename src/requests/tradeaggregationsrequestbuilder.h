#ifndef TRADEAGGREGATIONSREQUESTBUILDER_H
#define TRADEAGGREGATIONSREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/page.h"
#include "../responses/tradeaggregationresponse.h"
class Asset;
class TradeAggregationsRequestBuilder : public RequestBuilder
{
public:
    TradeAggregationsRequestBuilder(Server* server , Asset* baseAsset, Asset* counterAsset, qint64 startTime, qint64 endTime, qint64 resolution, qint64 offset);



    Page<TradeAggregationResponse>* execute(QUrl uri);

    Page<TradeAggregationResponse>* execute();
    TradeAggregationsRequestBuilder& cursor(QString token);
    TradeAggregationsRequestBuilder& limit(int number);
    TradeAggregationsRequestBuilder& order(Order direction);
private:
    void baseAsset(Asset* asset);
    void counterAsset(Asset* asset);
};

#endif // TRADEAGGREGATIONSREQUESTBUILDER_H
