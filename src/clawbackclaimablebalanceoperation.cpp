#include "clawbackclaimablebalanceoperation.h"

ClawbackClaimableBalanceOperation::ClawbackClaimableBalanceOperation(stellar::ClawbackClaimableBalanceOp &op):m_op(op)
{

}

ClawbackClaimableBalanceOperation::ClawbackClaimableBalanceOperation(QString balanceID)
{
    checkNotNull(balanceID, "balanceID cannot be null");    
    Util::claimableBalanceIdToXDR(balanceID,m_op.balanceID);
}

void ClawbackClaimableBalanceOperation::fillOperationBody(stellar::Operation &operation)
{
    operation.type = stellar::OperationType::CLAWBACK_CLAIMABLE_BALANCE;
    operation.operationClawbackClaimableBalance = m_op;
}

QString ClawbackClaimableBalanceOperation::getBalanceId()
{
    return Util::xdrToClaimableBalanceId(m_op.balanceID);
}

ClawbackClaimableBalanceOperation *ClawbackClaimableBalanceOperation::build(stellar::ClawbackClaimableBalanceOp &op)
{
    return new ClawbackClaimableBalanceOperation(op);
}

ClawbackClaimableBalanceOperation *ClawbackClaimableBalanceOperation::create(QString balanceId)
{
    return new ClawbackClaimableBalanceOperation(balanceId);
}

ClawbackClaimableBalanceOperation *ClawbackClaimableBalanceOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}

