#include "claimablebalancesrequestbuilder.h"
#include "../server.h"
ClaimableBalancesRequestBuilder::ClaimableBalancesRequestBuilder(Server *server)
:RequestBuilder(server, "claimable_balances")
{

}

ClaimableBalanceResponse *ClaimableBalancesRequestBuilder::claimableBalance(QUrl uri)   {
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<ClaimableBalanceResponse >(this);
}

ClaimableBalanceResponse *ClaimableBalancesRequestBuilder::claimableBalance(QString id){
    this->setSegments(QStringList() << "claimable_balances" << id);
    return this->claimableBalance(this->buildUri());
}

ClaimableBalancesRequestBuilder &ClaimableBalancesRequestBuilder::forSponsor(QString sponsor) {
    addParameter("sponsor", sponsor);
    return *this;
}

ClaimableBalancesRequestBuilder &ClaimableBalancesRequestBuilder::forAsset(Asset *asset) {
    RequestBuilder::setAssetParameter("asset", asset);
    return *this;
}

ClaimableBalancesRequestBuilder &ClaimableBalancesRequestBuilder::forClaimant(QString claimant) {
    addParameter("claimant",claimant);
    return *this;
}

Page<ClaimableBalanceResponse> *ClaimableBalancesRequestBuilder::execute(QUrl uri) {
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<Page<ClaimableBalanceResponse> >(this);
}

Page<ClaimableBalanceResponse> *ClaimableBalancesRequestBuilder::execute() {
    return server()->get<Page<ClaimableBalanceResponse> >(this);
}

ClaimableBalancesRequestBuilder &ClaimableBalancesRequestBuilder::cursor(QString token) {
    RequestBuilder::cursor(token);
    return *this;
}

ClaimableBalancesRequestBuilder &ClaimableBalancesRequestBuilder::limit(int number) {
    RequestBuilder::limit(number);
    return *this;
}

ClaimableBalancesRequestBuilder &ClaimableBalancesRequestBuilder::order(RequestBuilder::Order direction) {
    RequestBuilder::order(direction);
    return *this;
}
