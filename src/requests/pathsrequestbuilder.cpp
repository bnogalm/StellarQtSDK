#include "pathsrequestbuilder.h"
#include "../keypair.h"
#include "../asset.h"
#include "../assettypecreditalphanum.h"
#include "../server.h"

PathsRequestBuilder::PathsRequestBuilder(Server *server):RequestBuilder(server, "paths")
{

}

PathsRequestBuilder &PathsRequestBuilder::destinationAccount(KeyPair *account) {
    addParameter("destination_account",account->getAccountId());
    return *this;
}

PathsRequestBuilder &PathsRequestBuilder::sourceAccount(KeyPair *account) {
    addParameter("source_account", account->getAccountId());
    return *this;
}

PathsRequestBuilder &PathsRequestBuilder::destinationAmount(QString amount) {
    addParameter("destination_amount", amount);
    return *this;
}

PathsRequestBuilder &PathsRequestBuilder::destinationAsset(Asset *asset) {
    addParameter("destination_asset_type", asset->getType());
    if (AssetTypeCreditAlphaNum * creditAlphaNumAsset =dynamic_cast<AssetTypeCreditAlphaNum*>(asset)) {
        addParameter("destination_asset_code", creditAlphaNumAsset->getCode());
        addParameter("destination_asset_issuer", creditAlphaNumAsset->getIssuer().getAccountId());
    }
    return *this;
}

Page<PathResponse>* PathsRequestBuilder::execute(QUrl uri) {
    this->setRequestUri(uri);
    return server()->get<Page<PathResponse> >(this);
}

Page<PathResponse>* PathsRequestBuilder::execute() {
    return execute(this->buildUri());
}
