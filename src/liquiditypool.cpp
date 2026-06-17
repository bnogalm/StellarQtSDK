#include "liquiditypool.h"

#include <QDataStream>
#include <QCryptographicHash>
#include <stdexcept>
#include <cstring>   // std::memcmp (GCC 13's libstdc++ no longer pulls it in transitively)

#include "asset.h"

QSTELLAR_BEGIN_NS

namespace {

QByteArray assetToXdrBytes(Asset* a)
{
    if (!a) {
        throw std::runtime_error("Asset cannot be null");
    }
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stellar::Asset xdr = a->toXdr();
    stream << xdr;
    return bytes;
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

    stellar::LiquidityPoolParameters params;
    stellar::LiquidityPoolConstantProductParameters& cp = params.fillConstantProduct();
    cp.assetA = assetA->toXdr();
    cp.assetB = assetB->toXdr();
    cp.fee = fee;

    QByteArray xdr;
    QDataStream out(&xdr, QIODevice::WriteOnly);
    out << params;

    return QCryptographicHash::hash(xdr, QCryptographicHash::Sha256);
}

QSTELLAR_END_NS
