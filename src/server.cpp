#include "server.h"

#include "transaction.h"
#include "feebumptransaction.h"
#include "responses/submittransactionresponse.h"
#include "paymentoperation.h"
#include "pathpaymentstrictreceiveoperation.h"
#include "pathpaymentstrictsendoperation.h"
#include "accountmergeoperation.h"

#include "checkaccountrequiresmemo.h"


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

QList<QString> Server::checkMemoRequired(AbstractTransaction *abstractTransaction)
{    
    Transaction*  transaction = dynamic_cast<Transaction*>(abstractTransaction);
    if(!transaction)
    {
        if(FeeBumpTransaction *feeBumpTransaction = dynamic_cast<FeeBumpTransaction*>(abstractTransaction))
        {
            transaction = feeBumpTransaction->getInnerTransaction();
        }
    }
    if (Memo *memo = transaction->getMemo()){
        if(MemoText * text = dynamic_cast<MemoText*>(memo))
        {
            if(!text->getText().isEmpty())
                return QList<QString>();//if we have any text we accept it. We consider empty text as forgotten memo.
        }
        else if(MemoHashAbstract * hash = dynamic_cast<MemoHashAbstract*>(memo))
        {
            if(!hash->getHexValue().isEmpty())
                    return QList<QString>();
        }
        else if(MemoId * id = dynamic_cast<MemoId*>(memo))
        {
            if(id->getId()!=0)//i am sorry for this case, but i don't know why a server will require id 0, SEP29 should define a regular expression to validate memo.
                return QList<QString>();
        }
    }


    QSet<QString> destinations;
    QVector<Operation*> operations = transaction->getOperations();
    for(Operation* op : operations){
        QString destination;
        if (PaymentOperation * payment = dynamic_cast<PaymentOperation*>(op)) {
            destination = payment->getDestination();
        } else if (PathPaymentStrictReceiveOperation * payment = dynamic_cast<PathPaymentStrictReceiveOperation*>(op)) {
            destination =  payment->getDestination();
        } else if (PathPaymentStrictSendOperation * payment = dynamic_cast<PathPaymentStrictSendOperation*>(op)) {
            destination =  payment->getDestination();
        } else if (AccountMergeOperation * payment = dynamic_cast<AccountMergeOperation*>(op)) {
            destination =  payment->getDestination();
        } else {
            continue;
        }        
        if(StrKey::encodeToXDRMuxedAccount(destination).type==stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519)
            continue;
        destinations.insert(destination);
    }
    return destinations.values();
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

QNetworkAccessManager &Server::manager()
{
    return *m_httpClient;
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

ClaimableBalancesRequestBuilder Server::claimableBalances() {
    return ClaimableBalancesRequestBuilder(this);
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

SubmitTransactionResponse *Server::submitTransaction(AbstractTransaction *transaction, bool skipMemoRequiredCheck) {
    QUrl transactionsURI(serverURI().toString().append("/transactions"));
    if(!transactionsURI.isValid())
    {
        throw std::runtime_error("invalid uri");
    }

    QList<QString> pendingCheckAddressMemos;
    if(!skipMemoRequiredCheck)
        pendingCheckAddressMemos = checkMemoRequired(transaction);


    SubmitTransactionResponse *response = nullptr;
    QUrlQuery query;
    query.addQueryItem("tx",transaction->toEnvelopeXdrBase64());
    QNetworkRequest submitTransactionRequest = prepareRequest();
    submitTransactionRequest.setUrl(transactionsURI);
    QByteArray uri = escapeUri(query.toString()).toLatin1();
    if(pendingCheckAddressMemos.isEmpty())
    {
        QNetworkReply * reply = this->m_httpClient->post(submitTransactionRequest,uri);

        response = new SubmitTransactionResponse(reply,transaction);
    }
    else{
        response = new SubmitTransactionResponse(nullptr,transaction);
        //we will generate the request only after all the destination address are validated.

        CheckAccountRequiresMemo * check = new CheckAccountRequiresMemo(this,pendingCheckAddressMemos);

        connect(check, &CheckAccountRequiresMemo::error, response, &SubmitTransactionResponse::error);
        connect(check, &CheckAccountRequiresMemo::validated, [this,response,submitTransactionRequest,uri](){
            QNetworkReply * reply = this->m_httpClient->post(submitTransactionRequest,uri);
            response->loadFromReply(reply);
        });
    }

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
