#include "tradeaggregationsrequestbuilder.h"
#include "../server.h"
#include "../asset.h"
#include "../assettypecreditalphanum.h"
#include "../keypair.h"
TradeAggregationsRequestBuilder::TradeAggregationsRequestBuilder(Server *server, Asset *baseAsset, Asset *counterAsset, qint64 startTime, qint64 endTime, qint64 resolution, qint64 offset)
    :RequestBuilder(server,"trade_aggregations")
{
    this->baseAsset(baseAsset);
    this->counterAsset(counterAsset);
    addParameter("start_time", QString::number(startTime));
    addParameter("end_time", QString::number(endTime));
    addParameter("resolution", QString::number(resolution));
    addParameter("offset", QString::number(offset));
}

Page<TradeAggregationResponse> *TradeAggregationsRequestBuilder::execute(QUrl uri)
{
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<Page<TradeAggregationResponse> >(this);
}

Page<TradeAggregationResponse> *TradeAggregationsRequestBuilder::execute() {
    return server()->get<Page<TradeAggregationResponse> >(this);
}


TradeAggregationsRequestBuilder &TradeAggregationsRequestBuilder::cursor(QString token) {
    RequestBuilder::cursor(token);
    return *this;
}

TradeAggregationsRequestBuilder &TradeAggregationsRequestBuilder::limit(int number) {
    RequestBuilder::limit(number);
    return *this;
}

TradeAggregationsRequestBuilder &TradeAggregationsRequestBuilder::order(RequestBuilder::Order direction) {
    RequestBuilder::order(direction);
    return *this;
}

void TradeAggregationsRequestBuilder::baseAsset(Asset *asset) {
    addParameter("base_asset_type", asset->getType());
    if (AssetTypeCreditAlphaNum * creditAlphaNumAsset = dynamic_cast<AssetTypeCreditAlphaNum*>(asset)) {
        addParameter("base_asset_code", creditAlphaNumAsset->getCode());
        addParameter("base_asset_issuer", creditAlphaNumAsset->getIssuer().getAccountId());
    }
}

void TradeAggregationsRequestBuilder::counterAsset(Asset *asset) {
    addParameter("counter_asset_type", asset->getType());
    if (AssetTypeCreditAlphaNum * creditAlphaNumAsset = dynamic_cast<AssetTypeCreditAlphaNum*>(asset)) {
        addParameter("counter_asset_code", creditAlphaNumAsset->getCode());
        addParameter("counter_asset_issuer", creditAlphaNumAsset->getIssuer().getAccountId());
    }
}
