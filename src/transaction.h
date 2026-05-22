#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QtGlobal>

#include "xdr/stellartransaction.h"

#include <QSharedPointer>
#include <exception>
#include "transactionbuilderaccount.h"
#include "timebounds.h"
#include "transactionpreconditions.h"
#include "abstracttransaction.h"
#include "stellardeprecated.h"
#include "transactionbuilder.h"

class FeeBumpTransaction;
/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/transactions.html" target="_blank">Transaction</a> in Stellar network.
 */
class Transaction : public AbstractTransaction
{
    qint64 m_fee;
    QString m_sourceAccount;
    qint64 m_sequenceNumber;
    QVector<Operation*> m_operations;
    Memo *m_memo;
    TransactionPreconditions m_preconditions;
    // CAP-46 — populated by the builder when setSorobanData() was called.
    QSharedPointer<stellar::SorobanTransactionData> m_sorobanData;

    stellar::EnvelopeType m_envelopeType;
    friend class ::TransactionBuilder;
    friend class FeeBumpTransaction;
    /** Legacy ctor — only time bounds. The TimeBounds* is consumed (TransactionPreconditions takes ownership). */
    Transaction(AccountConverter accountConverter, QString sourceAccount, qint64 fee, qint64 sequenceNumber, QVector<Operation*> operations, Memo* memo, TimeBounds *timeBounds, Network* network);
    /** CAP-21 ctor — full preconditions bundle. Pass via std::move to preserve pointer identity. */
    Transaction(AccountConverter accountConverter, QString sourceAccount, qint64 fee, qint64 sequenceNumber, QVector<Operation*> operations, Memo* memo, TransactionPreconditions preconditions, Network* network);


public:
    virtual ~Transaction();

#ifdef STELLAR_ENABLE_TEST_METHODS
    // setEnvelopeType is only used in tests which is why this method is package protected
    void setEnvelopeType(stellar::EnvelopeType envelopeType) {
        m_envelopeType = envelopeType;
    }
    stellar::EnvelopeType envelopeType() const
    {
        return m_envelopeType;
    }
#endif
    QByteArray signatureBase() const;
    QString getSourceAccount() const;
    qint64 getSequenceNumber() const;
    Memo* getMemo() const;

    Network* getNetwork() const;

     /**
      * @return TimeBounds, or null (representing no time restrictions)
      */
     TimeBounds* getTimeBounds() const;

     /** Full CAP-21 preconditions bundle (covers TimeBounds plus all V2 fields). */
     const TransactionPreconditions& getPreconditions() const { return m_preconditions; }

     /** CAP-46 — Soroban transaction data if set (e.g. when this transaction
      *  contains an Invoke / Extend / Restore op). Null otherwise. */
     QSharedPointer<stellar::SorobanTransactionData> getSorobanData() const { return m_sorobanData; }

     /**
      * Returns operations in this transaction.
      */
     QVector<Operation*> getOperations() const;

     /**
      * Returns fee paid for transaction in stroops (1 stroop = 0.0000001 XLM).
      */
     qint64 getFee() const;

     /**
      * Returns the claimable balance ID for the CreateClaimableBalanceOperation at the given index within the transaction.
      */
     QString getClaimableBalanceId(int index);


     /**
      * Generates Transaction XDR v0 object.
      */
     stellar::TransactionV0 toV0Xdr(AccountConverter accountConverter = AccountConverter().enableMuxed()) const;

     /**
      * Generates Transaction XDR v0 object.
      */
     stellar::Transaction toV1Xdr(AccountConverter accountConverter = AccountConverter().enableMuxed()) const;

     /**
      * Returns new Transaction object from Transaction XDR object.
      * @param xdr XDR object
      */
     //static Transaction* fromXdr(stellar::Transaction& xdr, Network* network);


     static Transaction* fromV0EnvelopeXdr(stellar::TransactionV0Envelope& envelope, Network* network);
     static Transaction* fromV1EnvelopeXdr(stellar::TransactionV1Envelope& envelope, Network* network);

     static Transaction* fromV0EnvelopeXdr(AccountConverter accountConverter, stellar::TransactionV0Envelope& envelope, Network* network);
     static Transaction* fromV1EnvelopeXdr(AccountConverter accountConverter, stellar::TransactionV1Envelope& envelope, Network* network);



     /**
      * Generates TransactionEnvelope XDR object.
      */
     stellar::TransactionEnvelope toEnvelopeXdr();


     /**
      * @deprecated Use the free-standing ::TransactionBuilder class instead.
      * `Transaction::Builder` is kept as a type alias for source-compatibility
      * during the 0.x series and will be removed in 1.0.0.
      *
      * The alias is functionally identical — all the existing builder code
      * (`Transaction::Builder b(...); b.addOperation(...).build();`) keeps
      * compiling and behaving as before.
      */
     using Builder STELLAR_DEPRECATED("Use ::TransactionBuilder instead. Removed in 1.0.0") = ::TransactionBuilder;
};
Transaction* checkNotNull(Transaction* transaction, const char *error);



#endif // TRANSACTION_H
