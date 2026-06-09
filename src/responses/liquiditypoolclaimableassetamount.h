#ifndef LIQUIDITYPOOLCLAIMABLEASSETAMOUNT_H
#define LIQUIDITYPOOLCLAIMABLEASSETAMOUNT_H

#include <QObject>
#include <QString>
#include <QMetaType>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Asset / amount pair plus the resulting claimable_balance_id, used by
 * liquidity_pool_revoked effects.
 */
class LiquidityPoolClaimableAssetAmount
{
    Q_GADGET
    Q_PROPERTY(QString asset MEMBER m_asset)
    Q_PROPERTY(QString amount MEMBER m_amount)
    Q_PROPERTY(QString claimable_balance_id MEMBER m_claimableBalanceId)
    QString m_asset;
    QString m_amount;
    QString m_claimableBalanceId;
public:
    LiquidityPoolClaimableAssetAmount() = default;

    QString getAsset()              const { return m_asset; }
    QString getAmount()             const { return m_amount; }
    QString getClaimableBalanceId() const { return m_claimableBalanceId; }

    bool operator==(const LiquidityPoolClaimableAssetAmount& o) const
    {
        return m_asset == o.m_asset
            && m_amount == o.m_amount
            && m_claimableBalanceId == o.m_claimableBalanceId;
    }
    bool operator!=(const LiquidityPoolClaimableAssetAmount& o) const { return !(*this == o); }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(LiquidityPoolClaimableAssetAmount)



Q_DECLARE_METATYPE(qstellar::LiquidityPoolClaimableAssetAmount)

#endif // LIQUIDITYPOOLCLAIMABLEASSETAMOUNT_H
