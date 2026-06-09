#ifndef LIQUIDITYPOOLDEPOSITEDEFFECTRESPONSE_H
#define LIQUIDITYPOOLDEPOSITEDEFFECTRESPONSE_H

#include "effectresponse.h"
#include "../liquiditypool.h"
#include "../assetamount.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * `liquidity_pool_deposited` (type_i = 90) effect.
 */
class LiquidityPoolDepositedEffectResponse : public EffectResponse
{
    Q_OBJECT
    Q_PROPERTY(LiquidityPool liquidity_pool MEMBER m_liquidityPool)
    Q_PROPERTY(QList<AssetAmount> reserves_deposited MEMBER m_reservesDeposited)
    Q_PROPERTY(QString shares_received MEMBER m_sharesReceived)

    LiquidityPool m_liquidityPool;
    QList<AssetAmount> m_reservesDeposited;
    QString m_sharesReceived;
public:
    explicit LiquidityPoolDepositedEffectResponse(QNetworkReply* reply = nullptr)
        : EffectResponse(reply) {}

    LiquidityPool       getLiquidityPool() const   { return m_liquidityPool; }
    QList<AssetAmount>  getReservesDeposited() const { return m_reservesDeposited; }
    QString             getSharesReceived() const  { return m_sharesReceived; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(LiquidityPoolDepositedEffectResponse)



Q_DECLARE_METATYPE(qstellar::LiquidityPoolDepositedEffectResponse*)

#endif // LIQUIDITYPOOLDEPOSITEDEFFECTRESPONSE_H
