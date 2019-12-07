#ifndef TRANSACTIONRESPONSE_H
#define TRANSACTIONRESPONSE_H


#include "response.h"
#include "link.h"

#include "../util.h"

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
        Q_UNUSED(links)
        return true;
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

    Q_PROPERTY(QVariant successful MEMBER m_successful WRITE setSuccessful) //we use a custom set method to filter non Bool types

    Q_PROPERTY(QString paging_token MEMBER m_pagingToken)
    Q_PROPERTY(qint64 source_account_sequence MEMBER m_sourceAccountSequence)    
    Q_PROPERTY(qint64 max_fee MEMBER m_maxFee)
    Q_PROPERTY(qint64 fee_charged MEMBER m_feeCharged)
    Q_PROPERTY(int operation_count MEMBER m_operationCount)
    Q_PROPERTY(QString envelope_xdr MEMBER m_envelopeXdr)
    Q_PROPERTY(QString result_xdr MEMBER m_resultXdr)
    Q_PROPERTY(QString result_meta_xdr MEMBER m_resultMetaXdr)

    Q_PROPERTY(QString memo_type READ memoType WRITE setMemoType)
    Q_PROPERTY(QByteArray memo READ memo WRITE setMemo)

    Q_PROPERTY(TransactionResponseAttach::Links _links MEMBER m_links)


    QString m_hash;
    qint64 m_ledger;
    QString m_createdAt;

    KeyPair* m_sourceAccountKeypair;
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


    QString m_sourceAccount;

    QString m_memoType;
    QByteArray m_memoData;
    Memo * m_memo;

public:
    Q_INVOKABLE explicit TransactionResponse(QNetworkReply *reply=nullptr);
    virtual ~TransactionResponse();
      QString getHash() const;

      qint64 getLedger() const;

      QString getCreatedAt() const;

      KeyPair* getSourceAccount();

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

      QString sourceAccount() const;
      QString memoType() const;

      QByteArray memo() const;

public slots:
      void setSourceAccount(QString sourceAccount);
      void setMemoType(QString memoType);
      void setMemo(QByteArray memoData);
      void setSuccessful(QVariant successful);
};
Q_DECLARE_METATYPE(TransactionResponseAttach::Links)
Q_DECLARE_METATYPE(TransactionResponse*)
#endif // TRANSACTIONRESPONSE_H






