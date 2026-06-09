#ifndef FEEBUMPTRANSACTION_H
#define FEEBUMPTRANSACTION_H
#include "abstracttransaction.h"
#include "transaction.h"
#include "stellardeprecated.h"
#include "feebumptransactionbuilder.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class FeeBumpTransaction : public AbstractTransaction
{

    qint64 m_fee;
    QString m_feeAccount;
    Transaction* m_inner;


public:
    friend class FeeBumpTransactionBuilder;        // qstellar::FeeBumpTransactionBuilder
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
     * @deprecated Use the free-standing ::FeeBumpTransactionBuilder class instead.
     * Kept as a type alias for source-compatibility during the 0.x series.
     * Will be removed in 1.0.0.
     */
    using Builder STELLAR_DEPRECATED("Use ::FeeBumpTransactionBuilder instead. Removed in 1.0.0") = FeeBumpTransactionBuilder;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(FeeBumpTransaction)

#endif // FEEBUMPTRANSACTION_H
