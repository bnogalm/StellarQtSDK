#include "claimclaimablebalanceoperation.h"

ClaimClaimableBalanceOperation::ClaimClaimableBalanceOperation(stellar::ClaimClaimableBalanceOp& op):m_op(op)
{

}

ClaimClaimableBalanceOperation::ClaimClaimableBalanceOperation(QString balanceID)
{
    checkNotNull(balanceID, "balanceID cannot be null");    
    Util::claimableBalanceIdToXDR(balanceID,m_op.balanceID);
}

void ClaimClaimableBalanceOperation::fillOperationBody(stellar::Operation &operation)
{
    operation.type = stellar::OperationType::CLAIM_CLAIMABLE_BALANCE;
    operation.operationClaimClaimableBalance = m_op;
}

QString ClaimClaimableBalanceOperation::getBalanceId() {
    return Util::xdrToClaimableBalanceId(m_op.balanceID);
}

ClaimClaimableBalanceOperation *ClaimClaimableBalanceOperation::build(stellar::ClaimClaimableBalanceOp &op)
{
    return new ClaimClaimableBalanceOperation(op);
}

ClaimClaimableBalanceOperation *ClaimClaimableBalanceOperation::create(QString balanceId)
{
    return new ClaimClaimableBalanceOperation(balanceId);
}

ClaimClaimableBalanceOperation *ClaimClaimableBalanceOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
