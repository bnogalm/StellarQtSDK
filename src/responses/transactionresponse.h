#ifndef TRANSACTIONRESPONSE_H
#define TRANSACTIONRESPONSE_H


#include "response.h"
#include "link.h"

#include "../util.h"
#include "../feebumptransaction.h"


#include "../keypair.h"
#include "../memo.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

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
    bool operator ==(const Links& links) const
    {
        return m_account == links.m_account
                && m_effects == links.m_effects
                && m_ledger == links.m_ledger
                && m_operations == links.m_operations
                && m_precedes == links.m_precedes
                && m_self == links.m_self
                && m_succeeds == links.m_succeeds;
    }
    bool operator !=(const Links& links) const
    {
        return !(*this == links);
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
    Q_PROPERTY(QStringList signatures MEMBER m_signatures)
    QString m_hash;
    QStringList m_signatures;
public:
    QString getHash() const
    {
        return m_hash;
    }
    QStringList getSignatures() const
    {
        return m_signatures;
    }
    bool operator ==(const FeeBumpTransaction& obj) const
    {
        return m_hash == obj.m_hash && m_signatures == obj.m_signatures;
    }
    bool operator !=(const FeeBumpTransaction& obj) const
    {
        return !(*this == obj);
    }
};


/**
 * CAP-21 — preconditions block exposed by Horizon for V2-wrapped transactions.
 * Numeric uint64 fields (min_time, max_time, min_account_sequence,
 * min_account_sequence_age) are kept as strings to preserve full precision.
 */
class TimeBounds {
    Q_GADGET
    Q_PROPERTY(QString min_time MEMBER m_minTime)
    Q_PROPERTY(QString max_time MEMBER m_maxTime)
    QString m_minTime;
    QString m_maxTime;
public:
    QString getMinTime() const { return m_minTime; }
    QString getMaxTime() const { return m_maxTime; }
    bool operator==(const TimeBounds& other) const
    {
        return m_minTime == other.m_minTime && m_maxTime == other.m_maxTime;
    }
    bool operator!=(const TimeBounds& other) const { return !(*this == other); }
};

class LedgerBounds {
    Q_GADGET
    Q_PROPERTY(quint32 min_ledger MEMBER m_minLedger)
    Q_PROPERTY(quint32 max_ledger MEMBER m_maxLedger)
    quint32 m_minLedger = 0;
    quint32 m_maxLedger = 0;
public:
    quint32 getMinLedger() const { return m_minLedger; }
    quint32 getMaxLedger() const { return m_maxLedger; }
    bool operator==(const LedgerBounds& other) const
    {
        return m_minLedger == other.m_minLedger && m_maxLedger == other.m_maxLedger;
    }
    bool operator!=(const LedgerBounds& other) const { return !(*this == other); }
};

class Preconditions {
    Q_GADGET
    Q_PROPERTY(TimeBounds timebounds MEMBER m_timeBounds)
    Q_PROPERTY(LedgerBounds ledgerbounds MEMBER m_ledgerBounds)
    Q_PROPERTY(QString min_account_sequence MEMBER m_minAccountSequence)
    Q_PROPERTY(QString min_account_sequence_age MEMBER m_minAccountSequenceAge)
    Q_PROPERTY(quint32 min_account_sequence_ledger_gap MEMBER m_minAccountSequenceLedgerGap)
    Q_PROPERTY(QStringList extra_signers MEMBER m_extraSigners)
    TimeBounds m_timeBounds;
    LedgerBounds m_ledgerBounds;
    QString m_minAccountSequence;
    QString m_minAccountSequenceAge;
    quint32 m_minAccountSequenceLedgerGap = 0;
    QStringList m_extraSigners;
public:
    const TimeBounds& getTimeBounds() const { return m_timeBounds; }
    const LedgerBounds& getLedgerBounds() const { return m_ledgerBounds; }
    QString getMinAccountSequence() const { return m_minAccountSequence; }
    QString getMinAccountSequenceAge() const { return m_minAccountSequenceAge; }
    quint32 getMinAccountSequenceLedgerGap() const { return m_minAccountSequenceLedgerGap; }
    QStringList getExtraSigners() const { return m_extraSigners; }
    bool operator==(const Preconditions& other) const
    {
        return m_timeBounds == other.m_timeBounds
            && m_ledgerBounds == other.m_ledgerBounds
            && m_minAccountSequence == other.m_minAccountSequence
            && m_minAccountSequenceAge == other.m_minAccountSequenceAge
            && m_minAccountSequenceLedgerGap == other.m_minAccountSequenceLedgerGap
            && m_extraSigners == other.m_extraSigners;
    }
    bool operator!=(const Preconditions& other) const { return !(*this == other); }
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
    Q_PROPERTY(QStringList signatures MEMBER m_signatures)
    Q_PROPERTY(qint64 max_fee MEMBER m_maxFee)
    QString m_hash;
    QStringList m_signatures;
    qint64 m_maxFee;
public:
    QString getHash() const
    {
        return m_hash;
    }
    QStringList getSignatures() const
    {
        return m_signatures;
    }
    qint64 getMaxFee() const
    {
        return m_maxFee;
    }
    bool operator ==(const InnerTransaction& obj) const
    {
        return m_hash == obj.m_hash
                && m_signatures == obj.m_signatures
                && m_maxFee == obj.m_maxFee;
    }
    bool operator !=(const InnerTransaction& obj) const
    {
        return !(*this == obj);
    }

};

}
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

    Q_PROPERTY(QStringList signatures MEMBER m_signatures)
    Q_PROPERTY(TransactionResponseAttach::FeeBumpTransaction fee_bump_transaction MEMBER m_feeBumpTransaction)
    Q_PROPERTY(TransactionResponseAttach::InnerTransaction  inner_transaction MEMBER m_innerTransaction)
    Q_PROPERTY(TransactionResponseAttach::Preconditions preconditions MEMBER m_preconditions)

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
    QStringList m_signatures;
    TransactionResponseAttach::FeeBumpTransaction m_feeBumpTransaction;
    TransactionResponseAttach::InnerTransaction m_innerTransaction;
    TransactionResponseAttach::Preconditions m_preconditions;


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
      QStringList getSignatures() const;

      TransactionResponseAttach::FeeBumpTransaction& getFeeBump();
      TransactionResponseAttach::InnerTransaction& getInner();
      const TransactionResponseAttach::Preconditions& getPreconditions() const { return m_preconditions; }

      QString sourceAccount() const;
      QString memoType() const;

      QByteArray memo() const;


      void setSourceAccount(QString sourceAccount);
      void setMemoType(QString memoType);
      void setMemo(QByteArray memoData);
      void setSuccessful(QVariant successful);
      void setFeeAccount(QString feeAccount);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TransactionResponse)



QSTELLAR_NS_ALIAS(TransactionResponseAttach)
Q_DECLARE_METATYPE(qstellar::TransactionResponseAttach::Links)
Q_DECLARE_METATYPE(qstellar::TransactionResponseAttach::FeeBumpTransaction)
Q_DECLARE_METATYPE(qstellar::TransactionResponseAttach::InnerTransaction)
Q_DECLARE_METATYPE(qstellar::TransactionResponseAttach::TimeBounds)
Q_DECLARE_METATYPE(qstellar::TransactionResponseAttach::LedgerBounds)
Q_DECLARE_METATYPE(qstellar::TransactionResponseAttach::Preconditions)
Q_DECLARE_METATYPE(qstellar::TransactionResponse*)
#endif // TRANSACTIONRESPONSE_H






