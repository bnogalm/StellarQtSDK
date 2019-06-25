#include "offersrequestbuilder.h"
#include "../keypair.h"
#include "../server.h"
OffersRequestBuilder::OffersRequestBuilder(Server *server):RequestBuilder(server, "offers")
{

}

OffersRequestBuilder &OffersRequestBuilder::forAccount(KeyPair *account) {
    account = checkNotNull(account, "account cannot be null");

    this->setSegments(QStringList()<<"accounts"<< account->getAccountId()<< "offers");
    return *this;
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

OffersRequestBuilder &OffersRequestBuilder::stream() {
    setStreamMode();
    return *this;
}
