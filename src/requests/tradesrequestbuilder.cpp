#include "tradesrequestbuilder.h"
#include "../asset.h"
#include "../assettypecreditalphanum.h"
#include "../keypair.h"
#include "../server.h"
TradesRequestBuilder::TradesRequestBuilder(Server *server):RequestBuilder(server, "trades")
{

}

TradesRequestBuilder &TradesRequestBuilder::baseAsset(Asset *asset) {
    checkNotNull(asset, "asset cannot be null");
    addParameter("base_asset_type",asset->getType());
    if (AssetTypeCreditAlphaNum *creditAlphaNumAsset = dynamic_cast<AssetTypeCreditAlphaNum*>(asset)) {
        addParameter("base_asset_code", creditAlphaNumAsset->getCode());
        addParameter("base_asset_issuer", creditAlphaNumAsset->getIssuer().getAccountId());
    }
    return *this;
}

TradesRequestBuilder &TradesRequestBuilder::counterAsset(Asset *asset) {
    checkNotNull(asset, "asset cannot be null");
    addParameter("counter_asset_type",asset->getType());
    if (AssetTypeCreditAlphaNum *creditAlphaNumAsset = dynamic_cast<AssetTypeCreditAlphaNum*>(asset)) {
        addParameter("counter_asset_code", creditAlphaNumAsset->getCode());
        addParameter("counter_asset_issuer", creditAlphaNumAsset->getIssuer().getAccountId());
    }
    return *this;
}

TradesRequestBuilder &TradesRequestBuilder::offerId(QString offerId) {
    addParameter("offer_id", offerId);
    return *this;
}

TradesRequestBuilder &TradesRequestBuilder::forAccount(KeyPair* account) {
    checkNotNull(account, "account cannot be null");
    setSegments(QStringList() <<"accounts"<< account->getAccountId()<< "trades");
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
