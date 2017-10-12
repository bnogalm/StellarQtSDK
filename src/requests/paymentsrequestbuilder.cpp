#include "paymentsrequestbuilder.h"
#include "../keypair.h"
#include "../server.h"
#include "../responses/operationpage.h"
#include "../util.h"
PaymentsRequestBuilder::PaymentsRequestBuilder(Server *server):RequestBuilder(server,"payments")
{

}

PaymentsRequestBuilder &PaymentsRequestBuilder::forAccount(KeyPair *account) {
    account = checkNotNull(account, "account cannot be null");
    this->setSegments(QStringList() << "accounts"<< account->getAccountId()<< "payments");
    return *this;
}

PaymentsRequestBuilder &PaymentsRequestBuilder::forLedger(qint64 ledgerSeq) {
    this->setSegments(QStringList() <<"ledgers"<< QString::number(ledgerSeq)<< "payments");
    return *this;
}

PaymentsRequestBuilder &PaymentsRequestBuilder::forTransaction(QString transactionId) {
    checkNotNull(transactionId, "transactionId cannot be null");
    this->setSegments(QStringList()<<"transactions"<< transactionId<< "payments");
    return *this;
}

OperationPage *PaymentsRequestBuilder::execute(QUrl uri) {
    this->setRequestUri(uri);
    return server()->get<OperationPage>(this);
}

PaymentsRequestBuilder& PaymentsRequestBuilder::stream()
{
    setStreamMode();
    return *this;
}

OperationPage* PaymentsRequestBuilder::execute(){
    return execute(this->buildUri());
}
