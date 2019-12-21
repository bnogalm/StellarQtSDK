#include "strictreceivepathsrequestbuilder.h"
#include "../keypair.h"
#include "../asset.h"
#include "../assettypecreditalphanum.h"
#include "../server.h"

StrictReceivePathsRequestBuilder::StrictReceivePathsRequestBuilder(Server *server):RequestBuilder(server, "paths/strict-receive")
{   
}

StrictReceivePathsRequestBuilder &StrictReceivePathsRequestBuilder::destinationAccount(const KeyPair &account) {
    addParameter("destination_account",account.getAccountId());
    return *this;
}

StrictReceivePathsRequestBuilder &StrictReceivePathsRequestBuilder::sourceAccount(const KeyPair &account) {
    if (!parameter("source_assets").isNull()) {
        throw std::runtime_error("cannot set both source_assets and source_account");
    }
    addParameter("source_account", account.getAccountId());
    return *this;
}

StrictReceivePathsRequestBuilder &StrictReceivePathsRequestBuilder::sourceAssets(const QList<Asset *> assets) {
   if (!parameter("source_account").isNull()) {
     throw std::runtime_error("cannot set both source_assets and source_account");
   }
   setAssetsParameter("source_assets", assets);
   return *this;
 }

StrictReceivePathsRequestBuilder &StrictReceivePathsRequestBuilder::destinationAmount(QString amount) {
    addParameter("destination_amount", amount);
    return *this;
}

StrictReceivePathsRequestBuilder &StrictReceivePathsRequestBuilder::destinationAsset(Asset *asset) {
    addParameter("destination_asset_type", asset->getType());
    if (AssetTypeCreditAlphaNum * creditAlphaNumAsset =dynamic_cast<AssetTypeCreditAlphaNum*>(asset)) {
        addParameter("destination_asset_code", creditAlphaNumAsset->getCode());
        addParameter("destination_asset_issuer", creditAlphaNumAsset->getIssuer().getAccountId());
    }
    return *this;
}

Page<PathResponse>* StrictReceivePathsRequestBuilder::execute(QUrl uri) {
    this->setRequestUri(uri);
    return server()->get<Page<PathResponse> >(this);
}

Page<PathResponse>* StrictReceivePathsRequestBuilder::execute() {
    return execute(this->buildUri());
}
