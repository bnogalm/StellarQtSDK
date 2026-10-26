#include "submittransactionresponse.h"
#include "xdr/stellartransaction.h"
#include "../abstracttransaction.h"
#include "../exception/connectionerrorexception.h"
#include <QCoreApplication>
#include "qtcompat.h"

QSTELLAR_BEGIN_NS

static void registerTypes()
{
    regType<SubmitTransactionResponseAttach::ResultCodes>();
    regType<SubmitTransactionResponseAttach::Extras>();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // ResultCodes is referenced by its bare class name inside Extras (same
    // namespace), so moc stores the unqualified name.
    qRegisterMetaType<SubmitTransactionResponseAttach::ResultCodes>("ResultCodes");
#endif
}
Q_COREAPP_STARTUP_FUNCTION(registerTypes)

SubmitTransactionResponse::SubmitTransactionResponse(QNetworkReply *reply,AbstractTransaction * transactionRequest):Response(reply)
  ,m_ledger(0)
  ,m_transactionRequest(transactionRequest)
  ,m_posted(reply != nullptr)
{
    connect(this,&Response::ready,this,&SubmitTransactionResponse::responseReady);
    // An error that arrives while a reply is attached came from the POST,
    // however the reply was attached (Response::loadFromReply, called through
    // a Response pointer, does not pass through the one below). Connected
    // first, so it is recorded before any receiver of error() asks.
    connect(this, &Response::error, this, [this]() {
        if (m_reply)
            m_posted = true;
    });
}

void SubmitTransactionResponse::loadFromReply(QNetworkReply *reply)
{
    if (reply)
        m_posted = true;
    Response::loadFromReply(reply);
}

SubmitTransactionResponse::~SubmitTransactionResponse()
{
    if(m_transactionRequest)
        delete m_transactionRequest;
}

AbstractTransaction *SubmitTransactionResponse::transactionRequest() const
{
    return m_transactionRequest;
}

void SubmitTransactionResponse::readFailureBody(int, const QJsonObject& body)
{
    const QJsonValue extras = body.value(QStringLiteral("extras"));
    if (!extras.isObject())
        return;
    SubmitTransactionResponseAttach::Extras read;
    // Exactly, as a success is read; an unreadable member leaves the extras
    // empty rather than half filled.
    fillObject(&SubmitTransactionResponseAttach::Extras::staticMetaObject, &read, extras.toObject());
    m_extra = read;
}

QString SubmitTransactionResponse::getHash() const
{
    return m_hash;
}

qint64 SubmitTransactionResponse::getLedger() const
{
    return m_ledger;
}

QString SubmitTransactionResponse::getResultXdr()
{
    if (this->isSuccess()) {
        return this->m_result_xdr;
    } else {
        return this->getExtras().getResultXdr();
    }
}

SubmitTransactionResponseAttach::Extras &SubmitTransactionResponse::getExtras()
{
    return m_extra;
}

bool SubmitTransactionResponse::isTimeout() const
{
    // Posted: the transaction may land. A SEP-29 lookup that timed out stops
    // the submission before that, and nothing can. This asked m_reply, which
    // said so only while the reply lived.
    return getStatus() == 504
           || (m_posted && dynamic_cast<qstellar::exception::ClientTimeoutException*>(lastException()));
}

bool SubmitTransactionResponse::isSuccess() const
{
    return m_ledger != 0;
}

QString SubmitTransactionResponse::getEnvelopeXdr() {
    if (this->isSuccess()) {
        return this->m_envelope_xdr;
    } else {
        return this->getExtras().getTransactionResultCode();
    }
}

qint64 SubmitTransactionResponse::getOfferIdFromResult(int position) {
    if (!this->isSuccess()) {
        return 0;
    }

    // Base64 exactly: the lenient decoder skipped what it could not read,
    // and a result with characters that are not base64 read as other bytes.
    const QByteArray encoded = this->getResultXdr().toLatin1();
    const QByteArray::FromBase64Result decodedResult =
        QByteArray::fromBase64Encoding(encoded, XDR_BASE64ENCODING | QByteArray::AbortOnBase64DecodingErrors);
    if (decodedResult.decodingStatus != QByteArray::Base64DecodingStatus::Ok)
        return 0;
    QByteArray data = decodedResult.decoded;
    QDataStream stream(&data,QIODevice::ReadOnly);
    stellar::TransactionResult result;
    // 0 for anything unreadable, exceptions included: a 20-byte result_xdr
    // used to throw std::bad_alloc out of here, and nothing above catches it.
    try {
        stream >> result;
    } catch (...) {
        return 0;
    }
    // All of it or nothing: a result the reader could not follow used to hand
    // back 0, or another seller's offer id, as if it were this one's.
    if (stream.status() != QDataStream::Ok || !stream.atEnd()) {
        return 0;
    }

    // A fee bump carries the operations' results in its inner result.
    const bool feeBump = result.code == stellar::TransactionResultCode::txFEE_BUMP_INNER_SUCCESS
                         || result.code == stellar::TransactionResultCode::txFEE_BUMP_INNER_FAILED;
    const QVector<stellar::OperationResult>& results =
        feeBump ? result.innerResultPair.result.results : result.results;
    if (position < 0 || results.length()<=position) {
        return 0;
    }
    auto& operationResult = results.at(position);
    if (operationResult.code != stellar::OperationResultCode::opINNER) {
        return 0;
    }
    auto operationType = operationResult.type;

    if (operationType == stellar::OperationType::CREATE_PASSIVE_SELL_OFFER) {
        if (operationResult.createPassiveOfferResult.code
                != stellar::ManageSellOfferResultCode::MANAGE_SELL_OFFER_SUCCESS)
            return 0;
        auto& effect = operationResult.createPassiveOfferResult.success.effect;
        if (effect == stellar::ManageOfferEffect::MANAGE_OFFER_CREATED
                || effect == stellar::ManageOfferEffect::MANAGE_OFFER_UPDATED)
            return operationResult.createPassiveOfferResult.success.offer.offerID;
        return 0;
    }
    if (operationType == stellar::OperationType::MANAGE_SELL_OFFER
            && operationResult.manageSellOfferResult.code
               != stellar::ManageSellOfferResultCode::MANAGE_SELL_OFFER_SUCCESS) {
        return 0;
    }
    if (operationType == stellar::OperationType::MANAGE_BUY_OFFER
            && operationResult.manageBuyOfferResult.code
               != stellar::ManageBuyOfferResultCode::MANAGE_BUY_OFFER_SUCCESS) {
        return 0;
    }

    if (operationType == stellar::OperationType::MANAGE_SELL_OFFER) {
        auto& effect = operationResult.manageSellOfferResult.success.effect;
        if (effect == stellar::ManageOfferEffect::MANAGE_OFFER_CREATED
                ||effect == stellar::ManageOfferEffect::MANAGE_OFFER_UPDATED) {
            return operationResult.manageSellOfferResult.success.offer.offerID;
        }
    }
    if (operationType == stellar::OperationType::MANAGE_BUY_OFFER) {
        auto& effect = operationResult.manageBuyOfferResult.success.effect;
        if (effect == stellar::ManageOfferEffect::MANAGE_OFFER_CREATED
                ||effect == stellar::ManageOfferEffect::MANAGE_OFFER_UPDATED) {
            return operationResult.manageBuyOfferResult.success.offer.offerID;
        }
    }
    return 0;
}
QSTELLAR_END_NS
