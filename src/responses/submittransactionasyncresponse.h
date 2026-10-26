#ifndef SUBMITTRANSACTIONASYNCRESPONSE_H
#define SUBMITTRANSACTIONASYNCRESPONSE_H

#include "response.h"
#include "../qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * What Server::submitTransactionAsync() answers: Horizon's
 * POST /transactions_async, which hands the envelope to stellar-core and
 * returns core's verdict at once, without waiting for a ledger (js's
 * submitAsyncTransaction, py's submit_transaction_async).
 *
 * ready() comes with HTTP 201 and getTxStatus() "PENDING": core took the
 * transaction; it is not applied yet, so look it up by getHash()
 * (transactions().transaction()) for its result. Horizon sends the other
 * verdicts with an HTTP error, and error() is emitted for them, as js
 * rejects and py raises, but their body is read all the same:
 *  - "DUPLICATE" (409): core already has this transaction;
 *  - "ERROR" (400): core refused it; getErrorResultXdr() is the base64
 *    TransactionResult saying why;
 *  - "TRY_AGAIN_LATER" (503): core is busy; send the same envelope again.
 * lastException() is then a BadRequestException (400, 409) or a
 * BadResponseException (503) carrying the status and the body. Any other
 * failure (a problem document, such as a malformed envelope's 400, a
 * timeout, the connection, a SEP-29 refusal) leaves getTxStatus() empty.
 */
class SubmitTransactionAsyncResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(QString tx_status READ getTxStatus MEMBER m_txStatus NOTIFY responseReady)
    Q_PROPERTY(QString hash READ getHash MEMBER m_hash NOTIFY responseReady)
    Q_PROPERTY(QString error_result_xdr READ getErrorResultXdr MEMBER m_errorResultXdr NOTIFY responseReady)

    QString m_txStatus;
    QString m_hash;
    QString m_errorResultXdr;

    // Set once the POST of the envelope is attached: from then on core may
    // have the transaction (see isTimeout()).
    bool m_posted;

    // Server posts the envelope (attachPost) once SEP-29 let it go, or
    // reports why not through Response::emitError.
    friend class Server;
    void attachPost(QNetworkReply* reply);

public:
    explicit SubmitTransactionAsyncResponse(QNetworkReply* reply = nullptr);

    /** "PENDING", "DUPLICATE", "ERROR" or "TRY_AGAIN_LATER"; empty when Horizon gave no verdict. */
    QString getTxStatus() const;
    /** The transaction's hash, hex, as Horizon reports it with the verdict. */
    QString getHash() const;
    /** With "ERROR": the base64 XDR TransactionResult core refused it with; empty otherwise. */
    QString getErrorResultXdr() const;
    /**
     * True when Horizon answered 504 or the SDK gave up waiting once the
     * envelope was posted (a ClientTimeoutException): core may have the
     * transaction. Look it up by hash, or send the SAME envelope again;
     * never rebuild it with a new sequence number before knowing.
     */
    bool isTimeout() const;

    /**
     * Fills the response from Horizon's 201 answer, exactly; throws
     * std::runtime_error when tx_status or hash is missing, which error()
     * then reports as an UnknownResponseException.
     */
    void loadFromJson(QByteArray data) override;

protected:
    /** Reads the verdict Horizon sends with a 400, 409 or 503 (see the class comment). */
    void readFailureBody(int httpStatus, const QJsonObject& body) override;

signals:
    void responseReady();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(SubmitTransactionAsyncResponse)

#endif // SUBMITTRANSACTIONASYNCRESPONSE_H
