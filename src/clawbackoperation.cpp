#include "clawbackoperation.h"
#include "assettypecreditalphanum.h"


ClawbackOperation::ClawbackOperation(QString from, Asset *asset, QString amount):m_asset(nullptr)
{
    checkNotNull(from, "from cannot be null");
    checkNotNull(asset, "asset cannot be null");    
    AssetTypeCreditAlphaNum* assetAlphaNum = Util::assertNonNativeAsset(asset);
    checkNotNull(amount, "amount cannot be null");


    m_op.from = StrKey::encodeToXDRMuxedAccount(from);
    m_op.asset = assetAlphaNum->toXdr();
    m_op.amount = Operation::toXdrAmount(amount);
}

ClawbackOperation::ClawbackOperation(stellar::ClawbackOp &op)
    :m_op(op)
    ,m_asset(nullptr)
{

}

ClawbackOperation::~ClawbackOperation()
{
    if(m_asset)
        delete m_asset;
}

void ClawbackOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &operation)
{    
    operation.type = stellar::OperationType::CLAWBACK;
    operation.operationClawback = m_op;
    operation.operationClawback.from = accountConverter.filter(operation.operationClawback.from);
}

QString ClawbackOperation::getFrom() const
{
    return StrKey::encodeStellarMuxedAccount(m_op.from);
}

QString ClawbackOperation::getAmount() const
{
    return Operation::fromXdrAmount(m_op.amount);
}

Asset *ClawbackOperation::getAsset()
{
    if(!m_asset)
        m_asset = Asset::fromXdr(m_op.asset);
    return m_asset;
}

ClawbackOperation *ClawbackOperation::build(AccountConverter &accountConverter, stellar::ClawbackOp &op)
{
    auto result = new ClawbackOperation(op);
    result->m_op.from = accountConverter.filter(op.from);
    return result;
}

ClawbackOperation *ClawbackOperation::create(QString from, Asset *asset, QString amount)
{
    return new ClawbackOperation(from,asset,amount);
}

ClawbackOperation *ClawbackOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
