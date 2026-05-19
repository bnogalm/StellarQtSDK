#include "trustlineasset.h"

#include <stdexcept>
#include <cstring>

#include "asset.h"

TrustLineAsset* TrustLineAsset::wrap(Asset* asset)
{
    if (!asset) {
        throw std::runtime_error("Asset cannot be null");
    }
    return new TrustLineAssetWrapper(asset);
}

TrustLineAsset* TrustLineAsset::fromXdr(const stellar::TrustLineAsset& xdr)
{
    switch (xdr.type) {
    case stellar::AssetType::ASSET_TYPE_NATIVE:
    case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
    case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12: {
        stellar::Asset asXdr;
        asXdr.type = xdr.type;
        if (xdr.type == stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4) {
            std::memcpy(asXdr.alphaNum4.assetCode, xdr.alphaNum4.assetCode,
                        sizeof(asXdr.alphaNum4.assetCode));
            asXdr.alphaNum4.issuer = xdr.alphaNum4.issuer;
        } else if (xdr.type == stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12) {
            std::memcpy(asXdr.alphaNum12.assetCode, xdr.alphaNum12.assetCode,
                        sizeof(asXdr.alphaNum12.assetCode));
            asXdr.alphaNum12.issuer = xdr.alphaNum12.issuer;
        }
        return new TrustLineAssetWrapper(Asset::fromXdr(asXdr));
    }
    case stellar::AssetType::ASSET_TYPE_POOL_SHARE:
        return new LiquidityPoolShareTrustLineAsset(
            QByteArray(reinterpret_cast<const char*>(xdr.liquidityPoolID), 32));
    }
    throw std::runtime_error("unknown TrustLineAsset type");
}

// ─── TrustLineAssetWrapper ───────────────────────────────────────────────────

TrustLineAssetWrapper::TrustLineAssetWrapper(Asset* asset) : m_asset(asset) {}

TrustLineAssetWrapper::~TrustLineAssetWrapper() { delete m_asset; }

QString TrustLineAssetWrapper::getType() const { return m_asset->getType(); }

stellar::TrustLineAsset TrustLineAssetWrapper::toXdr() const
{
    stellar::Asset assetXdr = m_asset->toXdr();
    stellar::TrustLineAsset out;
    out.type = assetXdr.type;
    switch (assetXdr.type) {
    case stellar::AssetType::ASSET_TYPE_NATIVE: break;
    case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
        std::memcpy(out.alphaNum4.assetCode, assetXdr.alphaNum4.assetCode,
                    sizeof(out.alphaNum4.assetCode));
        out.alphaNum4.issuer = assetXdr.alphaNum4.issuer; break;
    case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
        std::memcpy(out.alphaNum12.assetCode, assetXdr.alphaNum12.assetCode,
                    sizeof(out.alphaNum12.assetCode));
        out.alphaNum12.issuer = assetXdr.alphaNum12.issuer; break;
    case stellar::AssetType::ASSET_TYPE_POOL_SHARE:
        throw std::runtime_error("plain Asset cannot carry POOL_SHARE");
    }
    return out;
}

bool TrustLineAssetWrapper::equals(const TrustLineAsset* other) const
{
    auto o = dynamic_cast<const TrustLineAssetWrapper*>(other);
    if (!o) return false;
    return m_asset->equals(o->m_asset);
}

// ─── LiquidityPoolShareTrustLineAsset ────────────────────────────────────────

LiquidityPoolShareTrustLineAsset::LiquidityPoolShareTrustLineAsset(const QByteArray& poolId)
    : m_poolId(poolId)
{
    if (poolId.size() != 32) {
        throw std::runtime_error("PoolID must be exactly 32 bytes");
    }
}

QString LiquidityPoolShareTrustLineAsset::getType() const
{
    return QStringLiteral("liquidity_pool_shares");
}

stellar::TrustLineAsset LiquidityPoolShareTrustLineAsset::toXdr() const
{
    stellar::TrustLineAsset out;
    out.type = stellar::AssetType::ASSET_TYPE_POOL_SHARE;
    std::memcpy(out.liquidityPoolID, m_poolId.constData(), 32);
    return out;
}

bool LiquidityPoolShareTrustLineAsset::equals(const TrustLineAsset* other) const
{
    auto o = dynamic_cast<const LiquidityPoolShareTrustLineAsset*>(other);
    if (!o) return false;
    return m_poolId == o->m_poolId;
}
