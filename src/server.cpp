#include "server.h"
#include "sep29.h"

#include "transaction.h"
#include "feebumptransaction.h"
#include "responses/accountresponse.h"
#include "responses/submittransactionresponse.h"
#include "responses/submittransactionasyncresponse.h"
#include "strkey.h"
#include "exception/accountrequiresmemoexception.h"
#include "exception/badrequestexception.h"
#include "exception/badresponseexception.h"
#include "exception/connectionerrorexception.h"
#include "exception/requesttimeoutexception.h"
#include "exception/toomanyrequestsexception.h"
#include "exception/unknownresponseexception.h"

#include <QPointer>
#include <QScopedPointer>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

QSTELLAR_BEGIN_NS

namespace {

/** SEP-29: the data entry, and its value as Horizon returns it, that asks for a memo. */
const char* const MEMO_REQUIRED_KEY = Sep29::DATA_NAME;
const char* const MEMO_REQUIRED_VALUE = Sep29::DATA_VALUE_BASE64;

/** Tries per destination before a failed lookup stops the submission. */
const int MEMO_CHECK_ATTEMPTS = 2;

/**
 * The account an operation pays, if it is one of the four SEP-29 names, read
 * from the operation as it will be submitted. nullptr for any other.
 */
const stellar::MuxedAccount* memoCheckedDestination(const stellar::Operation& op)
{
    switch (op.type) {
    case stellar::OperationType::PAYMENT:
        return &op.operationPayment.destination;
    case stellar::OperationType::PATH_PAYMENT_STRICT_RECEIVE:
        return &op.operationPathPaymentStrictReceive.destination;
    case stellar::OperationType::PATH_PAYMENT_STRICT_SEND:
        return &op.operationPathPaymentStrictSend.destination;
    case stellar::OperationType::ACCOUNT_MERGE:
        return &op.operationAccountMerge;
    default:
        return nullptr;
    }
}

/**
 * Whether the memo counts as given: any memo does, as js's
 * checkMemoRequired (memo.type !== "none") and py's (not NoneMemo) decide.
 * An empty text and id 0 used to count as none, so a transaction both
 * submit was refused with AccountRequiresMemoException, and only here.
 */
bool memoGiven(const stellar::Memo& memo)
{
    return memo.type != stellar::MemoType::MEMO_NONE;
}

/** A lookup that says the account does not exist, which SEP-29 lets through. */
bool accountNotFound(Response* response)
{
    if (response->getStatus() == 404)
        return true;
    auto* ex = dynamic_cast<qstellar::exception::NetworkException*>(response->lastException());
    return ex && ex->getCode() == 404;
}

/** A lookup Horizon refused for the rate limit: asked again at once, it only
 *  spends more of the allowance the Retry-After says to wait for. */
bool rateLimited(Response* response)
{
    return response->getStatus() == 429
        || dynamic_cast<qstellar::exception::TooManyRequestsException*>(response->lastException());
}

/**
 * The lookup's own failure, raised on the submission with a message that
 * names the account and SEP-29. The class is the lookup's, so a caller (and
 * HorizonQml) still tells a rate limit from a timeout or a server error by
 * it, and reads the same Retry-After or HTTP status; only what() says more.
 */
template <class Failure>
class MemoCheckFailure : public Failure
{
public:
    template <class... Args>
    explicit MemoCheckFailure(const QString& message, Args&&... args)
        : Failure(std::forward<Args>(args)...), m_message(message.toStdString()) {}
    const char* what() const noexcept override { return m_message.c_str(); }
private:
    std::string m_message;
};

/**
 * Every failed lookup used to become a ConnectionErrorException, whatever it
 * was: a 429 lost its rate-limit kind and its Retry-After, a 5xx its status,
 * and all of them read as "check your connection".
 */
qstellar::exception::SdkException* memoCheckFailure(const QString& destination, Response* lookup)
{
    using namespace qstellar::exception;
    SdkException* cause = lookup->lastException();
    QString reason = cause ? QString::fromUtf8(cause->what()) : QStringLiteral("request failed");
    auto* network = dynamic_cast<NetworkException*>(cause);
    if (network && network->getCode() > 0)
        reason += QStringLiteral(" (HTTP %1)").arg(network->getCode());
    const QString message = QStringLiteral("could not check whether %1 requires a memo (SEP-29): "
                                           "%2; the transaction was not submitted")
                                .arg(destination, reason);

    // The lookup's problem document is not carried over: attached, its title
    // would stand in for this message, which is the one that says SEP-29.
    if (auto* e = dynamic_cast<TooManyRequestsException*>(cause))
        return new MemoCheckFailure<TooManyRequestsException>(message, e->getRetryAfter());
    if (auto* e = dynamic_cast<RequestTimeoutException*>(cause))
        return new MemoCheckFailure<RequestTimeoutException>(message, e->getCode(), e->getBody());
    if (auto* e = dynamic_cast<BadResponseException*>(cause))
        return new MemoCheckFailure<BadResponseException>(message, e->getCode(), e->getBody(), nullptr);
    if (auto* e = dynamic_cast<BadRequestException*>(cause))
        return new MemoCheckFailure<BadRequestException>(message, e->getCode(), e->getBody(), nullptr);
    if (auto* e = dynamic_cast<UnknownResponseException*>(cause))
        return new MemoCheckFailure<UnknownResponseException>(message, e->getCode(), e->getBody());
    // The lookup ran out of time: a timeout, not a connection that failed,
    // which it was reported as.
    if (auto* e = dynamic_cast<ClientTimeoutException*>(cause))
        return new MemoCheckFailure<ClientTimeoutException>(message, e->getTimeout());
    if (lookup->getStatus() == 429)
        return new MemoCheckFailure<TooManyRequestsException>(message, -1);
    // The connection failed, or nothing says why.
    return new MemoCheckFailure<ConnectionErrorException>(message);
}

} // namespace

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

