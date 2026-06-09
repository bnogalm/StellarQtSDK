#ifndef LIQUIDITYPOOLREMOVEDEFFECTRESPONSE_H
#define LIQUIDITYPOOLREMOVEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * `liquidity_pool_removed` (type_i = 94) effect.
 *
 * Unlike the other LP effects, the JSON payload here only carries the pool's
 * id (the pool is gone by the time we receive the effect).
 */
class LiquidityPoolRemovedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(QString liquidity_pool_id MEMBER m_liquidityPoolId)
    QString m_liquidityPoolId;
public:
    explicit LiquidityPoolRemovedEffectResponse(QNetworkReply* reply = nullptr)
        : EffectResponse(reply) {}

    QString getLiquidityPoolId() const { return m_liquidityPoolId; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(LiquidityPoolRemovedEffectResponse)



Q_DECLARE_METATYPE(qstellar::LiquidityPoolRemovedEffectResponse*)

#endif // LIQUIDITYPOOLREMOVEDEFFECTRESPONSE_H
