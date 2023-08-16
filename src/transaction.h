#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QtGlobal>

#include "xdr/stellartransaction.h"

#include <exception>
#include "transactionbuilderaccount.h"
#include "timebounds.h"
#include "abstracttransaction.h"

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
    TimeBounds *m_timeBounds;

    stellar::EnvelopeType m_envelopeType;
    friend class Builder;
    friend class FeeBumpTransaction;
    Transaction(QString sourceAccount, qint64 fee, qint64 sequenceNumber, QVector<Operation*> operations, Memo* memo, TimeBounds *timeBounds, Network* network);


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
     stellar::TransactionV0 toV0Xdr() const;

     /**
      * Generates Transaction XDR v0 object.
      */
     stellar::Transaction toV1Xdr() const;

     /**
      * Returns new Transaction object from Transaction XDR object.
      * @param xdr XDR object
      */
     //static Transaction* fromXdr(stellar::Transaction& xdr, Network* network);


     static Transaction* fromV0EnvelopeXdr(stellar::TransactionV0Envelope& envelope, Network* network);
     static Transaction* fromV1EnvelopeXdr(stellar::TransactionV1Envelope& envelope, Network* network);

     /**
      * Generates TransactionEnvelope XDR object.
      */
     stellar::TransactionEnvelope toEnvelopeXdr();


     /**
      * Builds a new Transaction object.
      */
     class Builder {

         TransactionBuilderAccount *m_sourceAccount;
         Network* m_network;
         Memo *m_memo;
         TimeBounds *m_timeBounds;
         QVector<Operation*> m_operations;
         bool m_timeoutSet;

         quint32 m_baseFee;
         static quint32 s_defaultOperationFee;
        void clear();
     public:
        static const quint32 BASE_FEE = 100;
        static const qint64 TIMEOUT_INFINITE = 0;
         /**
        * Construct a new transaction builder.
        * @param sourceAccount The source account for this transaction. This account is the account
        * who will use a sequence number. When build() is called, the account object's sequence number
        * will be incremented.        
        */
         Builder(TransactionBuilderAccount *sourceAccount, Network* network= Network::current());
        ~Builder();

         int getOperationsCount();

         static void setDefaultOperationFee(quint32 opFee);

         /**
        * Adds a new <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">operation</a> to this transaction.
        * @param operation, you lose the ownership of the object, don't delete it.
        * @return Builder object so you can chain methods.
        * @see Operation
        */
         Builder& addOperation(Operation* operation);

         /**
        * Adds a <a href="https://www.stellar.org/developers/learn/concepts/transactions.html" target="_blank">memo</a> to this transaction.
        * @param memo, you lose the ownership of the object, don't delete it.
        * @return Builder object so you can chain methods.
        * @see Memo
        */
         Builder& addMemo(Memo* memo);

         /**
          * Adds a <a href="https://www.stellar.org/developers/learn/concepts/transactions.html" target="_blank">time-bounds</a> to this transaction.
          * @param timeBounds
          * @return Builder object so you can chain methods.
          * @see TimeBounds
          */
         Builder& addTimeBounds(TimeBounds* timeBounds);

         /**
          * Because of the distributed nature of the Stellar network it is possible that the status of your transaction
          * will be determined after a long time if the network is highly congested.
          * If you want to be sure to receive the status of the transaction within a given period you should set the
          * {@link TimeBounds} with <code>maxTime</code> on the transaction (this is what <code>setTimeout</code> does
          * internally; if there's <code>minTime</code> set but no <code>maxTime</code> it will be added).
          * Call to <code>Builder.setTimeout</code> is required if Transaction does not have <code>max_time</code> set.
          * If you don't want to set timeout, use <code>TIMEOUT_INFINITE</code>. In general you should set
          * <code>TIMEOUT_INFINITE</code> only in smart contracts.
          * Please note that Horizon may still return <code>504 Gateway Timeout</code> error, even for short timeouts.
          * In such case you need to resubmit the same transaction again without making any changes to receive a status.
          * This method is using the machine system time (UTC), make sure it is set correctly.
          * @param timeout Timeout in seconds.
          * @see TimeBounds
          * @return
          */
         Builder& setTimeout(qint64 timeout);

         Builder& setBaseFee(quint32 baseFee);

         /**
        * Builds a transaction. It will increment sequence number of the source account.
        * You take ownership of the object so delete it when you don't need anymore.
        */
         Transaction* build();
     };
};
Transaction* checkNotNull(Transaction* transaction, const char *error);



#endif // TRANSACTION_H
