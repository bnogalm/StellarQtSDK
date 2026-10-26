#include "tradeaggregationsrequestbuilder.h"
#include "../server.h"
#include "../asset.h"
#include "../assettypecreditalphanum.h"
#include "../keypair.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

QSTELLAR_BEGIN_NS

namespace {

/** The resolutions Horizon aggregates by, in ms (js allowedResolutions, py's list). */
const qint64 ALLOWED_RESOLUTIONS[] = { 60000, 300000, 900000, 3600000, 86400000, 604800000 };
const qint64 HOUR_MS = 3600000;

} // namespace

TradeAggregationsRequestBuilder::TradeAggregationsRequestBuilder(Server *server, Asset *baseAsset, Asset *counterAsset, qint64 startTime, qint64 endTime, qint64 resolution, qint64 offset)
    :RequestBuilder(server,"trade_aggregations")
{
    if (std::find(std::begin(ALLOWED_RESOLUTIONS), std::end(ALLOWED_RESOLUTIONS), resolution)
            == std::end(ALLOWED_RESOLUTIONS))
        throw std::runtime_error(QStringLiteral("invalid resolution: %1 ms (one of 60000, 300000, 900000, "
                                                "3600000, 86400000, 604800000)").arg(resolution).toStdString());
    if (offset < 0 || offset > resolution || offset >= 24 * HOUR_MS || offset % HOUR_MS != 0)
        throw std::runtime_error(QStringLiteral("invalid offset: %1 ms (whole hours, no more than the "
                                                "resolution, under 24 hours)").arg(offset).toStdString());
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
QSTELLAR_END_NS