QList<QPair<QString, int>> Server::memoCheckedDestinations(AbstractTransaction *abstractTransaction)
{
    // Read from the envelope that is about to be posted, not from the C++
    // objects it was built from. This recognised operations by class, and
    // Operation::fromXdr returns the deprecated PathPaymentOperation for a
    // strict-receive path payment: one decoded from XDR (a wallet signing a
    // SEP-7 request, a relayed envelope) went out without the check. The
    // envelope is also where a muxed destination written under a converter
    // that disables muxing has become the G account it really pays.
    const stellar::TransactionEnvelope envelope = abstractTransaction->toEnvelopeXdr();
    const stellar::Memo* memo = nullptr;
    const QVector<stellar::Operation>* operations = nullptr;
    switch (envelope.type) {
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0:
        memo = &envelope.v0.tx.memo;
        operations = &envelope.v0.tx.operations.value;
        break;
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX:
        memo = &envelope.v1.tx.memo;
        operations = &envelope.v1.tx.operations.value;
        break;
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
        if (envelope.feeBump.tx.type != stellar::EnvelopeType::ENVELOPE_TYPE_TX)
            throw std::runtime_error("SEP-29: unknown inner transaction type in a fee bump");
        memo = &envelope.feeBump.tx.v1.tx.memo;
        operations = &envelope.feeBump.tx.v1.tx.operations.value;
        break;
    default:
        throw std::runtime_error("SEP-29: unknown transaction envelope type");
    }

    QList<QPair<QString, int>> destinations;
    if (memoGiven(*memo))
        return destinations;

    QSet<QString> seen;
    for (int i = 0; i < operations->size(); ++i) {
        const stellar::MuxedAccount* destination = memoCheckedDestination(operations->at(i));
        // A muxed destination carries its own id, which is what the memo
        // would have said.
        if (!destination || destination->type == stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519)
            continue;
        const QString accountId = StrKey::encodeStellarMuxedAccount(*destination);
        if (seen.contains(accountId))
            continue;
        seen.insert(accountId);
        destinations.append(qMakePair(accountId, i));
    }
    return destinations;
}

void Server::checkMemoDestinations(QPointer<QObject> owner,
                                   QList<QPair<QString, int>> pending, int attempt,
                                   std::function<void()> passed,
                                   std::function<void(qstellar::exception::SdkException*)> failed)
{
    if (!owner)
        return;
    if (pending.isEmpty()) {
        passed();
        return;
    }
    const QString destination = pending.first().first;
    const int operationIndex = pending.first().second;

    AccountResponse* account = nullptr;
    {
        QScopedPointer<KeyPair> key(KeyPair::fromAccountId(destination));
        account = accounts().account(key.data());
    }
    // Acted on once, whichever of ready/error arrives: a second post of the
    // same envelope is not something to risk on a signal emitted twice.
    auto done = std::make_shared<bool>(false);

    connect(account, &Response::ready, this,
            [this, account, owner, done, pending, destination, operationIndex, passed, failed]() {
        if (*done)
            return;
        *done = true;
        account->deleteLater();
        if (!owner)
            return;
        if (account->getData().get(QString::fromLatin1(MEMO_REQUIRED_KEY))
                == QLatin1String(MEMO_REQUIRED_VALUE)) {
            // Reported as what it is, naming the account and the operation:
            // this used to be a bare error() that a caller could only read as
            // "the connection failed".
            failed(new qstellar::exception::AccountRequiresMemoException(
                QStringLiteral("account %1 requires a memo (SEP-29); the transaction was "
                               "not submitted").arg(destination),
                destination, operationIndex));
            return;
        }
        checkMemoDestinations(owner, pending.mid(1), 0, passed, failed);
    });

    connect(account, &Response::error, this,
            [this, account, owner, done, pending, attempt, destination, passed, failed]() {
        if (*done)
            return;
        *done = true;
        account->deleteLater();
        if (!owner)
            return;
        if (accountNotFound(account)) {
            // An account that does not exist yet has no data entries.
            checkMemoDestinations(owner, pending.mid(1), 0, passed, failed);
            return;
        }
        // Asked again once, except after a 429: that one says when to come
        // back, and was retried at once.
        if (!rateLimited(account) && attempt + 1 < MEMO_CHECK_ATTEMPTS) {
            checkMemoDestinations(owner, pending, attempt + 1, passed, failed);
            return;
        }
        failed(memoCheckFailure(destination, account));
    });

    // The lookup is this Server's: deleted with it, the connections above
    // went too, and the owner (a submission, a check) waited forever. It is
    // told instead, from the event loop.
    connect(account, &QObject::destroyed, owner.data(), [done, owner, destination, failed]() {
        if (*done || !owner)
            return;
        *done = true;
        failed(new qstellar::exception::ConnectionErrorException(
            QStringLiteral("SEP-29: the lookup of account %1 was abandoned: its Server was deleted")
                .arg(destination)));
    }, Qt::QueuedConnection);
}

