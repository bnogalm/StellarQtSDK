#ifndef FEEBUMPTRANSACTION_H
#define FEEBUMPTRANSACTION_H
#include "abstracttransaction.h"
#include "transaction.h"

class FeeBumpTransaction : public AbstractTransaction
{

    qint64 m_fee;
    QString m_feeAccount;
    Transaction* m_inner;


public:
    friend class Builder;
    FeeBumpTransaction(AccountConverter accountConverter, QString feeAccount, qint64 fee, Transaction* innerTransaction);
    virtual ~FeeBumpTransaction();
    qint64 getFee() const;
    QString getFeeAccount() const;

    Transaction* getInnerTransaction() const;

    static FeeBumpTransaction* fromFeeBumpTransactionEnvelope(AccountConverter accountConverter, stellar::FeeBumpTransactionEnvelope envelope, Network* network);
    static FeeBumpTransaction* fromFeeBumpTransactionEnvelope(stellar::FeeBumpTransactionEnvelope envelope, Network* network);
    stellar::FeeBumpTransaction toXdr() const;

    // AbstractTransaction interface
public:
    QByteArray signatureBase() const;
    stellar::TransactionEnvelope toEnvelopeXdr();


    /**
       * Builds a new FeeBumpTransaction object.
       */
      class Builder {
        Transaction* m_inner;
        qint64 m_baseFee;
        QString m_feeAccount;
        AccountConverter m_accountConverter;

      public:
        /**
         * Construct a new fee bump transaction builder.
         *
         * @param inner The inner transaction which will be fee bumped.
         */
        Builder(AccountConverter accountConverter, Transaction* inner);
        ~Builder();

        Builder& setBaseFee(qint64 baseFee);

        Builder& setFeeAccount(QString feeAccount);

        FeeBumpTransaction* build();

      };
};

#endif // FEEBUMPTRANSACTION_H
