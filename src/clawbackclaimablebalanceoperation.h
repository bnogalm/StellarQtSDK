#ifndef CLAWBACKCLAIMABLEBALANCEOPERATION_H
#define CLAWBACKCLAIMABLEBALANCEOPERATION_H

#include "operation.h"
#include "qstellar_namespace.h"
/**
 *
 * Represents a Clawback Claimable Balance operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
QSTELLAR_BEGIN_NS

class ClawbackClaimableBalanceOperation : public Operation
{

    stellar::ClawbackClaimableBalanceOp m_op;
public:
    ClawbackClaimableBalanceOperation(stellar::ClawbackClaimableBalanceOp &op);
    ClawbackClaimableBalanceOperation(QString balanceID);
    virtual ~ClawbackClaimableBalanceOperation();
    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);
    /**
     * The id of the claimable balance which will be clawed back.
     */
    QString getBalanceId();

    static ClawbackClaimableBalanceOperation* build(stellar::ClawbackClaimableBalanceOp &op);
    static ClawbackClaimableBalanceOperation* create(QString balanceId);
    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return ClawbackClaimableBalanceOp object so you can chain methods.
         */
    ClawbackClaimableBalanceOperation* setSourceAccount(QString sourceAccount);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(ClawbackClaimableBalanceOperation)

#endif // CLAWBACKCLAIMABLEBALANCEOPERATION_H
