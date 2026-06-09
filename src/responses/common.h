#ifndef COMMON_H
#define COMMON_H
#include "../asset.h"
#include "../keypair.h"
#include "../assettypenative.h"

QSTELLAR_BEGIN_NS

inline Asset* assetFromVariantMap(const QVariantMap &m)
{
    Asset *asset;
    if (m.value(QStringLiteral("asset_type")).toString()=="native") {
        asset = new AssetTypeNative();
    } else {
        KeyPair* issuer = KeyPair::fromAccountId(m.value(QStringLiteral("asset_issuer")).toString());
        asset = Asset::createNonNativeAsset(m.value(QStringLiteral("asset_code")).toString(), issuer);
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
