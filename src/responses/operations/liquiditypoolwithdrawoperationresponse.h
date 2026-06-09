#ifndef LIQUIDITYPOOLWITHDRAWOPERATIONRESPONSE_H
#define LIQUIDITYPOOLWITHDRAWOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
#include "../assetamount.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Represents LiquidityPoolWithdraw operation response.
 * @see <a href="https://developers.stellar.org/docs/data/apis/horizon/api-reference/resources/operations" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class LiquidityPoolWithdrawOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString liquidity_pool_id MEMBER m_liquidityPoolId)
    Q_PROPERTY(QList<AssetAmount> reserves_min MEMBER m_reservesMin)
    Q_PROPERTY(QString shares MEMBER m_shares)
    Q_PROPERTY(QList<AssetAmount> reserves_received MEMBER m_reservesReceived)

    QString m_liquidityPoolId;
    QList<AssetAmount> m_reservesMin;
    QString m_shares;
    QList<AssetAmount> m_reservesReceived;

public:
    LiquidityPoolWithdrawOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~LiquidityPoolWithdrawOperationResponse();

    /** Hex pool ID the withdraw targets. */
    QString            getLiquidityPoolId()  const { return m_liquidityPoolId; }
    /** Minimum reserves the withdrawer was willing to accept (asset/amount pairs). */
    QList<AssetAmount> getReservesMin()       const { return m_reservesMin; }
    /** Pool shares burned by the withdraw. */
    QString            getShares()            const { return m_shares; }
    /** Reserves actually received from the pool (asset/amount pairs). */
    QList<AssetAmount> getReservesReceived()  const { return m_reservesReceived; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(LiquidityPoolWithdrawOperationResponse)



Q_DECLARE_METATYPE(qstellar::LiquidityPoolWithdrawOperationResponse*)
#endif // LIQUIDITYPOOLWITHDRAWOPERATIONRESPONSE_H
