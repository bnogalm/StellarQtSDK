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
    request.setRawHeader("X-Client-Name", STELLAR_QT_SDK_CLIENT_NAME);
    request.setRawHeader("X-Client-Version", STELLAR_QT_SDK_CLIENT_VERSION);
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

RootRequestBuilder Server::root()
{
    return RootRequestBuilder(this);
}

AccountsRequestBuilder Server::accounts() {
    return AccountsRequestBuilder(this);
}

AssetsRequestBuilder Server::assets() {
    return AssetsRequestBuilder(this);
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

FeeStatsRequestBuilder Server::operationFeeStats() {
    return FeeStatsRequestBuilder(this);
}

OrderBookRequestBuilder Server::orderBook() {
    return OrderBookRequestBuilder(this);
}

TradesRequestBuilder Server::trades() {
    return TradesRequestBuilder(this);
}

TradeAggregationsRequestBuilder Server::tradeAggregations(Asset *baseAsset, Asset *counterAsset, qint64 startTime, qint64 endTime, qint64 resolution, qint64 offset) {
    return TradeAggregationsRequestBuilder(this, baseAsset, counterAsset, startTime, endTime, resolution, offset);
}

StrictReceivePathsRequestBuilder Server::strictReceivePaths() {
    return StrictReceivePathsRequestBuilder(this);
}

StrictSendPathsRequestBuilder Server::strictSendPaths() {
    return StrictSendPathsRequestBuilder(this);
}

PaymentsRequestBuilder Server::payments() {
    return PaymentsRequestBuilder(this);
}

TransactionsRequestBuilder Server::transactions() {
    return TransactionsRequestBuilder(this);
}

SubmitTransactionResponse *Server::submitTransaction(Transaction *transaction) {
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
    connect(response, &SubmitTransactionResponse::error, this, &Server::processTransactionError);
    return response;
}

void Server::processSubmitTransactionResponse()
{
    if(SubmitTransactionResponse * response = dynamic_cast<SubmitTransactionResponse*>(sender())){
        emit transactionResponse(response);//you have to delete the response
    }
}

void Server::processTransactionError()
{
    if(SubmitTransactionResponse * response = dynamic_cast<SubmitTransactionResponse*>(sender())){
        emit transactionError(response);//you have to delete the response
    }
}
