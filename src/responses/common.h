#ifndef COMMON_H
#define COMMON_H
#include "../asset.h"
#include "../keypair.h"
#include "../assettypenative.h"

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

#endif // COMMON_H
