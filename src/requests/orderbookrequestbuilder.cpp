#include "orderbookrequestbuilder.h"
#include "../keypair.h"
#include "../asset.h"
#include "../assettypecreditalphanum.h"
#include "../server.h"
OrderBookRequestBuilder::OrderBookRequestBuilder(Server *server):RequestBuilder(server, "order_book")
{

}

OrderBookRequestBuilder &OrderBookRequestBuilder::buyingAsset(Asset *asset) {
    addParameter("buying_asset_type", asset->getType());
    if (AssetTypeCreditAlphaNum *creditAlphaNumAsset = dynamic_cast<AssetTypeCreditAlphaNum*>(asset) ){
        addParameter("buying_asset_code", creditAlphaNumAsset->getCode());
        addParameter("buying_asset_issuer", creditAlphaNumAsset->getIssuer().getAccountId());
    }
    return *this;
}

OrderBookRequestBuilder &OrderBookRequestBuilder::sellingAsset(Asset *asset) {
    addParameter("selling_asset_type", asset->getType());
    if (AssetTypeCreditAlphaNum *creditAlphaNumAsset = dynamic_cast<AssetTypeCreditAlphaNum*>(asset) ){
        addParameter("selling_asset_code", creditAlphaNumAsset->getCode());
        addParameter("selling_asset_issuer", creditAlphaNumAsset->getIssuer().getAccountId());
    }
    return *this;
}

OrderBookRequestBuilder &OrderBookRequestBuilder::stream()
{
    setStreamMode();
    return *this;
}

OrderBookResponse *OrderBookRequestBuilder::execute(QUrl uri){
    this->setRequestUri(uri);
    return server()->get<OrderBookResponse>(this);
}

OrderBookResponse *OrderBookRequestBuilder::execute() {
    return execute(this->buildUri());
}

RequestBuilder& OrderBookRequestBuilder::order(RequestBuilder::Order direction) {
    RequestBuilder::order(direction);
    return *this;
}

RequestBuilder& OrderBookRequestBuilder::limit(int number) {
    RequestBuilder::limit(number);
    return *this;
}

RequestBuilder& OrderBookRequestBuilder::cursor(QString cursor) {
    RequestBuilder::cursor(cursor);
    return *this;
}
