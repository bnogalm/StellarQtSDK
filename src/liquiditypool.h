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
 * The Horizon pool resource in src/responses/liquiditypool.h has the same
 * class name (both part of the 1.0 API): include one of the two headers in a
 * translation unit, not both.
 *
 * Pool ID = SHA-256 of the XDR-encoded LiquidityPoolParameters union.
 * Assets must be in canonical order (XDR-byte lex). LIQUIDITY_POOL_FEE_V18 = 30,
 * the only fee a pool has (any other is refused).
 *
 * The order is stellar-core's (isPoolParamsValid compares the XDR). js-stellar-
 * base (Asset.compare) and py-stellar-sdk compare the issuers' G... strkeys as
 * ASCII text instead, which differs for two assets of one type and code whose
 * issuers first differ where one strkey has a letter and the other a digit
 * (in base32 the digits 2-7 come after Z; in ASCII before A). For such a pair
 * js and py accept the order core refuses, and refuse the one core takes:
 * this SDK computes the id of the pool that can exist (checked on testnet:
 * core returns op_malformed for the other order). No real pubnet pool is such
 * a pair (39,621 checked).
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