QByteArray Server::clientName()
{
    return STELLAR_QT_SDK_CLIENT_NAME;
}

QByteArray Server::clientVersion()
{
    return STELLAR_QT_SDK_CLIENT_VERSION;
}

Server::Server(QString uri) {
    m_serverURI = QUrl(uri);
    if(!m_serverURI.isValid())
        throw std::runtime_error("invalid uri");
    m_httpClient=new QNetworkAccessManager(this);
}

Server::~Server()
{
    // Deleted later, from the event loop: a Server may be deleted from a slot
    // of one of its responses (a stream's ready()), which runs inside a
    // signal of one of the manager's replies, and deleting the manager here
    // deleted that reply under Qt's feet. The responses this Server owns go
    // now, with it, and leave their replies to be deleted later too.
    m_httpClient->setParent(nullptr);
    m_httpClient->deleteLater();
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

LiquidityPoolsRequestBuilder Server::liquidityPools() {
    return LiquidityPoolsRequestBuilder(this);
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

    QList<QPair<QString, int>> pendingCheckAddressMemos;
    if(!skipMemoRequiredCheck)
        pendingCheckAddressMemos = memoCheckedDestinations(transaction);


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
        // Nothing is posted until every destination has been checked. This
        // used to go through CheckAccountRequiresMemo, whose single error()
        // signal could neither say which account asked for a memo nor tell
        // that apart from a lookup that failed: both reached the caller as
        // an error with no reason attached.
        QPointer<SubmitTransactionResponse> guard(response);
        checkMemoDestinations(response, pendingCheckAddressMemos, 0,
            [this, guard, submitTransactionRequest, uri]() {
                guard->loadFromReply(m_httpClient->post(submitTransactionRequest, uri));
            },
            [guard](qstellar::exception::SdkException* failure) { guard->emitError(failure); });
    }

    connect(response, &SubmitTransactionResponse::ready, this, &Server::processSubmitTransactionResponse);
    connect(response, &SubmitTransactionResponse::error, this, &Server::processTransactionError);
    return response;
}

MemoRequiredCheck *Server::checkMemoRequired(AbstractTransaction *transaction)
{
    auto* check = new MemoRequiredCheck();
    const QList<QPair<QString, int>> pending = memoCheckedDestinations(transaction);
    QPointer<MemoRequiredCheck> guard(check);
    QPointer<Server> self(this);
    // Queued even when there is nothing to look up, so that the caller has
    // connected to the response before it answers. Queued on the check, not
    // on this Server: a Server deleted before the start dropped it, and the
    // check never answered.
    QMetaObject::invokeMethod(check, [self, guard, pending]() {
        if (!guard)
            return;
        if (!self) {
            guard->emitError(new qstellar::exception::ConnectionErrorException(
                QStringLiteral("SEP-29: the check was abandoned: its Server was deleted")));
            return;
        }
        self->checkMemoDestinations(guard.data(), pending, 0,
            [guard]() { emit guard->ready(); },
            [guard](qstellar::exception::SdkException* failure) { guard->emitError(failure); });
    }, Qt::QueuedConnection);
    return check;
}

SubmitTransactionAsyncResponse *Server::submitTransactionAsync(AbstractTransaction *transaction,
                                                               bool skipMemoRequiredCheck)
{
    QUrl transactionsURI(serverURI().toString().append("/transactions_async"));
    if (!transactionsURI.isValid())
        throw std::runtime_error("invalid uri");

    // Everything is read from the transaction now: it stays the caller's.
    QList<QPair<QString, int>> pending;
    if (!skipMemoRequiredCheck)
        pending = memoCheckedDestinations(transaction);
    QUrlQuery query;
    query.addQueryItem("tx", transaction->toEnvelopeXdrBase64());
    QNetworkRequest request = prepareRequest();
    request.setUrl(transactionsURI);
    const QByteArray body = escapeUri(query.toString()).toLatin1();

    auto* response = new SubmitTransactionAsyncResponse();
    if (pending.isEmpty()) {
        response->attachPost(m_httpClient->post(request, body));
        return response;
    }
    QPointer<SubmitTransactionAsyncResponse> guard(response);
    checkMemoDestinations(response, pending, 0,
        [this, guard, request, body]() { guard->attachPost(m_httpClient->post(request, body)); },
        [guard](qstellar::exception::SdkException* failure) { guard->emitError(failure); });
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

QSTELLAR_END_NS
