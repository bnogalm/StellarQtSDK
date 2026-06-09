#ifndef LIQUIDITYPOOLWITHDREWEFFECTRESPONSE_H
#define LIQUIDITYPOOLWITHDREWEFFECTRESPONSE_H

#include "effectresponse.h"
#include "../liquiditypool.h"
#include "../assetamount.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * `liquidity_pool_withdrew` (type_i = 91) effect.
 */
class LiquidityPoolWithdrewEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(LiquidityPool liquidity_pool MEMBER m_liquidityPool)
    Q_PROPERTY(QList<AssetAmount> reserves_received MEMBER m_reservesReceived)
    Q_PROPERTY(QString shares_redeemed MEMBER m_sharesRedeemed)

    LiquidityPool m_liquidityPool;
    QList<AssetAmount> m_reservesReceived;
    QString m_sharesRedeemed;
public:
    explicit LiquidityPoolWithdrewEffectResponse(QNetworkReply* reply = nullptr)
        : EffectResponse(reply) {}

    LiquidityPool       getLiquidityPool() const   { return m_liquidityPool; }
    QList<AssetAmount>  getReservesReceived() const { return m_reservesReceived; }
    QString             getSharesRedeemed() const  { return m_sharesRedeemed; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(LiquidityPoolWithdrewEffectResponse)



Q_DECLARE_METATYPE(qstellar::LiquidityPoolWithdrewEffectResponse*)

#endif // LIQUIDITYPOOLWITHDREWEFFECTRESPONSE_H
