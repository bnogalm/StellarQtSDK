#include "settrustlineflagsoperation.h"
#include "assettypecreditalphanum.h"
SetTrustlineFlagsOperation::SetTrustlineFlagsOperation(stellar::SetTrustLineFlagsOp& op):m_op(op),m_asset(nullptr)
{

}

SetTrustlineFlagsOperation::SetTrustlineFlagsOperation(QString trustor, Asset *asset, QFlags<stellar::TrustLineFlags> clearFlags, QFlags<stellar::TrustLineFlags> setFlags)
    :m_asset(nullptr)
{
    checkNotNull(trustor, "from cannot be null");
    checkNotNull(asset, "asset cannot be null");
    AssetTypeCreditAlphaNum* assetAlphaNum= Util::assertNonNativeAsset(asset);
    m_op.trustor = StrKey::encodeToXDRAccountId(trustor);
    m_op.asset = assetAlphaNum->toXdr();
    m_op.clearFlags = clearFlags;
    m_op.setFlags = setFlags;
}

SetTrustlineFlagsOperation::~SetTrustlineFlagsOperation()
{
    if(m_asset)
        delete m_asset;
}

void SetTrustlineFlagsOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &operation)
{
    Q_UNUSED(accountConverter)
    operation.type = stellar::OperationType::SET_TRUST_LINE_FLAGS;
    operation.operationSetTrustLineFlags = m_op;
}

Asset *SetTrustlineFlagsOperation::getAsset() {
    if(!m_asset)
    {
        m_asset = AssetTypeCreditAlphaNum::fromXdr(m_op.asset);
    }
    return m_asset;
}

QFlags<stellar::TrustLineFlags> SetTrustlineFlagsOperation::getSetFlags()
{
    return QFlags<stellar::TrustLineFlags>(m_op.setFlags);
}

QFlags<stellar::TrustLineFlags> SetTrustlineFlagsOperation::getClearFlags()
{
    return QFlags<stellar::TrustLineFlags>(m_op.clearFlags);
}

SetTrustlineFlagsOperation *SetTrustlineFlagsOperation::build(stellar::SetTrustLineFlagsOp &op)
{
    return new SetTrustlineFlagsOperation(op);
}

SetTrustlineFlagsOperation *SetTrustlineFlagsOperation::create(QString trustor, Asset *asset, QFlags<stellar::TrustLineFlags> clearFlags, QFlags<stellar::TrustLineFlags> setFlags)
{
    return new SetTrustlineFlagsOperation(trustor,asset,clearFlags,setFlags);
}

SetTrustlineFlagsOperation *SetTrustlineFlagsOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
