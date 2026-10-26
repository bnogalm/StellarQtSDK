#ifndef RESPONSES_LIQUIDITYPOOL_H
#define RESPONSES_LIQUIDITYPOOL_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMetaType>
#include "assetamount.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Liquidity pool shape returned by Horizon (used by LP effects and the
 * /liquidity_pools endpoint).
 *
 * Its name is also that of the pool-id helper in src/liquiditypool.h (both
 * part of the 1.0 API): a translation unit can include one of the two
 * headers, not both. They shared an include guard, so including both
 * silently dropped the second; it is a clear redefinition error now.
 */
class LiquidityPool
{
    Q_GADGET
    Q_PROPERTY(QString id MEMBER m_id)
    Q_PROPERTY(qint32 fee_bp MEMBER m_feeBp)
    Q_PROPERTY(QString type MEMBER m_type)
    Q_PROPERTY(QString total_trustlines MEMBER m_totalTrustlines)
    Q_PROPERTY(QString total_shares MEMBER m_totalShares)
    Q_PROPERTY(QList<AssetAmount> reserves MEMBER m_reserves)

    QString m_id;
    qint32  m_feeBp = 0;
    QString m_type;
    QString m_totalTrustlines;
    QString m_totalShares;
    QList<AssetAmount> m_reserves;

public:
    LiquidityPool() = default;

    QString            getId() const               { return m_id; }
    qint32             getFeeBp() const            { return m_feeBp; }
    QString            getType() const             { return m_type; }
    QString            getTotalTrustlines() const  { return m_totalTrustlines; }
    QString            getTotalShares() const      { return m_totalShares; }
    QList<AssetAmount> getReserves() const         { return m_reserves; }

    bool operator==(const LiquidityPool& o) const
    {
        return m_id == o.m_id
            && m_feeBp == o.m_feeBp
            && m_type == o.m_type
            && m_totalTrustlines == o.m_totalTrustlines
            && m_totalShares == o.m_totalShares
            && m_reserves == o.m_reserves;
    }
    bool operator!=(const LiquidityPool& o) const { return !(*this == o); }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(LiquidityPool)



Q_DECLARE_METATYPE(qstellar::LiquidityPool)

#endif // RESPONSES_LIQUIDITYPOOL_H
