#ifndef CHANGETRUSTASSET_H
#define CHANGETRUSTASSET_H

#include <QByteArray>
#include "xdr/stellarledgerentries.h"

class Asset;

/**
 * Abstract wrapper for the ChangeTrustAsset XDR union (CAP-38, Protocol 18+).
 *
 * For native / credit-alphanum assets, use ChangeTrustAsset::wrap(Asset*).
 * For pool shares, use LiquidityPoolShareChangeTrustAsset(assetA, assetB, fee).
 */
class ChangeTrustAsset
{
public:
    virtual ~ChangeTrustAsset() = default;

    /** Wraps a regular Asset (native/alphanum4/alphanum12). Takes ownership of `asset`. */
    static ChangeTrustAsset* wrap(Asset* asset);

    /** Builds the wrapper that matches an XDR ChangeTrustAsset payload. */
    static ChangeTrustAsset* fromXdr(const stellar::ChangeTrustAsset& xdr);

    /** "native" / "credit_alphanum4" / "credit_alphanum12" / "liquidity_pool_shares". */
    virtual QString getType() const = 0;

    virtual stellar::ChangeTrustAsset toXdr() const = 0;

    virtual bool equals(const ChangeTrustAsset* other) const = 0;
};

/**
 * ChangeTrustAsset variant carrying a regular Asset (non-pool-share).
 * Owns the underlying Asset.
 */
class ChangeTrustAssetWrapper : public ChangeTrustAsset
{
    Asset* m_asset;
public:
    explicit ChangeTrustAssetWrapper(Asset* asset);
    ~ChangeTrustAssetWrapper() override;

    Asset* getAsset() const { return m_asset; }

    QString getType() const override;
    stellar::ChangeTrustAsset toXdr() const override;
    bool equals(const ChangeTrustAsset* other) const override;
};

/**
 * ChangeTrustAsset variant carrying LiquidityPoolParameters (CAP-38 pool share).
 * The wire format embeds the full parameters so core can create the pool if needed.
 */
class LiquidityPoolShareChangeTrustAsset : public ChangeTrustAsset
{
    Asset* m_assetA;
    Asset* m_assetB;
    qint32 m_fee;
public:
    /** Takes ownership of both assets. Order must be canonical (a < b XDR-byte). */
    LiquidityPoolShareChangeTrustAsset(Asset* assetA, Asset* assetB, qint32 fee);
    ~LiquidityPoolShareChangeTrustAsset() override;

    Asset* getAssetA() const { return m_assetA; }
    Asset* getAssetB() const { return m_assetB; }
    qint32 getFee() const { return m_fee; }

    /** SHA-256(LiquidityPoolParameters XDR) — same as LiquidityPool::getLiquidityPoolID. */
    QByteArray getLiquidityPoolID() const;

    QString getType() const override;
    stellar::ChangeTrustAsset toXdr() const override;
    bool equals(const ChangeTrustAsset* other) const override;
};

#endif // CHANGETRUSTASSET_H
