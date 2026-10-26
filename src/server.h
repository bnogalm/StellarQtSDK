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
#include "requests/liquiditypoolsrequestbuilder.h"
#include <QTimer>
#include <QHash>
#include <QPair>
#include <functional>
#include <QPointer>
#include "qstellar_namespace.h"
#include "responses/response.h"
QSTELLAR_FWD(Transaction)
QSTELLAR_FWD(FeeBumpTransaction)
QSTELLAR_FWD(SubmitTransactionResponse)
QSTELLAR_FWD(SubmitTransactionAsyncResponse)

QSTELLAR_BEGIN_NS

/** What Server::checkMemoRequired() answers: ready(), or error() with lastException(). */
class MemoRequiredCheck : public Response
{
    friend class Server;
public:
    MemoRequiredCheck() : Response(nullptr) {}
};

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



    /**
     * SEP-29: the destinations to check before `transaction` goes out, each
     * with the index of the first operation that pays it, in operation order.
     * Empty when the memo makes the check unnecessary.
     */
    QList<QPair<QString, int>> memoCheckedDestinations(AbstractTransaction* transaction);

    /**
     * Checks the first of `pending` (`attempt` is how many tries it already
     * had), then the rest; `passed` runs once every destination passed, and
     * `failed` takes a refusal or a failed check instead. Neither runs once
     * `owner` is gone.
     */
    void checkMemoDestinations(QPointer<QObject> owner,
                               QList<QPair<QString, int>> pending, int attempt,
                               std::function<void()> passed,
                               std::function<void(qstellar::exception::SdkException*)> failed);

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

    /**
     * The responses it owns (every request's but those of
     * submitTransaction, submitTransactionAsync and checkMemoRequired, which
     * are yours) go with it. Its network manager is deleted later, from the
     * event loop, so that a Server can be deleted from a slot of one of its
     * own responses (a stream's ready() or error()); before 1.0.4 that freed
     * a reply Qt was still using.
     *
     * A request whose response is yours ends then with error(): a
     * ConnectionErrorException saying that its Server was deleted (a
     * submission may still have reached Horizon). It waited forever, with
     * no ready() and no error(). js and py have no Server to delete: their
     * promise or call always settles.
     */
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
     * Returns a builder for the `/liquidity_pools` endpoint.
     */
    LiquidityPoolsRequestBuilder liquidityPools();

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
     *
     * Unless `skipMemoRequiredCheck`, every account the transaction pays
     * (payment, both path payments, account merge; a muxed M destination
     * carries its own id and is not checked) is looked up first when the
     * transaction has no memo (any memo counts, id 0 and an empty text
     * included, as in js and py; before 1.0.4 those two did not), and one
     * that sets SEP-29's `config.memo_required` stops the submission:
     * nothing is posted, and
     * transactionError delivers a response whose lastException() is an
     * AccountRequiresMemoException naming the account and the operation.
     * A lookup that fails (other than "not found") stops it too, with the
     * lookup's own kind of failure, its message naming the account and
     * SEP-29: TooManyRequestsException with the Retry-After (a 429 is not
     * asked again), RequestTimeoutException (Horizon's 504),
     * ClientTimeoutException (no answer within Response::replyTimeout(); it
     * is a ConnectionErrorException too), BadResponseException or
     * BadRequestException with the HTTP status, UnknownResponseException
     * (an unreadable reply, with its HTTP status) or ConnectionErrorException;
     * the others are asked once more first. It used to be a
     * ConnectionErrorException whatever the cause, and before 1.0.4 a lookup
     * that timed out still was.
     *
     * The submission itself can time out too: lastException() is then a
     * RequestTimeoutException (504) or a ClientTimeoutException, and
     * isTimeout() is true: the transaction may still be applied. So may it
     * after a connection error once the POST was written (the connection
     * closed or reset before the answer; on Qt 5 also a refused resend of the
     * POST): look the transaction up by its hash (transactions().transaction)
     * before building it again. Horizon answers a resubmission of the same
     * envelope with the first one's result, so sending the SAME signed
     * envelope again is safe; a new one with a fresh sequence number is not.
     * You lose ownership of the object so don't delete it.
     * You will have to delete the response returned by transactionResponse signal
     * @throws IOException
     */
    SubmitTransactionResponse* submitTransaction(AbstractTransaction* transaction, bool skipMemoRequiredCheck=false);

    /**
     * SEP-29 on its own, as js's Horizon.Server.checkMemoRequired: the check
     * submitTransaction() makes, and nothing is submitted. The response
     * emits ready() when the transaction may go out (it has a memo, pays no
     * account, or none it pays requires one) and error() otherwise, its
     * lastException() what submitTransaction() would have delivered: an
     * AccountRequiresMemoException naming the account and the operation, or
     * the failed lookup's. The caller owns the response.
     */
    MemoRequiredCheck* checkMemoRequired(AbstractTransaction* transaction);

    /**
     * Submits `transaction` through Horizon's POST /transactions_async, as
     * js's submitAsyncTransaction and py's submit_transaction_async: Horizon
     * answers with stellar-core's verdict at once instead of waiting for the
     * ledger (see SubmitTransactionAsyncResponse). Unless
     * `skipMemoRequiredCheck`, SEP-29 runs first exactly as for
     * submitTransaction(), js and py checking too: a destination that
     * requires a memo, or a lookup that fails, stops it before anything is
     * posted, error() carrying the AccountRequiresMemoException or the
     * lookup's failure.
     *
     * Unlike submitTransaction(), the transaction stays yours (it is read
     * before this returns), and the answer comes on the response's own
     * ready() / error(), not on transactionResponse / transactionError.
     * The caller owns the response.
     */
    SubmitTransactionAsyncResponse* submitTransactionAsync(AbstractTransaction* transaction,
                                                           bool skipMemoRequiredCheck = false);



private slots:
    void processSubmitTransactionResponse();
    void processTransactionError();
signals:
    void transactionResponse(SubmitTransactionResponse * response);
    void transactionError(SubmitTransactionResponse * response);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Server)
QSTELLAR_ALIAS(MemoRequiredCheck)

#endif // SERVER_H
