#include "effectsrequestbuilder.h"
#include "../keypair.h"
#include "../util.h"
#include "../server.h"
EffectsRequestBuilder::EffectsRequestBuilder(Server *server):RequestBuilder(server, "effects")
{

}

EffectsRequestBuilder &EffectsRequestBuilder::forAccount(KeyPair *account) {
    account = checkNotNull(account, "account cannot be null");
    this->setSegments(QStringList() <<"accounts"<< account->getAccountId()<< "effects");
    return *this;
}

EffectsRequestBuilder &EffectsRequestBuilder::forLedger(qint64 ledgerSeq) {
    this->setSegments(QStringList()<<"ledgers"<< QString::number(ledgerSeq)<< "effects");
    return *this;
}

EffectsRequestBuilder &EffectsRequestBuilder::forTransaction(QString transactionId) {
    transactionId = checkNotNull(transactionId, "transactionId cannot be null");
    this->setSegments(QStringList()<<"transactions"<< transactionId<< "effects");
    return *this;
}

EffectsRequestBuilder &EffectsRequestBuilder::forOperation(qint64 operationId) {
    this->setSegments(QStringList() << "operations"<< QString::number(operationId)<< "effects");
    return *this;
}

EffectsPage *EffectsRequestBuilder::execute(QUrl uri) {
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<EffectsPage >(this);
}

EffectsRequestBuilder& EffectsRequestBuilder::stream()
{
    setStreamMode();
    return *this;
}

EffectsPage *EffectsRequestBuilder::execute(){
    return execute(this->buildUri());
}

EffectsRequestBuilder &EffectsRequestBuilder::cursor(QString token) {
    RequestBuilder::cursor(token);
    return *this;
}

EffectsRequestBuilder &EffectsRequestBuilder::limit(qint32 number) {
    RequestBuilder::limit(number);
    return *this;
}

EffectsRequestBuilder &EffectsRequestBuilder::order(RequestBuilder::Order direction) {
    RequestBuilder::order(direction);
    return *this;
}
