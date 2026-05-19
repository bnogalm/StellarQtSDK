#ifndef ASSETAMOUNT_H
#define ASSETAMOUNT_H

#include <QObject>
#include <QString>
#include <QMetaType>

/**
 * Asset / amount pair used in Horizon responses (trades, LP effects, ...).
 * `asset` is in canonical form "code:issuer" or "native".
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
Q_DECLARE_METATYPE(AssetAmount)

#endif // ASSETAMOUNT_H
