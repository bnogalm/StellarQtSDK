#include "liquiditypool.h"

#include <QDataStream>
#include <QCryptographicHash>
#include <stdexcept>
#include <cstring>   // std::memcmp (GCC 13's libstdc++ no longer pulls it in transitively)
#include <algorithm>

#include "asset.h"

QSTELLAR_BEGIN_NS

namespace {

QByteArray assetToXdrBytes(Asset* a)
{
    if (!a) {
        throw std::runtime_error("Asset cannot be null");
    }
    // Checked: an asset the writer cannot encode wrote nothing, and nothing
    // sorts before every asset, so it passed as the lesser of any pair.
    return xdr::encodeOrThrow(a->toXdr(), "the asset");
}

} // namespace

int LiquidityPool::compareAssets(Asset* a, Asset* b)
{
    QByteArray ba = assetToXdrBytes(a);
    QByteArray bb = assetToXdrBytes(b);
    int c = std::memcmp(ba.constData(), bb.constData(),
                        std::min(ba.size(), bb.size()));
    if (c != 0) return c;
    return ba.size() - bb.size();
}

QByteArray LiquidityPool::getLiquidityPoolID(stellar::LiquidityPoolType type,
                                             Asset* assetA,
                                             Asset* assetB,
                                             qint32 fee)
{
    if (type != stellar::LiquidityPoolType::LIQUIDITY_POOL_CONSTANT_PRODUCT) {
        throw std::runtime_error("unsupported LiquidityPoolType");
    }
    if (compareAssets(assetA, assetB) >= 0) {
        throw std::runtime_error("AssetA must be < AssetB in canonical XDR order");
    }
    // 30 is the only fee a pool has: an id for any other names no pool (js
    // and py refuse it).
    if (fee != FEE)
        throw std::runtime_error(QStringLiteral("a liquidity pool's fee is %1 basis points, not %2")
                                     .arg(FEE).arg(fee).toStdString());

    stellar::LiquidityPoolParameters params;
    stellar::LiquidityPoolConstantProductParameters& cp = params.fillConstantProduct();
    cp.assetA = assetA->toXdr();
    cp.assetB = assetB->toXdr();
    cp.fee = fee;

    // Hashed, so checked: short bytes would name some other pool.
    return QCryptographicHash::hash(xdr::encodeOrThrow(params, "the liquidity pool parameters"),
                                    QCryptographicHash::Sha256);
}

QSTELLAR_END_NS
