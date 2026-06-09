#ifndef LIQUIDITYPOOLDEPOSITOPERATIONRESPONSE_H
#define LIQUIDITYPOOLDEPOSITOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
#include "../assetamount.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Represents LiquidityPoolDeposit operation response.
 * @see <a href="https://developers.stellar.org/docs/data/apis/horizon/api-reference/resources/operations" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class LiquidityPoolDepositOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString liquidity_pool_id MEMBER m_liquidityPoolId)
    Q_PROPERTY(QList<AssetAmount> reserves_max MEMBER m_reservesMax)
    Q_PROPERTY(QString min_price MEMBER m_minPrice)
    Q_PROPERTY(QString max_price MEMBER m_maxPrice)
    Q_PROPERTY(QList<AssetAmount> reserves_deposited MEMBER m_reservesDeposited)
    Q_PROPERTY(QString shares_received MEMBER m_sharesReceived)

    QString m_liquidityPoolId;
    QList<AssetAmount> m_reservesMax;
    QString m_minPrice;
    QString m_maxPrice;
    QList<AssetAmount> m_reservesDeposited;
    QString m_sharesReceived;

public:
    LiquidityPoolDepositOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~LiquidityPoolDepositOperationResponse();

    /** Hex pool ID the deposit targets. */
    QString            getLiquidityPoolId()  const { return m_liquidityPoolId; }
    /** Maximum reserves the depositor was willing to add (asset/amount pairs). */
    QList<AssetAmount> getReservesMax()       const { return m_reservesMax; }
    /** Minimum price (reserveA/reserveB) the deposit accepted, as a decimal string. */
    QString            getMinPrice()          const { return m_minPrice; }
    /** Maximum price (reserveA/reserveB) the deposit accepted, as a decimal string. */
    QString            getMaxPrice()          const { return m_maxPrice; }
    /** Reserves actually deposited into the pool (asset/amount pairs). */
    QList<AssetAmount> getReservesDeposited() const { return m_reservesDeposited; }
    /** Pool shares received by the depositor. */
    QString            getSharesReceived()    const { return m_sharesReceived; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(LiquidityPoolDepositOperationResponse)



Q_DECLARE_METATYPE(qstellar::LiquidityPoolDepositOperationResponse*)
#endif // LIQUIDITYPOOLDEPOSITOPERATIONRESPONSE_H
