#include "claimclaimablebalanceoperation.h"

ClaimClaimableBalanceOperation::ClaimClaimableBalanceOperation(stellar::ClaimClaimableBalanceOp& op):m_op(op)
{

}

ClaimClaimableBalanceOperation::ClaimClaimableBalanceOperation(QString balanceID)
{
    checkNotNull(balanceID, "balanceID cannot be null");
    m_op.balanceID.type = stellar::ClaimableBalanceIDType::CLAIMABLE_BALANCE_ID_TYPE_V0;
    QByteArray balanceIDDecoded = QByteArray::fromHex(balanceID.toLower().toLatin1());
    if(balanceIDDecoded.length()!=sizeof(m_op.balanceID.v0))
        throw std::runtime_error("invalid balance id length");
    memcpy(balanceIDDecoded.data(),m_op.balanceID.v0,sizeof(m_op.balanceID.v0));
}

void ClaimClaimableBalanceOperation::fillOperationBody(stellar::Operation &operation)
{
    operation.type = stellar::OperationType::CLAIM_CLAIMABLE_BALANCE;
    operation.operationClaimClaimableBalance = m_op;
}

QString ClaimClaimableBalanceOperation::getBalanceId() {
    return QString(QByteArray::fromRawData((char*)m_op.balanceID.v0,sizeof(m_op.balanceID.v0)).toHex());
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
