#ifndef SERVER_H
#define SERVER_H
#include <QUrl>
#include <QNetworkAccessManager>
#include "requests/rootrequestbuilder.h"
#include "requests/accountsrequestbuilder.h"
#include "requests/assetsrequestbuilder.h"
#include "requests/paymentsrequestbuilder.h"
#include "requests/transactionsrequestbuilder.h"
#include "requests/tradesrequestbuilder.h"
#include "requests/tradeaggregationsrequestbuilder.h"
#include "requests/strictreceivepathsrequestbuilder.h"
#include "requests/strictsendpathsrequestbuilder.h"
#include "requests/orderbookrequestbuilder.h"
#include "requests/offersrequestbuilder.h"
#include "requests/ledgersrequestbuilder.h"
#include "requests/operationsrequestbuilder.h"
#include "requests/effectsrequestbuilder.h"
#include "requests/feestatsrequestbuilder.h"
#include "requests/claimablebalancesrequestbuilder.h"
#include <QTimer>
#include <QHash>
class Transaction;
class FeeBumpTransaction;
class SubmitTransactionResponse;
/**
 * Main class used to connect to Horizon server.
 */
class Server : public QObject
{
    Q_OBJECT
    QUrl m_serverURI;
    QNetworkAccessManager *m_httpClient;

    QString escapeUri(QString uri);
    QNetworkRequest prepareRequest();



    QList<QString> checkMemoRequired(AbstractTransaction* transaction);

    static QByteArray clientName();
    static QByteArray clientVersion();

public:
    template<class T>
    T* get(RequestBuilder * rb )
    {
        if(rb->requestUri().isEmpty())
            rb->buildUri();
        //qDebug() << "REQUEST "<< rb->requestUri();
        QNetworkRequest request(rb->requestUri());
        if(rb->streamMode())
            request.setRawHeader("Accept","text/event-stream");
        request.setRawHeader("X-Client-Name", clientName());
        request.setRawHeader("X-Client-Version", clientVersion());
        QNetworkReply * r = m_httpClient->get(request);
        auto response = new T(r);
        response->setParent(this);//so it is not collected by QML
        return response;
    }

    Server(QString uri);

    ~Server();

    QNetworkAccessManager& manager();

    QUrl serverURI();
    /**
         * Returns {@link RootRequestBuilder} instance.
         */
    RootRequestBuilder root();

    /**
         * Returns {@link AccountsRequestBuilder} instance.
         */
    AccountsRequestBuilder accounts();

    /**
     * Returns {@link AssetsRequestBuilder} instance.
     */
    AssetsRequestBuilder assets();

    /**
     * Returns {@link ClaimableBalancesRequestBuilder} instance.
     */
    ClaimableBalancesRequestBuilder claimableBalances();

    /**
         * Returns {@link EffectsRequestBuilder} instance.
         */
    EffectsRequestBuilder effects();

    /**
    * Returns {@link LedgersRequestBuilder} instance.
    */
    LedgersRequestBuilder ledgers();

    /**
    * Returns {@link OffersRequestBuilder} instance.
    */
    OffersRequestBuilder offers();

    /**
         * Returns {@link OperationsRequestBuilder} instance.
         */
    OperationsRequestBuilder operations();

    /**
       * Returns {@link FeeStatsResponse} instance.
       */
    FeeStatsRequestBuilder operationFeeStats();

    /**
         * Returns {@link OrderBookRequestBuilder} instance.
         */
    OrderBookRequestBuilder orderBook();
    /**
         * Returns {@link TradesRequestBuilder} instance.
         */
    TradesRequestBuilder trades();

    /**
    * Returns {@link TradeAggregationsRequestBuilder} instance.
    */
    TradeAggregationsRequestBuilder tradeAggregations(Asset* baseAsset, Asset* counterAsset, qint64 startTime, qint64 endTime, qint64 resolution, qint64 offset);


    /**
    * Returns {@link StrictReceivePathsRequestBuilder} instance.
    */
    StrictReceivePathsRequestBuilder strictReceivePaths();

    /**
    * Returns {@link StrictSendPathsRequestBuilder} instance.
    */
    StrictSendPathsRequestBuilder strictSendPaths();

    /**
    * Returns {@link PaymentsRequestBuilder} instance.
    */
    PaymentsRequestBuilder payments();

    /**
    * Returns {@link TransactionsRequestBuilder} instance.
    */
    TransactionsRequestBuilder transactions();

    /**
     * Submits transaction to the network.
     * @param transaction transaction to submit to the network.
     * Async method, you will get the response at transactionResponse signal
     * You lose ownership of the object so don't delete it.
     * You will have to delete the response returned by transactionResponse signal
     * @throws IOException
     */
    SubmitTransactionResponse* submitTransaction(AbstractTransaction* transaction, bool skipMemoRequiredCheck=false);



private slots:
    void processSubmitTransactionResponse();
    void processTransactionError();
signals:
    void transactionResponse(SubmitTransactionResponse * response);
    void transactionError(SubmitTransactionResponse * response);
};


#endif // SERVER_H
