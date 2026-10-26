#ifndef TRADEAGGREGATIONSREQUESTBUILDER_H
#define TRADEAGGREGATIONSREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/page.h"
#include "../responses/tradeaggregationresponse.h"
#include "../qstellar_namespace.h"
QSTELLAR_FWD(Asset)
QSTELLAR_BEGIN_NS

class TradeAggregationsRequestBuilder : public RequestBuilder
{
public:
    /**
     * Throws std::runtime_error, as js and py refuse them, for a
     * `resolution` (milliseconds) other than 60000, 300000, 900000,
     * 3600000, 86400000 or 604800000, and for an `offset` that is not a
     * whole number of hours (3600000 ms), is above the resolution or is a
     * day or more. A negative offset is refused too, which js and py let
     * through only because -3600000 % 3600000 is 0: Horizon documents the
     * offset as the whole hours, under a day, that the buckets start
     * after. Both used to go out unchecked, for Horizon to answer 400.
     */
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

QSTELLAR_END_NS

QSTELLAR_ALIAS(TradeAggregationsRequestBuilder)

#endif // TRADEAGGREGATIONSREQUESTBUILDER_H
