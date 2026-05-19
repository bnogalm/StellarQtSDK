#ifndef ORDERBOOKREQUESTBUILDER_H
#define ORDERBOOKREQUESTBUILDER_H
#include "requestbuilder.h"
#include "../responses/orderbookresponse.h"
#include "../responses/page.h"
class Asset;
/**
 * Builds requests connected to order book.
 */
class OrderBookRequestBuilder : public RequestBuilder
{
public:
    OrderBookRequestBuilder(Server* server);


     OrderBookRequestBuilder& buyingAsset(Asset* asset);

     OrderBookRequestBuilder& sellingAsset(Asset* asset);

     /**
           * Allows to stream SSE events from horizon.
           * Certain endpoints in Horizon can be called in streaming mode using Server-Sent Events.
           * This mode will keep the connection to horizon open and horizon will continue to return
           * responses as ledgers close.
           */
     OrderBookRequestBuilder &stream();

     OrderBookResponse* execute(QUrl uri);

     OrderBookResponse* execute();

     // Horizon /order_book accepts cursor, limit and order as standard
     // pagination/ordering params (see
     // https://developers.stellar.org/docs/data/apis/horizon/api-reference/retrieve-an-order-book).
     // Implemented natively (forward to the base RequestBuilder).
     OrderBookRequestBuilder& cursor(QString cursor);
     OrderBookRequestBuilder& limit(int number);
     OrderBookRequestBuilder& order(RequestBuilder::Order direction);
};

#endif // ORDERBOOKREQUESTBUILDER_H
