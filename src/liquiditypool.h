#ifndef LIQUIDITYPOOL_H
#define LIQUIDITYPOOL_H

#include <QByteArray>
#include "xdr/stellarledgerentries.h"
#include "asset.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Static helpers around CAP-38 Liquidity Pools.
 *
 * Pool ID = SHA-256 of the XDR-encoded LiquidityPoolParameters union.
 * Assets must be in canonical order (XDR-byte lex). LIQUIDITY_POOL_FEE_V18 = 30.
 */
class LiquidityPool
{
public:
    static const qint32 FEE = 30;

    /** XDR-byte lexicographic comparison. <0 if a<b, 0 if a==b, >0 if a>b. */
    static int compareAssets(Asset* a, Asset* b);

    /** SHA-256 of XDR(LiquidityPoolParameters). Throws if assets are not ordered. */
    static QByteArray getLiquidityPoolID(stellar::LiquidityPoolType type,
                                         Asset* assetA,
                                         Asset* assetB,
                                         qint32 fee);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(LiquidityPool)

#endif // LIQUIDITYPOOL_H
