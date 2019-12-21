#include "strictsendpathsrequestbuilder.h"

#include "../keypair.h"
#include "../asset.h"
#include "../assettypecreditalphanum.h"
#include "../server.h"

StrictSendPathsRequestBuilder::StrictSendPathsRequestBuilder(Server *server):RequestBuilder(server,  "paths/strict-send")
{
}

StrictSendPathsRequestBuilder &StrictSendPathsRequestBuilder::destinationAccount(const KeyPair& account) {
    if (!parameter("destination_assets").isNull()) {
        throw std::runtime_error("cannot set both destination_assets and destination_account");
    }
    addParameter("destination_account",account.getAccountId());
    return *this;
}

StrictSendPathsRequestBuilder &StrictSendPathsRequestBuilder::destinationAssets(QList<Asset*> assets) {
   if (!parameter("destination_account").isNull()) {
        throw std::runtime_error("cannot set both destination_assets and destination_account");
   }
   setAssetsParameter("destination_assets", assets);
   return *this;
 }

StrictSendPathsRequestBuilder &StrictSendPathsRequestBuilder::sourceAmount(QString amount) {
    addParameter("source_amount", amount);
    return *this;
}

StrictSendPathsRequestBuilder &StrictSendPathsRequestBuilder::sourceAsset(Asset *asset) {
    addParameter("source_asset_type", asset->getType());
    if (AssetTypeCreditAlphaNum * creditAlphaNumAsset =dynamic_cast<AssetTypeCreditAlphaNum*>(asset)) {
        addParameter("source_asset_code", creditAlphaNumAsset->getCode());
        addParameter("source_asset_issuer", creditAlphaNumAsset->getIssuer().getAccountId());
    }
    return *this;
}

Page<PathResponse>* StrictSendPathsRequestBuilder::execute(QUrl uri) {
    this->setRequestUri(uri);
    return server()->get<Page<PathResponse> >(this);
}

Page<PathResponse>* StrictSendPathsRequestBuilder::execute() {
    return execute(this->buildUri());
}
