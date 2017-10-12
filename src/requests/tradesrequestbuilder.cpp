#include "tradesrequestbuilder.h"
#include "../asset.h"
#include "../assettypecreditalphanum.h"
#include "../keypair.h"
#include "../server.h"
TradesRequestBuilder::TradesRequestBuilder(Server *server):RequestBuilder(server, "order_book/trades")
{

}

TradesRequestBuilder &TradesRequestBuilder::buyingAsset(Asset *asset) {
    checkNotNull(asset, "asset cannot be null");
    addParameter("buying_asset_type",asset->getType());
    if (AssetTypeCreditAlphaNum *creditAlphaNumAsset = dynamic_cast<AssetTypeCreditAlphaNum*>(asset)) {
        addParameter("buying_asset_code", creditAlphaNumAsset->getCode());
        addParameter("buying_asset_issuer", creditAlphaNumAsset->getIssuer().getAccountId());
    }
    return *this;
}

TradesRequestBuilder &TradesRequestBuilder::sellingAsset(Asset *asset) {
    checkNotNull(asset, "asset cannot be null");
    addParameter("selling_asset_type",asset->getType());
    if (AssetTypeCreditAlphaNum *creditAlphaNumAsset = dynamic_cast<AssetTypeCreditAlphaNum*>(asset)) {
        addParameter("selling_asset_code", creditAlphaNumAsset->getCode());
        addParameter("selling_asset_issuer", creditAlphaNumAsset->getIssuer().getAccountId());
    }
    return *this;
}

Page<TradeResponse>* TradesRequestBuilder::execute(QUrl uri)
{
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<Page<TradeResponse> >(this);
}

Page<TradeResponse>* TradesRequestBuilder::execute() {
    return server()->get<Page<TradeResponse> >(this);
}
