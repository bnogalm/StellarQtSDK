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
#include "responses/friendbotresponse.h"
QSTELLAR_FWD(Transaction)
QSTELLAR_FWD(FeeBumpTransaction)
QSTELLAR_FWD(SubmitTransactionResponse)
QSTELLAR_FWD(SubmitTransactionAsyncResponse)
QSTELLAR_FWD(TimeBounds)
QSTELLAR_FWD(KeyPair)

QSTELLAR_BEGIN_NS

/** What Server::checkMemoRequired() answers: ready(), or error() with lastException(). */
class MemoRequiredCheck : public Response
{
    friend class Server;
public:
    MemoRequiredCheck() : Response(nullptr) {}
};

/**
 * What Server::fetchBaseFee() answers: Horizon's /fee_stats, read exactly as
 * a FeeStatsResponse, and baseFee() once ready(). The caller owns it.
 */
class BaseFeeResponse : public FeeStatsResponse
{
    Q_OBJECT
    friend class Server;
public:
    explicit BaseFeeResponse(QNetworkReply* reply = nullptr);
    virtual ~BaseFeeResponse();

    /** Reads /fee_stats; throws std::runtime_error when last_ledger_base_fee is not a positive number of stroops. */
    void loadFromJson(QByteArray data) override;

    /** The last ledger's base fee, in stroops; 0 before ready(). */
    qint64 baseFee() const;
};

/**
 * What Server::fetchTimebounds() answers: time bounds {0, the server's time
 * plus the seconds asked for}, once ready(). The caller owns it.
 */
class TimeboundsResponse : public Response
{
    Q_OBJECT
    friend class Server;
    qint64 m_seconds;
    qint64 m_maxTime;
    bool m_known;
    // The request for the server's time, while it runs.
    QPointer<QNetworkReply> m_serverTimeRequest;
    void deliver(qint64 serverNow);
public:
    explicit TimeboundsResponse(qint64 seconds);
    virtual ~TimeboundsResponse();

    /** Always 0, as js's fetchTimebounds gives it. */
    qint64 minTime() const;

    /**
     * The server's time, in seconds since the Unix epoch, plus the seconds
     * asked for: always positive once ready() (0 would be no upper bound);
     * 0 before.
     */
    qint64 maxTime() const;

    /**
     * New TimeBounds {minTime(), maxTime()}, for
     * TransactionBuilder::addTimeBounds (which takes it over). You own the
     * result. Throws std::runtime_error before ready().
     */
    TimeBounds* toTimeBounds() const;
};

/**
 * Main class used to connect to Horizon server.
 */
class Server : public QObject
{
    Q_OBJECT
    QUrl m_serverURI;
    QNetworkAccessManager *m_httpClient;

    // The Date of the last answer from this Server's host and port, in
    // seconds since the Unix epoch, and monotonicMSecs() when it came; -1
    // when none came yet.
    qint64 m_serverTime;
    qint64 m_serverTimeRecordedAt;

    QString escapeUri(QString uri);
    QNetworkRequest prepareRequest();

    /** A GET of `url` with the headers every request of this SDK carries. */
    QNetworkRequest getRequest(const QUrl& url) const;

    /**
     * The server's time now, from the last Date recorded, when it came
     * within the last 5 minutes (js's rule), moved on by the time elapsed
     * since; false otherwise.
     */
    bool currentServerTime(qint64* now) const;

