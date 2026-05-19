#ifndef LIQUIDITYPOOLREVOKEDEFFECTRESPONSE_H
#define LIQUIDITYPOOLREVOKEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "../liquiditypool.h"
#include "../liquiditypoolclaimableassetamount.h"

/**
 * `liquidity_pool_revoked` (type_i = 95) effect.
 *
 * Emitted when a trustline backing a pool is revoked; each reserve is
 * converted into a claimable balance whose id is captured in
 * `reserves_revoked`.
 */
class LiquidityPoolRevokedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(LiquidityPool liquidity_pool MEMBER m_liquidityPool)
    Q_PROPERTY(QList<LiquidityPoolClaimableAssetAmount> reserves_revoked MEMBER m_reservesRevoked)
    Q_PROPERTY(QString shares_revoked MEMBER m_sharesRevoked)

    LiquidityPool m_liquidityPool;
    QList<LiquidityPoolClaimableAssetAmount> m_reservesRevoked;
    QString m_sharesRevoked;
public:
    explicit LiquidityPoolRevokedEffectResponse(QNetworkReply* reply = nullptr)
        : EffectResponse(reply) {}

    LiquidityPool                              getLiquidityPool() const   { return m_liquidityPool; }
    QList<LiquidityPoolClaimableAssetAmount>   getReservesRevoked() const { return m_reservesRevoked; }
    QString                                    getSharesRevoked() const   { return m_sharesRevoked; }
};
Q_DECLARE_METATYPE(LiquidityPoolRevokedEffectResponse*)

#endif // LIQUIDITYPOOLREVOKEDEFFECTRESPONSE_H
