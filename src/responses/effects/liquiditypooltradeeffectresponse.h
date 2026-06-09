#ifndef LIQUIDITYPOOLTRADEEFFECTRESPONSE_H
#define LIQUIDITYPOOLTRADEEFFECTRESPONSE_H

#include "effectresponse.h"
#include "../liquiditypool.h"
#include "../assetamount.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * `liquidity_pool_trade` (type_i = 92) effect.
 */
class LiquidityPoolTradeEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(LiquidityPool liquidity_pool MEMBER m_liquidityPool)
    Q_PROPERTY(AssetAmount sold MEMBER m_sold)
    Q_PROPERTY(AssetAmount bought MEMBER m_bought)

    LiquidityPool m_liquidityPool;
    AssetAmount m_sold;
    AssetAmount m_bought;
public:
    explicit LiquidityPoolTradeEffectResponse(QNetworkReply* reply = nullptr)
        : EffectResponse(reply) {}

    LiquidityPool getLiquidityPool() const { return m_liquidityPool; }
    AssetAmount   getSold() const          { return m_sold; }
    AssetAmount   getBought() const        { return m_bought; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(LiquidityPoolTradeEffectResponse)



Q_DECLARE_METATYPE(qstellar::LiquidityPoolTradeEffectResponse*)

#endif // LIQUIDITYPOOLTRADEEFFECTRESPONSE_H
