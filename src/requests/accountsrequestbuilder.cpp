#include "accountsrequestbuilder.h"
#include "../keypair.h"
#include "../server.h"

AccountsRequestBuilder::AccountsRequestBuilder(Server *server):RequestBuilder(server, "accounts") {

}

AccountResponse *AccountsRequestBuilder::account(QUrl uri)  {
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<AccountResponse>(this);
}

AccountResponse* AccountsRequestBuilder::account(KeyPair *account) {
    this->setSegments(QStringList()<< "accounts"<< account->getAccountId());
    return this->account(this->buildUri());
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
