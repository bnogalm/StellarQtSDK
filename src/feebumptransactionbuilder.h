#ifndef FEEBUMPTRANSACTIONBUILDER_H
#define FEEBUMPTRANSACTIONBUILDER_H

#include <QtGlobal>
#include <QString>
#include "accountconverter.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class Transaction;
class FeeBumpTransaction;

/**
 * @brief Free FeeBumpTransactionBuilder class (introduced in 0.4.0).
 *
 * Equivalent to the legacy `FeeBumpTransaction::Builder` nested class —
 * which now lives on as a deprecated `using` alias for source compatibility.
 *
 * Takes ownership of the `inner` Transaction (for V0 envelopes, an internal
 * deep-clone is made via XDR round-trip so the caller's `inner` stays
 * usable — see `feebumptransaction.cpp` §1.2 fix).
 */
class FeeBumpTransactionBuilder
{
    Transaction* m_inner;
    qint64 m_baseFee;
    QString m_feeAccount;
    AccountConverter m_accountConverter;

public:
    /**
     * @param inner the inner transaction to be fee-bumped.
     */
    FeeBumpTransactionBuilder(AccountConverter accountConverter, Transaction* inner);
    ~FeeBumpTransactionBuilder();

    FeeBumpTransactionBuilder& setBaseFee(qint64 baseFee);
    FeeBumpTransactionBuilder& setFeeAccount(QString feeAccount);

    /**
     * Build the FeeBumpTransaction. Caller takes ownership of the result.
     */
    FeeBumpTransaction* build();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(FeeBumpTransactionBuilder)

#endif // FEEBUMPTRANSACTIONBUILDER_H
