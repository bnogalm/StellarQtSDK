#include "accountsrequestbuilder.h"
#include "../keypair.h"
#include "../server.h"

AccountsRequestBuilder::AccountsRequestBuilder(Server *server):RequestBuilder(server, "accounts") {

}

AccountResponse *AccountsRequestBuilder::account(QUrl uri)  {
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<AccountResponse>(this);
}

AccountResponse* AccountsRequestBuilder::account(QString accountID)
{
    this->setSegments(QStringList()<< "accounts"<< accountID);
    return this->account(this->buildUri());
}

AccountResponse* AccountsRequestBuilder::account(KeyPair *account) {    
    return this->account(account->getAccountId());
}

AccountsRequestBuilder &AccountsRequestBuilder::forSigner(QString accountID)
{
    if (!parameter("asset").isNull()) {
        throw std::runtime_error("cannot set both signer and asset");
    }
    if (!parameter("sponsor").isNull()) {
        throw std::runtime_error("cannot set both signer and sponsor");
    }
    addParameter("signer",accountID);
    return *this;
}

AccountsRequestBuilder &AccountsRequestBuilder::forSigner(KeyPair *accountID)
{    
    return forSigner(accountID->getAccountId());
}

AccountsRequestBuilder &AccountsRequestBuilder::forSponsor(QString sponsor) {
    if (!parameter("signer").isNull()) {
        throw std::runtime_error("cannot set both signer and sponsor");
    }
    if (!parameter("asset").isNull()) {
        throw std::runtime_error("cannot set both asset and sponsor");
    }
    addParameter("sponsor", sponsor);
    return *this;
}

AccountsRequestBuilder &AccountsRequestBuilder::forSponsor(KeyPair* sponsor) {
    return forSponsor(sponsor->getAccountId());
}

AccountsRequestBuilder &AccountsRequestBuilder::forAsset(Asset *asset)
{
    if (!parameter("signer").isNull()) {
        throw std::runtime_error("cannot set both signer and asset");
    }
    if (!parameter("sponsor").isNull()) {
        throw std::runtime_error("cannot set both sponsor and asset");
    }
    RequestBuilder::setAssetParameter("asset",asset);
    return *this;
}

AccountsRequestBuilder &AccountsRequestBuilder::cursor(QString token) {
    RequestBuilder::cursor(token);
    return *this;
}

AccountsRequestBuilder &AccountsRequestBuilder::limit(int number) {
    RequestBuilder::limit(number);
    return *this;
}

AccountsRequestBuilder &AccountsRequestBuilder::order(RequestBuilder::Order direction) {
    RequestBuilder::order(direction);
    return *this;
}

AccountsRequestBuilder &AccountsRequestBuilder::stream()
{
    setStreamMode();
    return *this;
}

Page<AccountResponse>* AccountsRequestBuilder::execute(QUrl uri) {
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<Page<AccountResponse> >(this);
}

Page<AccountResponse>* AccountsRequestBuilder::execute() {
    return server()->get<Page<AccountResponse> >(this);
}
