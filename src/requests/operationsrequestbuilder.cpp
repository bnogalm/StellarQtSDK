#include "operationsrequestbuilder.h"
#include "../keypair.h"
#include "../server.h"
#include "../util.h"
OperationsRequestBuilder::OperationsRequestBuilder(Server *server):RequestBuilder(server, "operations")
{

}

OperationsRequestBuilder &OperationsRequestBuilder::includeFailed(bool value) {
    addParameter("include_failed",value ? "true":"false");
    return *this;
}

GenericOperation* OperationsRequestBuilder::operation(QUrl uri) {
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<GenericOperation>(this);
}

GenericOperation* OperationsRequestBuilder::operation(qint64 operationId) {
    setSegments(QStringList()<< "operation"<< QString::number(operationId));
    return operation(buildUri());
}

OperationsRequestBuilder &OperationsRequestBuilder::forAccount(KeyPair *account) {
    account = checkNotNull(account, "account cannot be null");
    setSegments(QStringList()<<"accounts"<<account->getAccountId()<< "operations");
    return *this;
}

OperationsRequestBuilder &OperationsRequestBuilder::forLedger(qint64 ledgerSeq) {
    setSegments(QStringList()<<"ledgers"<< QString::number(ledgerSeq)<< "operations");
    return *this;
}

OperationsRequestBuilder &OperationsRequestBuilder::forTransaction(QString transactionId) {
    transactionId = checkNotNull(transactionId, "transactionId cannot be null");
    setSegments(QStringList()<<"transactions"<< transactionId<< "operations");
    return *this;
}

OperationsRequestBuilder &OperationsRequestBuilder::forClaimableBalance(QString claimableBalance) {
    claimableBalance = checkNotNull(claimableBalance, "claimableBalance cannot be null");
    setSegments(QStringList()<<"claimable_balances"<< claimableBalance<< "operations");
    return *this;
}

OperationsRequestBuilder &OperationsRequestBuilder::stream()
{
    setStreamMode();
    return *this;
}

OperationPage *OperationsRequestBuilder::execute(QUrl uri){
    this->setRequestUri(uri);
    return server()->get<OperationPage >(this);
}

OperationPage *OperationsRequestBuilder::execute(){
    return execute(buildUri());
}

OperationsRequestBuilder &OperationsRequestBuilder::cursor(QString token) {
    RequestBuilder::cursor(token);
    return *this;
}

OperationsRequestBuilder &OperationsRequestBuilder::limit(int number) {
    RequestBuilder::limit(number);
    return *this;
}

OperationsRequestBuilder &OperationsRequestBuilder::order(RequestBuilder::Order direction) {
    RequestBuilder::order(direction);
    return *this;
}
