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

     OrderBookResponse* execute(QUrl uri);

     OrderBookResponse* execute();

     //marked as unimplemented in java sdk, no tested
     RequestBuilder& cursor(QString cursor);
    //marked as unimplemented in java sdk, no tested
     RequestBuilder& limit(int number);
    //marked as unimplemented in java sdk, no tested
     RequestBuilder& order(RequestBuilder::Order direction);
};

#endif // ORDERBOOKREQUESTBUILDER_H
