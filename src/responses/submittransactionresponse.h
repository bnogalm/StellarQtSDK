#ifndef SUBMITTRANSACTIONRESPONSE_H
#define SUBMITTRANSACTIONRESPONSE_H

#include "response.h"
namespace SubmitTransactionResponseAttach
{
/**
 * Contains result codes for this transaction.
 * @see <a href="https://github.com/stellar/horizon/blob/master/src/github.com/stellar/horizon/codes/main.go" target="_blank">Possible values</a>
 */
class ResultCodes {
    Q_GADGET
    Q_PROPERTY(QString transaction MEMBER m_transaction)
    Q_PROPERTY(QStringList operations MEMBER m_operations)
    QString m_transaction;
    QStringList m_operations;
public:
    QString getTransactionResultCode() {
        return m_transaction;
    }

    QStringList getOperationsResultCodes() {
        return m_operations;
    }
    bool operator !=(ResultCodes& rc)
    {
        Q_UNUSED(rc)
        return true;
    }
};


    /**
     * Additional information returned by a server.
     */
class Extras
{
    Q_GADGET

    Q_PROPERTY(QString envelope_xdr MEMBER m_envelopeXdr)
    Q_PROPERTY(QString result_xdr MEMBER m_resultXdr)
    Q_PROPERTY(SubmitTransactionResponseAttach::ResultCodes result_codes MEMBER m_resultCodes)
    QString m_envelopeXdr;
    QString m_resultXdr;
    ResultCodes m_resultCodes;
public:

    /**
    * Returns XDR TransactionEnvelope base64-encoded string.
    * Use <a href="http://stellar.github.io/xdr-viewer/">xdr-viewer</a> to debug.
    */
    QString getTransactionResultCode() const
    {
        return m_envelopeXdr;
    }
    /**
    * Returns XDR TransactionResult base64-encoded string
    * Use <a href="http://stellar.github.io/xdr-viewer/">xdr-viewer</a> to debug.
    */
    QString getResultXdr() const
    {
        return m_resultXdr;
    }
    /**
    * Returns ResultCodes object that contains result codes for transaction.
    */
    ResultCodes& getResultCodes() {
        return m_resultCodes;
    }

    bool operator !=(Extras& rc)
    {
        Q_UNUSED(rc)
        return true;
    }
};


}
class Transaction;
class SubmitTransactionResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(QString hash READ getHash MEMBER m_hash NOTIFY responseReady)
    Q_PROPERTY(qint64 ledger READ getLedger MEMBER m_ledger NOTIFY responseReady)
    Q_PROPERTY(QString envelope_xdr READ getEnvelopeXdr MEMBER m_envelope_xdr NOTIFY responseReady)
    Q_PROPERTY(QString result_xdr READ getResultXdr MEMBER m_result_xdr NOTIFY responseReady)
    Q_PROPERTY(SubmitTransactionResponseAttach::Extras extras READ getExtras MEMBER m_extra NOTIFY responseReady)

    QString m_hash;

    qint64 m_ledger;

    QString m_result_xdr;

    SubmitTransactionResponseAttach::Extras m_extra;

    QString m_envelope_xdr;
    Transaction *m_transactionRequest;
public:
    explicit SubmitTransactionResponse(QNetworkReply* reply,Transaction * transactionRequest=nullptr);
    virtual ~SubmitTransactionResponse();
    Transaction * transactionRequest() const;
    QString getHash() const;

    qint64 getLedger() const;

    QString getResultXdr();

    SubmitTransactionResponseAttach::Extras& getExtras();
    bool isSuccess() const;
    QString getEnvelopeXdr();
    /**
        * Helper method that returns Offer ID for ManageSellOffer/ManageBuyOffer from TransactionResult Xdr.
        * This is helpful when you need ID of an offer to update it later.
        * @param position Position of ManageSellOffer/ManageBuyOffer operation. If it is second operation in this transaction this should be equal <code>1</code>.
        * @return Offer ID or <code>null</code> when operation at <code>position</code> is not a ManageSellOffer/ManageBuyOffer operation or error has occurred.
        */
    qint64 getOfferIdFromResult(int position);
signals:
    void responseReady();
};


Q_DECLARE_METATYPE(SubmitTransactionResponseAttach::ResultCodes)
Q_DECLARE_METATYPE(SubmitTransactionResponseAttach::Extras)

#endif // SUBMITTRANSACTIONRESPONSE_H
