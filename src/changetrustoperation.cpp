#include "changetrustoperation.h"

#include <cstring>

#include "asset.h"
#include "changetrustasset.h"

QSTELLAR_BEGIN_NS


namespace {

// Copies a plain stellar::Asset into a stellar::ChangeTrustAsset (3 shared variants).
void assignAssetToLine(stellar::ChangeTrustAsset& line, const stellar::Asset& asset)
{
    line.type = asset.type;
    switch (asset.type) {
    case stellar::AssetType::ASSET_TYPE_NATIVE: break;
    case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
        std::memcpy(line.alphaNum4.assetCode, asset.alphaNum4.assetCode,
                    sizeof(line.alphaNum4.assetCode));
        line.alphaNum4.issuer = asset.alphaNum4.issuer;
        break;
    case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
        std::memcpy(line.alphaNum12.assetCode, asset.alphaNum12.assetCode,
                    sizeof(line.alphaNum12.assetCode));
        line.alphaNum12.issuer = asset.alphaNum12.issuer;
        break;
    case stellar::AssetType::ASSET_TYPE_POOL_SHARE:
        throw std::runtime_error("plain Asset cannot carry POOL_SHARE");
    }
}

} // namespace

ChangeTrustOperation::ChangeTrustOperation(Asset *asset, QString limit)
    : m_asset(nullptr), m_lineCache(nullptr)
{
    checkNotNull(asset, "asset cannot be null");
    checkNotNull(limit, "limit cannot be null");

    assignAssetToLine(m_op.line, asset->toXdr());
    m_op.limit = Operation::toXdrAmount(limit);
}

ChangeTrustOperation::ChangeTrustOperation(ChangeTrustAsset* line, QString limit)
    : m_asset(nullptr), m_lineCache(nullptr)
{
    if (!line) throw std::runtime_error("line cannot be null");
    checkNotNull(limit, "limit cannot be null");

    m_op.line = line->toXdr();
    m_op.limit = Operation::toXdrAmount(limit);
}

ChangeTrustOperation::ChangeTrustOperation(stellar::ChangeTrustOp &op)
    : m_asset(nullptr)
    , m_lineCache(nullptr)
    , m_op(op)
{
}

ChangeTrustOperation::~ChangeTrustOperation()
{
    delete m_asset;
    delete m_lineCache;
}

Asset *ChangeTrustOperation::getAsset() {
    if (m_op.line.type == stellar::AssetType::ASSET_TYPE_POOL_SHARE) {
        return nullptr;
    }
    if (!m_asset) {
        stellar::Asset asXdr;
        asXdr.type = m_op.line.type;
        switch (asXdr.type) {
        case stellar::AssetType::ASSET_TYPE_NATIVE: break;
        case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            std::memcpy(asXdr.alphaNum4.assetCode, m_op.line.alphaNum4.assetCode,
                        sizeof(asXdr.alphaNum4.assetCode));
            asXdr.alphaNum4.issuer = m_op.line.alphaNum4.issuer;
            break;
        case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            std::memcpy(asXdr.alphaNum12.assetCode, m_op.line.alphaNum12.assetCode,
                        sizeof(asXdr.alphaNum12.assetCode));
            asXdr.alphaNum12.issuer = m_op.line.alphaNum12.issuer;
            break;
        case stellar::AssetType::ASSET_TYPE_POOL_SHARE: break;
        }
        m_asset = Asset::fromXdr(asXdr);
    }
    return m_asset;
}

ChangeTrustAsset* ChangeTrustOperation::getLine()
{
    if (!m_lineCache) {
        m_lineCache = ChangeTrustAsset::fromXdr(m_op.line);
    }
    return m_lineCache;
}

QString ChangeTrustOperation::getLimit() {
    return Operation::fromXdrAmount(m_op.limit);
}

void ChangeTrustOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &operation){
    Q_UNUSED(accountConverter)
    stellar::ChangeTrustOp& dst = operation.fillChangeTrustOp();
    dst = m_op;
}

ChangeTrustOperation *ChangeTrustOperation::build(stellar::ChangeTrustOp &op)
{
    return new ChangeTrustOperation(op);
}

ChangeTrustOperation *ChangeTrustOperation::create(Asset* asset, QString limit)
{
    return new ChangeTrustOperation(asset, limit);
}

ChangeTrustOperation *ChangeTrustOperation::create(ChangeTrustAsset* line, QString limit)
{
    return new ChangeTrustOperation(line, limit);
}

ChangeTrustOperation *ChangeTrustOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
QSTELLAR_END_NS
