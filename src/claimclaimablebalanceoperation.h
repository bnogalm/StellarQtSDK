#ifndef CLAIMCLAIMABLEBALANCEOPERATION_H
#define CLAIMCLAIMABLEBALANCEOPERATION_H

#include "operation.h"

class ClaimClaimableBalanceOperation : public Operation
{
    stellar::ClaimClaimableBalanceOp m_op;
public:
    ClaimClaimableBalanceOperation(stellar::ClaimClaimableBalanceOp &op);
    ClaimClaimableBalanceOperation(QString balanceID);

    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);
    QString getBalanceId();
    static ClaimClaimableBalanceOperation* build(stellar::ClaimClaimableBalanceOp &op);
    static ClaimClaimableBalanceOperation* create(QString balanceId);
    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return ClaimClaimableBalanceOperation object so you can chain methods.
         */
    ClaimClaimableBalanceOperation* setSourceAccount(QString sourceAccount);
};

#endif // CLAIMCLAIMABLEBALANCEOPERATION_H
