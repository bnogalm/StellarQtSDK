#include "server.h"

#include "transaction.h"
#include "responses/submittransactionresponse.h"

QString Server::escapeUri(QString uri)
{
    return uri.replace('+',"%2B").replace('/',"%2F");//when passing data on the POST payload, we have to encode this two characters
}

QNetworkRequest Server::prepareRequest()
{
    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    return request;
}

Server::Server(QString uri) {
    m_serverURI = QUrl(uri);
    if(!m_serverURI.isValid())
        throw std::runtime_error("invalid uri");
    m_httpClient=new QNetworkAccessManager(this);
}

Server::~Server()
{
}

QUrl Server::serverURI()
{
    return this->m_serverURI;
}

AccountsRequestBuilder Server::accounts() {
    return AccountsRequestBuilder(this);
}

EffectsRequestBuilder Server::effects() {
    return EffectsRequestBuilder(this);
}

LedgersRequestBuilder Server::ledgers() {
    return LedgersRequestBuilder(this);
}

OffersRequestBuilder Server::offers() {
    return OffersRequestBuilder(this);
}

OperationsRequestBuilder Server::operations() {
    return OperationsRequestBuilder(this);
}


OrderBookRequestBuilder Server::orderBook() {
    return OrderBookRequestBuilder(this);
}

TradesRequestBuilder Server::trades() {
    return TradesRequestBuilder(this);
}

PathsRequestBuilder Server::paths() {
    return PathsRequestBuilder(this);
}

PaymentsRequestBuilder Server::payments() {
    return PaymentsRequestBuilder(this);
}

TransactionsRequestBuilder Server::transactions() {
    return TransactionsRequestBuilder(this);
}

void Server::submitTransaction(Transaction *transaction) {
    QUrl transactionsURI(serverURI().toString().append("/transactions"));
    if(!transactionsURI.isValid())
    {
        throw std::runtime_error("invalid uri");
    }

    QUrlQuery query;

    query.addQueryItem("tx",transaction->toEnvelopeXdrBase64());

    QNetworkRequest submitTransactionRequest = prepareRequest();
    submitTransactionRequest.setUrl(transactionsURI);


    QNetworkReply * reply = this->m_httpClient->post(submitTransactionRequest,escapeUri(query.toString()).toLatin1());

    auto response = new SubmitTransactionResponse(reply,transaction);

    connect(response, &SubmitTransactionResponse::ready, this, &Server::processSubmitTransactionResponse);
    connect(response, &SubmitTransactionResponse::error, this, &Server::transactionError);
}

void Server::processSubmitTransactionResponse()
{
    if(SubmitTransactionResponse * response = dynamic_cast<SubmitTransactionResponse*>(sender())){
        emit transactionResponse(response);//you have to delete the response
    }
}
