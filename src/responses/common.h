#ifndef COMMON_H
#define COMMON_H
// QVariantMap is used below: without this the installed header compiled only
// after something else had pulled <QVariant> in.
#include <QVariant>
#include <QVariantMap>
#include <stdexcept>
#include "../asset.h"
#include "../keypair.h"
#include "../assettypenative.h"

QSTELLAR_BEGIN_NS

/**
 * The asset Horizon names as {"asset_type", "asset_code", "asset_issuer"}.
 * Throws std::runtime_error when the type is not native, credit_alphanum4 or
 * credit_alphanum12, or the code's length is not the type's: every type but
 * "native" used to become the asset the code's length made, so a
 * credit_alphanum4 type with a 5-letter code, or a liquidity_pool_shares
 * type, read as another asset.
 */
inline Asset* assetFromVariantMap(const QVariantMap &m)
{
    Asset *asset;
    const QString type = m.value(QStringLiteral("asset_type")).toString();
    const QString code = m.value(QStringLiteral("asset_code")).toString();
    if (type == QLatin1String("native")) {
        if (!code.isEmpty() || !m.value(QStringLiteral("asset_issuer")).toString().isEmpty())
            throw std::runtime_error("a native asset with a code or an issuer");
        asset = new AssetTypeNative();
    } else if ((type == QLatin1String("credit_alphanum4") && code.size() >= 1 && code.size() <= 4)
               || (type == QLatin1String("credit_alphanum12") && code.size() >= 5 && code.size() <= 12)) {
        KeyPair* issuer = KeyPair::fromAccountId(m.value(QStringLiteral("asset_issuer")).toString());
        asset = Asset::createNonNativeAsset(code, issuer);
    } else {
        throw std::runtime_error(QStringLiteral("not an asset: type \"%1\", code \"%2\"")
                                     .arg(type, code).toStdString());
    }
    return asset;
}

QSTELLAR_END_NS

#ifdef STELLAR_QT_LEGACY_GLOBAL_NS
// Free helper (no class alias macro applies): expose it unqualified for 0.x
// callers that include this header at global scope (e.g. the test suite).
using ::qstellar::assetFromVariantMap;
#endif

#endif // COMMON_H
