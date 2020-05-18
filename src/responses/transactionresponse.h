#ifndef TRANSACTIONRESPONSE_H
#define TRANSACTIONRESPONSE_H


#include "response.h"
#include "link.h"

#include "../util.h"
#include "../feebumptransaction.h"


namespace TransactionResponseAttach
{
/**
   * Links connected to transaction.
   */
class Links {
    Q_GADGET
    Q_PROPERTY(Link account MEMBER m_account)
    Q_PROPERTY(Link effects MEMBER m_effects)
    Q_PROPERTY(Link ledger MEMBER m_ledger)
    Q_PROPERTY(Link operations MEMBER m_operations)
    Q_PROPERTY(Link precedes MEMBER m_precedes)
    Q_PROPERTY(Link self MEMBER m_self)
    Q_PROPERTY(Link succeeds MEMBER m_succeeds)
    Link m_account;
    Link m_effects;
    Link m_ledger;
    Link m_operations;
    Link m_precedes;
    Link m_self;
    Link m_succeeds;
public:
    Link getAccount() {
        return m_account;
    }
    Link getEffects() {
        return m_effects;
    }
    Link getLedger() {
        return m_ledger;
    }
    Link getOperations() {
        return m_operations;
    }
    Link getPrecedes() {
        return m_precedes;
    }
    Link getSelf() {
        return m_self;
    }
    Link getSucceeds() {
        return m_succeeds;
    }
    bool operator !=(Links& links)
    {
        return m_account!= links.m_account
                || m_effects!= links.m_effects
                || m_ledger!= links.m_ledger
                || m_operations!= links.m_operations
                || m_precedes!= links.m_precedes
                || m_self!= links.m_self
                || m_succeeds!= links.m_succeeds;
    }
};

/**
 * FeeBumpTransaction is only present in a TransactionResponse if the transaction is a fee bump transaction or is
 * wrapped by a fee bump transaction. The object has two fields: the hash of the fee bump transaction and the
 * signatures present in the fee bump transaction envelope.
 */
class FeeBumpTransaction {
    Q_GADGET
    Q_PROPERTY(QString hash MEMBER m_hash)
    Q_PROPERTY(QList<QString> signatures MEMBER m_signatures)
    QString m_hash;
    QList<QString> m_signatures;
public:
    QString getHash() const
    {
        return m_hash;
    }
    QList<QString> getSignatures() const
    {
        return m_signatures;
    }
    bool operator !=(FeeBumpTransaction& obj)
    {
        return m_hash!=obj.m_hash || m_signatures != obj.m_signatures;
    }
};


/**
 * InnerTransaction is only present in a TransactionResponse if the transaction is a fee bump transaction or is
 * wrapped by a fee bump transaction. The object has three fields: the hash of the inner transaction wrapped by the
 * fee bump transaction, the max fee set in the inner transaction, and the signatures present in the inner
 * transaction envelope.
 */
class InnerTransaction {
    Q_GADGET
    Q_PROPERTY(QString hash MEMBER m_hash)
    Q_PROPERTY(QList<QString> signatures MEMBER m_signatures)
    Q_PROPERTY(qint64 max_fee MEMBER m_maxFee)
    QString m_hash;
    QList<QString> m_signatures;
    qint64 m_maxFee;
public:
    QString getHash() const
    {
        return m_hash;
    }
    QList<QString> getSignatures() const
    {
        return m_signatures;
    }
    qint64 getMaxFee() const
    {
        return m_maxFee;
    }
    bool operator !=(InnerTransaction& obj)
    {
        return m_hash!=obj.m_hash || m_signatures != obj.m_signatures || m_maxFee != obj.m_maxFee;
    }

};

}
class KeyPair;
class Memo;
/**
 * Represents transaction response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/transaction.html" target="_blank">Transaction documentation</a>
 * @see org.stellar.sdk.requests.TransactionsRequestBuilder
 * @see org.stellar.sdk.Server#transactions()
 */
class TransactionResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(QString hash MEMBER m_hash)
    Q_PROPERTY(qint64 ledger MEMBER m_ledger)
    Q_PROPERTY(QString created_at MEMBER m_createdAt)
    Q_PROPERTY(QString source_account READ sourceAccount WRITE setSourceAccount)
    Q_PROPERTY(QString fee_account MEMBER m_feeAccount)

    Q_PROPERTY(QVariant successful MEMBER m_successful WRITE setSuccessful) //we use a custom set method to filter non Bool types

    Q_PROPERTY(QString paging_token MEMBER m_pagingToken)
    Q_PROPERTY(qint64 source_account_sequence MEMBER m_sourceAccountSequence)    
    Q_PROPERTY(qint64 max_fee MEMBER m_maxFee)
    Q_PROPERTY(qint64 fee_charged MEMBER m_feeCharged)
    Q_PROPERTY(int operation_count MEMBER m_operationCount)
    Q_PROPERTY(QString envelope_xdr MEMBER m_envelopeXdr)
    Q_PROPERTY(QString result_xdr MEMBER m_resultXdr)
    Q_PROPERTY(QString result_meta_xdr MEMBER m_resultMetaXdr)

    Q_PROPERTY(QList<QString> signatures MEMBER m_signatures)
    Q_PROPERTY(TransactionResponseAttach::FeeBumpTransaction fee_bump_transaction MEMBER m_feeBumpTransaction)
    Q_PROPERTY(TransactionResponseAttach::InnerTransaction  inner_transaction MEMBER m_innerTransaction)

    Q_PROPERTY(QString memo_type READ memoType WRITE setMemoType)
    Q_PROPERTY(QByteArray memo READ memo WRITE setMemo)

    Q_PROPERTY(TransactionResponseAttach::Links _links MEMBER m_links)


    QString m_hash;
    qint64 m_ledger;
    QString m_createdAt;    
    QVariant m_successful;

    QString m_pagingToken;
    qint64 m_sourceAccountSequence;
    qint64 m_maxFee;
    qint64 m_feeCharged;
    int m_operationCount;
    QString m_envelopeXdr;
    QString m_resultXdr;
    QString m_resultMetaXdr;
    TransactionResponseAttach::Links m_links;
    QList<QString> m_signatures;
    TransactionResponseAttach::FeeBumpTransaction m_feeBumpTransaction;
    TransactionResponseAttach::InnerTransaction m_innerTransaction;


    QString m_sourceAccount;

    QString m_memoType;
    QByteArray m_memoData;
    Memo * m_memo;

    QString m_feeAccount;

public:
    Q_INVOKABLE explicit TransactionResponse(QNetworkReply *reply=nullptr);
    virtual ~TransactionResponse();
      QString getHash() const;

      qint64 getLedger() const;

      QString getCreatedAt() const;

      QString getSourceAccount() const;

      QString getPagingToken() const;

      Boolean isSuccessful() const;

      qint64 getSourceAccountSequence() const;

      qint64 getMaxFee() const;

      qint64 getFeeCharged() const;

      int getOperationCount() const;

      QString getEnvelopeXdr() const;

      QString getResultXdr() const;

      QString getResultMetaXdr() const;

      Memo* getMemo();

      TransactionResponseAttach::Links& getLinks();

      QString getFeeAccount() const;
      QList<QString> getSignatures() const;

      TransactionResponseAttach::FeeBumpTransaction& getFeeBump();
      TransactionResponseAttach::InnerTransaction& getInner();

      QString sourceAccount() const;
      QString memoType() const;

      QByteArray memo() const;


      void setSourceAccount(QString sourceAccount);
      void setMemoType(QString memoType);
      void setMemo(QByteArray memoData);
      void setSuccessful(QVariant successful);
      void setFeeAccount(QString feeAccount);
};
Q_DECLARE_METATYPE(TransactionResponseAttach::Links)
Q_DECLARE_METATYPE(TransactionResponseAttach::FeeBumpTransaction)
Q_DECLARE_METATYPE(TransactionResponseAttach::InnerTransaction )
Q_DECLARE_METATYPE(TransactionResponse*)
#endif // TRANSACTIONRESPONSE_H






