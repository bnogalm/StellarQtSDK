#ifndef ASSETAMOUNT_H
#define ASSETAMOUNT_H

#include <QObject>
#include <QString>
#include <QMetaType>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Asset / amount pair used in Horizon responses (trades, LP effects, ...).
 * `asset` is in canonical form "code:issuer" or "native", with one
 * exception: it is an EMPTY QString in the reserves of a
 * liquidity_pool_deposit or liquidity_pool_withdraw operation whose
 * transaction failed (reserves_max / reserves_deposited, reserves_min /
 * reserves_received), because Horizon leaves the asset out there: it names
 * the pool's assets only for a transaction that succeeded (and for one that
 * failed, nothing was deposited or received, which read 0). Everywhere else
 * (a pool's reserves, a pool effect, those reserves when the transaction
 * succeeded) it is always present, and a record without it is refused.
 * Check isTransactionSuccessful() (or an empty asset) before reading one of
 * those reserves as an asset.
 */
class AssetAmount
{
    Q_GADGET
    Q_PROPERTY(QString asset MEMBER m_asset)
    Q_PROPERTY(QString amount MEMBER m_amount)
    QString m_asset;
    QString m_amount;
public:
    AssetAmount() = default;

    QString getAsset()  const { return m_asset; }
    QString getAmount() const { return m_amount; }

    bool operator==(const AssetAmount& other) const
    {
        return m_asset == other.m_asset && m_amount == other.m_amount;
    }
    bool operator!=(const AssetAmount& other) const { return !(*this == other); }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(AssetAmount)



Q_DECLARE_METATYPE(qstellar::AssetAmount)

#endif // ASSETAMOUNT_H
