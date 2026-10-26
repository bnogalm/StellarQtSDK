#include "submittransactionasyncresponse.h"
#include "../exception/connectionerrorexception.h"

#include <stdexcept>

QSTELLAR_BEGIN_NS

SubmitTransactionAsyncResponse::SubmitTransactionAsyncResponse(QNetworkReply *reply)
    : Response(reply)
    , m_posted(reply != nullptr)
{
    connect(this, &Response::ready, this, &SubmitTransactionAsyncResponse::responseReady);
}

void SubmitTransactionAsyncResponse::attachPost(QNetworkReply *reply)
{
    if (reply)
        m_posted = true;
    loadFromReply(reply);
}

QString SubmitTransactionAsyncResponse::getTxStatus() const
{
    return m_txStatus;
}

QString SubmitTransactionAsyncResponse::getHash() const
{
    return m_hash;
}

QString SubmitTransactionAsyncResponse::getErrorResultXdr() const
{
    return m_errorResultXdr;
}

bool SubmitTransactionAsyncResponse::isTimeout() const
{
    return getStatus() == 504
           || (m_posted && dynamic_cast<qstellar::exception::ClientTimeoutException*>(lastException()));
}

void SubmitTransactionAsyncResponse::loadFromJson(QByteArray data)
{
    Response::loadFromJson(data);
    if (m_txStatus.isEmpty() || m_hash.isEmpty())
        throw std::runtime_error("transactions_async answer without tx_status or hash");
}

void SubmitTransactionAsyncResponse::readFailureBody(int, const QJsonObject& body)
{
    // A verdict is the 201's object with another tx_status; a problem
    // document (a malformed envelope, a server error) has none.
    if (!body.value(QStringLiteral("tx_status")).isString())
        return;
    m_txStatus = body.value(QStringLiteral("tx_status")).toString();
    m_hash = body.value(QStringLiteral("hash")).toString();
    m_errorResultXdr = body.value(QStringLiteral("error_result_xdr")).toString();
}

QSTELLAR_END_NS