    /** fetchTimebounds() with no recent server time: asks the Horizon root for its Date. */
    void askServerTime(QPointer<TimeboundsResponse> response);



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
     * submitTransaction, submitTransactionAsync, checkMemoRequired,
     * fetchBaseFee and fetchTimebounds, which are yours) go with it. Its network manager is deleted later, from the
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

    /**
     * The last ledger's base fee, in stroops, as js's
     * Horizon.Server.fetchBaseFee: GET /fee_stats and its
     * last_ledger_base_fee. ready() once it is read, baseFee() holding it;
     * error() otherwise, lastException() saying why: the request's own
     * failure (an HTTP error, no answer within Response::replyTimeout(), a
     * connection that failed, the Server deleted), or an
     * UnknownResponseException for an answer with no positive base fee in it
     * (missing, null, 0, negative, not a number) or that is not /fee_stats.
     *
     * js answers 100 when the fee does not parse or is 0 (and returns a
     * negative one as it is), and py (fetch_base_fee, which reads the
     * latest ledger's base_fee_in_stroops) when there is no ledger; neither
     * when the request fails. Here no fee is made up: a
     * caller that wants that default takes 100 on error() (or
     * Transaction::Builder::BASE_FEE). The caller owns the response.
     */
    BaseFeeResponse* fetchBaseFee();

    /**
     * Time bounds for a transaction valid for `seconds` from now by the
     * server's clock, as js's Horizon.Server.fetchTimebounds: {minTime 0,
     * maxTime: the server's time + seconds}. A device whose clock is off
     * would otherwise sign transactions Horizon takes as expired, or valid
     * for longer than meant.
     *
     * The server's time is the Date header of the last answer this Server's
     * network manager got from its own host and port (any request: an
     * account, a submission, ...), when it came within the last 5 minutes,
     * moved on by the time elapsed since on a monotonic clock (js uses the
     * wall clock, which a clock change moves). It is taken when the
     * answer's headers arrive, the moment the Date was written, not when its
     * body ends. Streams do not count: a stream's headers come once, when it
     * opens, and js's EventSource streams are not recorded either. Without
     * one, the Horizon root is asked (GET /) and its Date is used; an answer
     * without a readable Date, or with one at or before 1970, gives the
     * local time, as js's retry does. js keeps one table for every Server in
     * the process, keyed by host and port, and only from 2xx answers, and
     * an answer without a Date erases what it had; here each Server keeps
     * its own, from every HTTP answer (an error's Date is the server's clock
     * too), and an answer without a Date leaves the last one. For the same
     * reason the root's Date is used whatever its status; js fails on an
     * HTTP error there. js records a redirected answer's Date under the
     * host it asked (axios's config.url); here only the host that answered
     * counts, so the Date of a friendbot that Horizon redirected to is not
     * taken for Horizon's clock.
     *
     * Answered from the event loop, so the caller connects first. error()
     * only when the root is not answered at all: a connection that failed,
     * no answer within Response::replyTimeout() (ClientTimeoutException),
     * or the Server deleted (ConnectionErrorException), as js propagates the
     * error of its request; or, with an SdkException, when the root sent no
     * usable Date and the local clock is at or before 1970, so that ready()
     * never gives a maxTime of 0 (no upper bound) or a negative one.
     *
     * Throws std::invalid_argument for negative seconds (js does not check:
     * it gives a maxTime in the past, a transaction that cannot be applied),
     * or for more than a maxTime past the year 9999 could need, which would
     * not fit a qint64. The caller owns the response.
     */
    TimeboundsResponse* fetchTimebounds(qint64 seconds);

    /**
     * Asks this Horizon's friendbot to fund `account`, as js's
     * Horizon.Server.friendbot(addr).call(): GET <serverURI>/friendbot?addr=G...
     * SDF's test Horizons answer that with a 307 to friendbot.stellar.org;
     * redirects are followed (set on the request: Qt 5.15 does not follow
     * them by default), never from https to http: js (node's
     * follow-redirects) follows that one too, dropping only confidential
     * headers; it is refused here so that a funding request is never
     * downgraded to plain http. py's Horizon Server has no
     * friendbot. See FriendbotResponse for what is ready() and what is
     * error(); like every Horizon request it is given up after
     * Response::replyTimeout() (ClientTimeoutException: the account may
     * still be funded). The response is this Server's, as get() makes them:
     * deleted with it, or by you before.
     */
    FriendbotResponse* friendbot(KeyPair* account);

    /**
     * The same, asking the friendbot at `friendbotUrl` (an http or https
     * URL; SDF's is https://friendbot.stellar.org): its query is kept and
     * addr set in it, replacing any addr it had, as QML's
     * Horizon.fundAccount() does. Throws std::runtime_error for a URL that is
     * not http or https.
     */
    FriendbotResponse* friendbot(KeyPair* account, const QUrl& friendbotUrl);

protected:
    /**
     * Milliseconds on a monotonic clock (QElapsedTimer), for the age of the
     * server time fetchTimebounds() reads. A subclass may give its own clock.
     */
    virtual qint64 monotonicMSecs() const;

    /** The local time, seconds since the Unix epoch: fetchTimebounds()'s last resort. */
    virtual qint64 localSecsSinceEpoch() const;

private slots:
    void processSubmitTransactionResponse();
    void processTransactionError();
    /** Keeps the Date of an answer from this Server's host and port as its headers arrive (see fetchTimebounds()). */
    void recordServerTime(QNetworkReply* reply);
signals:
    void transactionResponse(SubmitTransactionResponse * response);
    void transactionError(SubmitTransactionResponse * response);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Server)
QSTELLAR_ALIAS(MemoRequiredCheck)
QSTELLAR_ALIAS(BaseFeeResponse)
QSTELLAR_ALIAS(TimeboundsResponse)

#endif // SERVER_H
