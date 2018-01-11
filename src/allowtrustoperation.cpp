#include "allowtrustoperation.h"



AllowTrustOperation::AllowTrustOperation(KeyPair *trustor, QString assetCode, bool authorize)
    :m_trustor(nullptr)
{
    checkNotNull(trustor, "trustor cannot be null");
    checkNotNull(assetCode, "assetCode cannot be null");

    // trustor
    m_op.trustor = trustor->getXdrPublicKey();
    // asset
    QByteArray assetCodeUtf8 = assetCode.toUtf8();
    if (assetCodeUtf8.length() <= 4) {
        m_op.asset.type = stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4;
        QByteArray padded = Util::paddedByteArray(assetCodeUtf8,4);
        memcpy(m_op.asset.alphaNum4.assetCode,padded.data(),4);
    } else {
        m_op.asset.type = stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12;
        QByteArray padded = Util::paddedByteArray(assetCodeUtf8,12);
        memcpy(m_op.asset.alphaNum12.assetCode,padded.data(),12);
    }
    // authorize
    m_op.authorize=authorize;
}

AllowTrustOperation::AllowTrustOperation(stellar::AllowTrustOp &op)
    :m_trustor(nullptr),m_op(op)
{

}

AllowTrustOperation::~AllowTrustOperation()
{
    if(m_trustor)
        delete m_trustor;
}


KeyPair *AllowTrustOperation::getTrustor() {
    if(!m_trustor){
        m_trustor = KeyPair::fromXdrPublicKey(m_op.trustor);
    }
    return m_trustor;
}

QString AllowTrustOperation::getAssetCode() {
    switch (m_op.asset.type) {
    case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
        return Util::removeTailChars(QString::fromUtf8((char*)m_op.asset.alphaNum4.assetCode,4),'\0');
    case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
        return Util::removeTailChars(QString::fromUtf8((char*)m_op.asset.alphaNum12.assetCode,12),'\0');
    default:
        throw std::runtime_error("Unknown asset code");
    }
    return QString();
}

bool AllowTrustOperation::getAuthorize() {
    return m_op.authorize;
}

void AllowTrustOperation::fillOperationBody(stellar::Operation &operation) {
    operation.type = stellar::OperationType::ALLOW_TRUST;
    operation.operationAllowTrust = m_op;
}

AllowTrustOperation *AllowTrustOperation::build(stellar::AllowTrustOp &op)
{
    return new AllowTrustOperation(op);
}

AllowTrustOperation *AllowTrustOperation::create(KeyPair *trustor, QString assetCode, bool authorize)
{
    return new AllowTrustOperation(trustor, assetCode, authorize);
}

AllowTrustOperation *AllowTrustOperation::setSourceAccount(KeyPair *sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}

AllowTrustOperation *AllowTrustOperation::setSourceAccount(KeyPair &sourceAccount)
{
    Operation::setSourceAccount(new KeyPair(sourceAccount));
    return this;
}
