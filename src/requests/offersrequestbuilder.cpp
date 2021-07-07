#include "offersrequestbuilder.h"
#include "../keypair.h"
#include "../server.h"
#include "../assettypecreditalphanum.h"
OffersRequestBuilder::OffersRequestBuilder(Server *server):RequestBuilder(server, "offers")
{

}

OffersRequestBuilder &OffersRequestBuilder::forAccount(KeyPair *account) {
    account = checkNotNull(account, "account cannot be null");

    this->setSegments(QStringList()<<"accounts"<< account->getAccountId()<< "offers");
    return *this;
}

OffersRequestBuilder &OffersRequestBuilder::forSponsor(QString sponsor) {
    RequestBuilder::addParameter("sponsor",sponsor);
    return *this;
}

OffersRequestBuilder &OffersRequestBuilder::forSponsor(KeyPair *sponsor) {
    return forSponsor(sponsor->getAccountId());
}

Page<OfferResponse>* OffersRequestBuilder::execute(QUrl uri){
    this->setRequestUri(uri);
    return server()->get<Page<OfferResponse> >(this);
}

Page<OfferResponse>* OffersRequestBuilder::execute(){
    return execute(this->buildUri());
}

OffersRequestBuilder &OffersRequestBuilder::cursor(QString token) {
    RequestBuilder::cursor(token);
    return *this;
}

OffersRequestBuilder &OffersRequestBuilder::limit(int number) {
    RequestBuilder::limit(number);
    return *this;
}

OffersRequestBuilder &OffersRequestBuilder::order(RequestBuilder::Order direction) {
    RequestBuilder::order(direction);
    return *this;
}

OffersRequestBuilder &OffersRequestBuilder::seller(KeyPair *accountId)
{
    RequestBuilder::addParameter("seller",accountId->getAccountId());
    return *this;
}

OffersRequestBuilder &OffersRequestBuilder::selling(Asset *asset)
{
    RequestBuilder::addParameter("selling_asset_type",asset->getType());
    if (AssetTypeCreditAlphaNum * creditAsset = dynamic_cast<AssetTypeCreditAlphaNum*>(asset)) {
        RequestBuilder::addParameter("selling_asset_code",creditAsset->getCode());
        RequestBuilder::addParameter("selling_asset_issuer",creditAsset->getIssuer().getAccountId());
    }
    return *this;
}

OffersRequestBuilder &OffersRequestBuilder::buying(Asset *asset)
{
    RequestBuilder::addParameter("buying_asset_type",asset->getType());
    if (AssetTypeCreditAlphaNum * creditAsset = dynamic_cast<AssetTypeCreditAlphaNum*>(asset)) {
        RequestBuilder::addParameter("buying_asset_code",creditAsset->getCode());
        RequestBuilder::addParameter("buying_asset_issuer",creditAsset->getIssuer().getAccountId());
    }
    return *this;
}

OffersRequestBuilder &OffersRequestBuilder::stream() {
    setStreamMode();
    return *this;
}
