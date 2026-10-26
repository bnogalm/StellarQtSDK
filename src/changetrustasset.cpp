#include "changetrustasset.h"

#include <stdexcept>
#include <cstring>
#include <memory>

#include "asset.h"
#include "liquiditypool.h"

QSTELLAR_BEGIN_NS

ChangeTrustAsset* ChangeTrustAsset::wrap(Asset* asset)
{
    if (!asset) {
        throw std::runtime_error("Asset cannot be null");
    }
    return new ChangeTrustAssetWrapper(asset);
}

ChangeTrustAsset* ChangeTrustAsset::fromXdr(const stellar::ChangeTrustAsset& xdr)
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
        return new ChangeTrustAssetWrapper(Asset::fromXdr(asXdr));
    }
    case stellar::AssetType::ASSET_TYPE_POOL_SHARE: {
        // Constant product is the only pool type: anything else has no
        // parameters to read, and was read as a constant-product pool.
        if (xdr.liquidityPool().type != stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT)
            throw std::runtime_error("unknown liquidity pool type");
        const auto& cp = xdr.liquidityPool().constantProduct;
        // Held until the pool share has taken them: a constructor that
        // refuses them (assets out of order, a fee other than 30) leaves them
        // with the caller, and here they leaked, both, on every such decode.
        // So did the first when the second failed to decode.
        std::unique_ptr<Asset> assetA(Asset::fromXdr(cp.assetA));
        std::unique_ptr<Asset> assetB(Asset::fromXdr(cp.assetB));
        ChangeTrustAsset* line = new LiquidityPoolShareChangeTrustAsset(assetA.get(), assetB.get(), cp.fee);
        assetA.release();
        assetB.release();
        return line;
    }
    }
    throw std::runtime_error("unknown ChangeTrustAsset type");
}

// ─── ChangeTrustAssetWrapper ─────────────────────────────────────────────────

ChangeTrustAssetWrapper::ChangeTrustAssetWrapper(Asset* asset) : m_asset(asset) {}

ChangeTrustAssetWrapper::~ChangeTrustAssetWrapper() { delete m_asset; }

QString ChangeTrustAssetWrapper::getType() const
{
    return m_asset->getType();
}

stellar::ChangeTrustAsset ChangeTrustAssetWrapper::toXdr() const
{
    stellar::Asset assetXdr = m_asset->toXdr();
    stellar::ChangeTrustAsset out;
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
    default:
        // Nor any other type: it was passed on with no arm, and written as a
        // bare discriminant.
        throw std::runtime_error("unknown asset type");
    }
    return out;
}

bool ChangeTrustAssetWrapper::equals(const ChangeTrustAsset* other) const
{
    auto o = dynamic_cast<const ChangeTrustAssetWrapper*>(other);
    if (!o) return false;
    return m_asset->equals(o->m_asset);
}

// ─── LiquidityPoolShareChangeTrustAsset ──────────────────────────────────────

LiquidityPoolShareChangeTrustAsset::LiquidityPoolShareChangeTrustAsset(
    Asset* assetA, Asset* assetB, qint32 fee)
    : m_assetA(assetA), m_assetB(assetB), m_fee(fee)
{
    if (!assetA || !assetB) {
        throw std::runtime_error("Assets cannot be null");
    }
    if (LiquidityPool::compareAssets(assetA, assetB) >= 0) {
        throw std::runtime_error("AssetA must be < AssetB in canonical XDR order");
    }
    // 30 basis points is the only fee a pool has (LIQUIDITY_POOL_FEE_V18):
    // core refuses a changeTrust with any other, js and py refuse to build it.
    if (fee != LiquidityPool::FEE)
        throw std::runtime_error(QStringLiteral("a liquidity pool's fee is %1 basis points, not %2")
                                     .arg(LiquidityPool::FEE).arg(fee).toStdString());
}

LiquidityPoolShareChangeTrustAsset::~LiquidityPoolShareChangeTrustAsset()
{
    delete m_assetA;
    delete m_assetB;
}

QByteArray LiquidityPoolShareChangeTrustAsset::getLiquidityPoolID() const
{
    return LiquidityPool::getLiquidityPoolID(
        stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT,
        m_assetA, m_assetB, m_fee);
}

QString LiquidityPoolShareChangeTrustAsset::getType() const
{
    return QStringLiteral("liquidity_pool_shares");
}

stellar::ChangeTrustAsset LiquidityPoolShareChangeTrustAsset::toXdr() const
{
    stellar::ChangeTrustAsset out;
    stellar::LiquidityPoolParameters& params = out.fillLiquidityPool();
    auto& cp = params.fillConstantProduct();
    cp.assetA = m_assetA->toXdr();
    cp.assetB = m_assetB->toXdr();
    cp.fee = m_fee;
    return out;
}

bool LiquidityPoolShareChangeTrustAsset::equals(const ChangeTrustAsset* other) const
{
    auto o = dynamic_cast<const LiquidityPoolShareChangeTrustAsset*>(other);
    if (!o) return false;
    return m_fee == o->m_fee
        && m_assetA->equals(o->m_assetA)
        && m_assetB->equals(o->m_assetB);
}

QSTELLAR_END_NS
