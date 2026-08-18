#include "createclaimablebalanceoperation.h"

QSTELLAR_BEGIN_NS


CreateClaimableBalanceOperation::CreateClaimableBalanceOperation(QString amount, Asset *asset, QList<Claimant> claimants):m_asset(nullptr)
{
    checkNotNull(amount, "amount cannot be null");
    checkNotNull(asset, "asset cannot be null");
    checkNotNull(claimants, "claimants cannot be null");
    m_op.amount = Operation::toXdrAmount(amount);
    m_op.asset = asset->toXdr();
    m_op.claimants.clear();
    // stellar::Array silently drops anything past its max: with 11 or more
    // claimants we signed a balance with fewer beneficiaries than the user
    // approved. The path payment operations already validate this way.
    checkArgument(claimants.size() <= m_op.claimants.maxSize(),
                  "too many claimants");
    for(const auto& c:claimants)
        m_op.claimants.append(c.toXdr());

}

CreateClaimableBalanceOperation::CreateClaimableBalanceOperation(stellar::CreateClaimableBalanceOp &op):m_op(op),m_asset(nullptr)
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
    // CreateClaimableBalanceOp holds an Array<Claimant> (a QVector), so it is
    // one of the union's non-trivial members: it MUST be placement-constructed
    // before being assigned, or the assignment operates on an unconstructed
    // container and corrupts its refcount.
    operation.fillCreateClaimableBalanceOp() = m_op;
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
QSTELLAR_END_NS
