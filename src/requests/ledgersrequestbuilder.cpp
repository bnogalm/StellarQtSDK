#include "ledgersrequestbuilder.h"
#include "../server.h"
LedgersRequestBuilder::LedgersRequestBuilder(Server *server):RequestBuilder(server, "ledgers")
{

}

LedgerResponse* LedgersRequestBuilder::ledger(QUrl uri) {
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<LedgerResponse>(this);
}

LedgerResponse *LedgersRequestBuilder::ledger(long ledgerSeq){
    setSegments(QStringList()<<"ledgers"<< QString::number(ledgerSeq));
    return ledger(this->buildUri());
}

Page<LedgerResponse>* LedgersRequestBuilder::execute(QUrl uri) {
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<Page<LedgerResponse> >(this);
}

LedgersRequestBuilder &LedgersRequestBuilder::stream() {
    setStreamMode();
    return *this;
}

Page<LedgerResponse>* LedgersRequestBuilder::execute()  {
    return execute(this->buildUri());
}

LedgersRequestBuilder& LedgersRequestBuilder::cursor(QString token) {
    RequestBuilder::cursor(token);
    return *this;
}

LedgersRequestBuilder& LedgersRequestBuilder::limit(int number) {
    RequestBuilder::limit(number);
    return *this;
}

LedgersRequestBuilder& LedgersRequestBuilder::order(RequestBuilder::Order direction) {
    RequestBuilder::order(direction);
    return *this;
}
