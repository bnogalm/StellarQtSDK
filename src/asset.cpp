#include "asset.h"
#include "assettypecreditalphanum.h"
#include "assettypecreditalphanum4.h"
#include "assettypecreditalphanum12.h"
#include "assettypenative.h"
#include "util.h"
#include "keypair.h"
#include <stdexcept>

Asset *Asset::createNonNativeAsset(QString code, KeyPair *issuer) {
    if (code.length() >= 1 && code.length() <= 4) {
        return new AssetTypeCreditAlphaNum4(code, issuer);
    } else if (code.length() >= 5 && code.length() <= 12) {
        return new AssetTypeCreditAlphaNum12(code, issuer);
    } else {
        throw new AssetCodeLengthInvalidException();
    }
}

Asset *Asset::createNonNativeAsset(QString code, const KeyPair &issuer)
{
    if (code.length() >= 1 && code.length() <= 4) {
        return new AssetTypeCreditAlphaNum4(code, issuer);
    } else if (code.length() >= 5 && code.length() <= 12) {
        return new AssetTypeCreditAlphaNum12(code, issuer);
    } else {
        throw new AssetCodeLengthInvalidException();
    }
}

Asset *Asset::createNonNativeAsset(QString code, QString issuer)
{
    return Asset::createNonNativeAsset(code, KeyPair::fromAccountId(issuer));
}

Asset *Asset::create(QString type, QString code, QString issuer) {
    if (type == "native") {
        return new AssetTypeNative();
    }
    else{
        return Asset::createNonNativeAsset(code, KeyPair::fromAccountId(issuer));
    }
}

Asset *Asset::create(QString code, QString issuer)
{
    return Asset::createNonNativeAsset(code, KeyPair::fromAccountId(issuer));
}

Asset *Asset::create(QString canonicalForm)
{
    if (canonicalForm == "native")
    {
        return new AssetTypeNative();
    }

    QStringList parts = canonicalForm.split(":");
    if (parts.size() != 2)
    {
        throw std::runtime_error("invalid asset");
    }
    return Asset::createNonNativeAsset(parts[0], parts[1]);
}

Asset *Asset::fromXdr(const stellar::Asset &xdr) {
    switch (xdr.type) {
    case stellar::AssetType::ASSET_TYPE_NATIVE:
        return new AssetTypeNative();
    case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
    {
        QString assetCode4 = Util::paddedByteArrayToString(QByteArray((char*)xdr.alphaNum4.assetCode,sizeof(xdr.alphaNum4.assetCode)));
        //memcpy(&pk.ed25519, xdr.alphaNum4.issuer.ed25519,sizeof(pk.ed25519));
        //pk.type = stellar::PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
        KeyPair* issuer4 = KeyPair::fromXdrPublicKey(xdr.alphaNum4.issuer);
        return new AssetTypeCreditAlphaNum4(assetCode4, issuer4);
    }
    case stellar::AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
    {
        QString assetCode12 = Util::paddedByteArrayToString(QByteArray((char*)xdr.alphaNum12.assetCode,sizeof(xdr.alphaNum12.assetCode)));
        KeyPair* issuer12 = KeyPair::fromXdrPublicKey(xdr.alphaNum12.issuer);
        return new AssetTypeCreditAlphaNum12(assetCode12, issuer12);
    }
    default:
        throw std::runtime_error("Unknown asset type");
    }
}

bool Asset::equals(Asset *object) {
    Q_UNUSED(object)
    return false;
}

Asset *checkNotNull(Asset *asset, const char *error)
{
    if(!asset){
        throw std::runtime_error(error);
    }
    return asset;
}
