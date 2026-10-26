#ifndef QSTELLAR_ASSETREGISTRY_H
#define QSTELLAR_ASSETREGISTRY_H

#include <QHash>
#include <QObject>
#include <QString>

#include "qstellar_namespace.h"

QSTELLAR_FWD(Asset)

QSTELLAR_BEGIN_NS

/**
 * Long-lived, canonical-string-keyed owner of Asset instances.
 *
 * Why this exists: Asset ownership is inconsistent across the SDK. Most
 * operations (PaymentOperation, ChangeTrustOperation's legacy ctor,
 * PathPaymentStrict*, ClawbackOperation) copy `asset->toXdr()` and do NOT take
 * ownership, while ChangeTrustAsset::wrap() and
 * LiquidityPoolShareChangeTrustAsset DO. `asset.h` itself notes the problem
 * ("there should be an Asset database/list/pool"). A caller that mixes the two
 * either leaks or double-frees.
 *
 * QML makes that worse: it has no way to express ownership at all, and cannot
 * hold a raw Asset*. So the QML layer never passes Asset pointers around — it
 * passes the canonical string ("native", "USD:GABC...") and asks the registry
 * for the pointer at the C++ boundary.
 *
 * Instances are created once and reused, so the pointer handed to an operation
 * stays valid for the lifetime of the registry. Nothing else may delete them.
 */
class AssetRegistry
{
public:
    AssetRegistry();
    ~AssetRegistry();

    /**
     * Asset for a canonical form: "native", "CODE:ISSUER". Created on first
     * use and cached. The registry keeps ownership — never delete the result,
     * and never hand it to an API that takes ownership (ChangeTrustAsset::wrap,
     * LiquidityPoolShareChangeTrustAsset); use copyOf() for those.
     *
     * Throws std::runtime_error if the canonical form is malformed.
     */
    Asset* assetFor(const QString& canonicalForm);

    /**
     * A fresh, caller-owned Asset for the same canonical form. Use this at the
     * few call sites that take ownership.
     */
    Asset* copyOf(const QString& canonicalForm) const;

    /** Canonical form of an asset, the inverse of assetFor(). */
    static QString canonicalFormOf(Asset* asset);

    /** True if the canonical form parses; never throws. */
    static bool isValid(const QString& canonicalForm);

    /** Drops every cached asset. Invalidates every pointer handed out. */
    void clear();

    int count() const { return m_assets.size(); }

private:
    Q_DISABLE_COPY(AssetRegistry)
    QHash<QString, Asset*> m_assets;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(AssetRegistry)

#endif // QSTELLAR_ASSETREGISTRY_H
