#ifndef LIQUIDITYPOOLCREATEDEFFECTRESPONSE_H
#define LIQUIDITYPOOLCREATEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "../liquiditypool.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * `liquidity_pool_created` (type_i = 93) effect.
 */
class LiquidityPoolCreatedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(LiquidityPool liquidity_pool MEMBER m_liquidityPool)
    LiquidityPool m_liquidityPool;
public:
    explicit LiquidityPoolCreatedEffectResponse(QNetworkReply* reply = nullptr)
        : EffectResponse(reply) {}

    LiquidityPool getLiquidityPool() const { return m_liquidityPool; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(LiquidityPoolCreatedEffectResponse)



Q_DECLARE_METATYPE(qstellar::LiquidityPoolCreatedEffectResponse*)

#endif // LIQUIDITYPOOLCREATEDEFFECTRESPONSE_H
