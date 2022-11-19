#include "transactionsrequestbuilder.h"
#include "../keypair.h"
#include "../../src/server.h"
TransactionsRequestBuilder::TransactionsRequestBuilder(Server* server):RequestBuilder(server, "transactions")
{
}

TransactionsRequestBuilder &TransactionsRequestBuilder::includeFailed(bool value) {
    addParameter("include_failed", value ? "true" : "false");
    return *this;
}

TransactionResponse* TransactionsRequestBuilder::transaction(QUrl uri) {
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<TransactionResponse>(this);
}

TransactionResponse* TransactionsRequestBuilder::transaction(QString transactionId) {
    this->setSegments(QStringList()<<"transactions"<< transactionId);
    return this->transaction(this->buildUri());
}

TransactionsRequestBuilder &TransactionsRequestBuilder::forAccount(KeyPair *account) {
    account = checkNotNull(account, "account cannot be null");

    this->setSegments(QStringList()<<"accounts"<< account->getAccountId()<< "transactions");
    return *this;
}

TransactionsRequestBuilder& TransactionsRequestBuilder::forLedger(qint64 ledgerSeq) {
    this->setSegments(QStringList()<<"ledgers"<< QString::number(ledgerSeq)<< "transactions");
    return *this;
}

TransactionsRequestBuilder &TransactionsRequestBuilder::forClaimableBalance(QString claimableBalance) {
    claimableBalance = checkNotNull(claimableBalance, "claimableBalance cannot be null");
    setSegments(QStringList() << "claimable_balances" << claimableBalance << "transactions");
    return *this;
}

Page<TransactionResponse> *TransactionsRequestBuilder::execute(QUrl uri){
    this->setRequestUri(uri);//we overwrite the uri
    return server()->get<Page<TransactionResponse> >(this);
}

TransactionsRequestBuilder &TransactionsRequestBuilder::stream()
{
    setStreamMode();
    return *this;
}

Page<TransactionResponse> *TransactionsRequestBuilder::execute() {
    return execute(this->buildUri());
}

TransactionsRequestBuilder &TransactionsRequestBuilder::cursor(QString token) {
    RequestBuilder::cursor(token);
    return *this;
}

TransactionsRequestBuilder &TransactionsRequestBuilder::limit(int number) {
    RequestBuilder::limit(number);
    return *this;
}

TransactionsRequestBuilder &TransactionsRequestBuilder::order(RequestBuilder::Order direction) {
    RequestBuilder::order(direction);
    return *this;
}
