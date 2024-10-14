#include "changetrustoperation.h"
#include "asset.h"

ChangeTrustOperation::ChangeTrustOperation(Asset *asset, QString limit)
    :m_asset(nullptr)
{
    checkNotNull(asset, "asset cannot be null");
    checkNotNull(limit, "limit cannot be null");

    m_op.line = asset->toXdr();
    m_op.limit = Operation::toXdrAmount(limit);
}

ChangeTrustOperation::ChangeTrustOperation(stellar::ChangeTrustOp &op)
    :m_asset(nullptr)
    ,m_op(op)
{

}

ChangeTrustOperation::~ChangeTrustOperation()
{
    if(m_asset)
        delete m_asset;
}

Asset *ChangeTrustOperation::getAsset() {
    if(!m_asset){
        m_asset = Asset::fromXdr(m_op.line);
    }
    return m_asset;
}

QString ChangeTrustOperation::getLimit() {
    return Operation::fromXdrAmount(m_op.limit);
}

void ChangeTrustOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &operation){
    Q_UNUSED(accountConverter)
    operation.type = stellar::OperationType::CHANGE_TRUST;
    operation.operationChangeTrust=m_op;
}

ChangeTrustOperation *ChangeTrustOperation::build(stellar::ChangeTrustOp &op)
{
    return new ChangeTrustOperation(op);
}

ChangeTrustOperation *ChangeTrustOperation::create(Asset* asset, QString limit)
{
    return new ChangeTrustOperation(asset, limit);
}

ChangeTrustOperation *ChangeTrustOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
