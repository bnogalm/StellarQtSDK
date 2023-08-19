#include "createclaimablebalanceoperation.h"

CreateClaimableBalanceOperation::CreateClaimableBalanceOperation(QString amount, Asset *asset, QList<Claimant> claimants):m_asset(nullptr)
{
    checkNotNull(amount, "amount cannot be null");
    checkNotNull(asset, "asset cannot be null");
    checkNotNull(claimants, "claimants cannot be null");
    m_op.amount = Operation::toXdrAmount(amount);
    m_op.asset = asset->toXdr();
    m_op.claimants.clear();
    for(const auto& c:claimants)
        m_op.claimants.append(c.toXdr());

}

CreateClaimableBalanceOperation::CreateClaimableBalanceOperation(stellar::CreateClaimableBalanceOp &op):m_op(op)
{

}

CreateClaimableBalanceOperation::~CreateClaimableBalanceOperation()
{
    if(m_asset)
        delete m_asset;
}

Asset *CreateClaimableBalanceOperation::getAsset() {
    if(!m_asset)
        m_asset= Asset::fromXdr(m_op.asset);
    return m_asset;
}

QString CreateClaimableBalanceOperation::getAmount() {
    return Operation::fromXdrAmount(m_op.amount);
}

QList<Claimant> CreateClaimableBalanceOperation::getClaimants() {
    QList<Claimant> claimants;
    for(const auto& c : m_op.claimants.value)
    {
        claimants.append(Claimant::fromXdr(c));
    }
    return claimants;
}

void CreateClaimableBalanceOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &operation)
{
    Q_UNUSED(accountConverter)
    operation.type = stellar::OperationType::CREATE_CLAIMABLE_BALANCE;
    operation.operationCreateClaimableBalance = m_op;
}

CreateClaimableBalanceOperation *CreateClaimableBalanceOperation::build(stellar::CreateClaimableBalanceOp &op)
{
    return new CreateClaimableBalanceOperation(op);
}

CreateClaimableBalanceOperation *CreateClaimableBalanceOperation::create(QString amount, Asset *asset, QList<Claimant> claimants)
{
    return new CreateClaimableBalanceOperation(amount,asset,claimants);
}

CreateClaimableBalanceOperation *CreateClaimableBalanceOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
