#ifndef TRUSTLINEASSET_H
#define TRUSTLINEASSET_H

#include <QByteArray>
#include "xdr/stellarledgerentries.h"
#include "asset.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Abstract wrapper for the TrustLineAsset XDR union (CAP-38, Protocol 18+).
 *
 * For native / credit-alphanum assets, use TrustLineAsset::wrap(Asset*).
 * For pool shares, use LiquidityPoolShareTrustLineAsset(poolId).
 *
 * Unlike ChangeTrustAsset, the POOL_SHARE variant only carries the 32-byte
 * PoolID — the ledger already stores the pool parameters separately.
 */
class TrustLineAsset
{
public:
    virtual ~TrustLineAsset() = default;

    static TrustLineAsset* wrap(Asset* asset);
    static TrustLineAsset* fromXdr(const stellar::TrustLineAsset& xdr);

    virtual QString getType() const = 0;
    virtual stellar::TrustLineAsset toXdr() const = 0;
    virtual bool equals(const TrustLineAsset* other) const = 0;
};

class TrustLineAssetWrapper : public TrustLineAsset
{
    Asset* m_asset;
public:
    explicit TrustLineAssetWrapper(Asset* asset);
    ~TrustLineAssetWrapper() override;

    Asset* getAsset() const { return m_asset; }

    QString getType() const override;
    stellar::TrustLineAsset toXdr() const override;
    bool equals(const TrustLineAsset* other) const override;
};

class LiquidityPoolShareTrustLineAsset : public TrustLineAsset
{
    QByteArray m_poolId; // 32 bytes
public:
    /** poolId must be exactly 32 bytes (PoolID = SHA-256). */
    explicit LiquidityPoolShareTrustLineAsset(const QByteArray& poolId);

    const QByteArray& getLiquidityPoolID() const { return m_poolId; }

    QString getType() const override;
    stellar::TrustLineAsset toXdr() const override;
    bool equals(const TrustLineAsset* other) const override;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TrustLineAsset)
QSTELLAR_ALIAS(TrustLineAssetWrapper)
QSTELLAR_ALIAS(LiquidityPoolShareTrustLineAsset)

#endif // TRUSTLINEASSET_H
