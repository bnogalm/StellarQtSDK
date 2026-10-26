#include "asset.h"
#include "assettypecreditalphanum.h"
#include "assettypecreditalphanum4.h"
#include "assettypecreditalphanum12.h"
#include "assettypenative.h"
#include "util.h"
#include "keypair.h"
#include "stellaraddress.h"
#include <stdexcept>

QSTELLAR_BEGIN_NS

Asset *Asset::createNonNativeAsset(QString code, KeyPair *issuer) {
    if (code.length() >= 1 && code.length() <= 4) {
        return new AssetTypeCreditAlphaNum4(code, issuer);
    } else if (code.length() >= 5 && code.length() <= 12) {
        return new AssetTypeCreditAlphaNum12(code, issuer);
    } else {
        // Ownership of `issuer` transfers only by constructing one of the two
        // assets above. On this arm nothing takes it, so throwing without
        // deleting abandoned a KeyPair on every bad code length — and the
        // caller cannot clean up, because it has already handed the pointer
        // over and does not know which arm ran.
        delete issuer;
        throw AssetCodeLengthInvalidException(QStringLiteral("an asset code is 1 to 12 ASCII letters and digits, not \"%1\"").arg(code).toStdString());
    }
}

Asset *Asset::createNonNativeAsset(QString code, const KeyPair &issuer)
{
    if (code.length() >= 1 && code.length() <= 4) {
        return new AssetTypeCreditAlphaNum4(code, issuer);
    } else if (code.length() >= 5 && code.length() <= 12) {
        return new AssetTypeCreditAlphaNum12(code, issuer);
    } else {
        throw AssetCodeLengthInvalidException(QStringLiteral("an asset code is 1 to 12 ASCII letters and digits, not \"%1\"").arg(code).toStdString());
    }
}

Asset *Asset::createNonNativeAsset(QString code, QString issuer)
{
    return Asset::createNonNativeAsset(code, KeyPair::fromAccountId(issuer));
}

Asset *Asset::create(QString type, QString code, QString issuer) {
    // Every type but "native" used to become the asset the code's length
    // made: "credit_alphanum12" with "USD" an alphanum4 asset, and a type
    // nobody knows (a typo, "liquidity_pool_shares") an issued asset.
    if (type == QLatin1String("native")) {
        if (!code.isEmpty() || !issuer.isEmpty())
            throw std::runtime_error(QStringLiteral("a native asset has no code and no issuer: type \"%1\", code \"%2\"")
                                         .arg(type, code).toStdString());
        return new AssetTypeNative();
    }
    int minLength = 0, maxLength = 0;
    if (type == QLatin1String("credit_alphanum4")) {
        minLength = 1;
        maxLength = 4;
    } else if (type == QLatin1String("credit_alphanum12")) {
        minLength = 5;
        maxLength = 12;
    } else {
        throw std::runtime_error(QStringLiteral("unknown asset type \"%1\" (code \"%2\")")
                                     .arg(type, code).toStdString());
    }
    // Core's rule for a code (isAssetValid): ASCII letters and digits.
    bool alphanumeric = true;
    for (const QChar c : code) {
        const ushort u = c.unicode();
        alphanumeric = alphanumeric && ((u >= '0' && u <= '9') || (u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z'));
    }
    if (code.size() < minLength || code.size() > maxLength || !alphanumeric)
        throw std::runtime_error(QStringLiteral("asset type \"%1\" takes a code of %2 to %3 letters or digits, not \"%4\"")
                                     .arg(type).arg(minLength).arg(maxLength).arg(code).toStdString());
    KeyPair* keyPair = KeyPair::fromAccountId(issuer);
    // Both constructors own keyPair from here on, and free it if they throw.
    if (maxLength == 4)
        return new AssetTypeCreditAlphaNum4(code, keyPair);
    return new AssetTypeCreditAlphaNum12(code, keyPair);
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

QByteArray Asset::toSAC(const QString& networkPassphrase)
{
    return Util::getContractAddressFromAsset(networkPassphrase, this->toXdr());
}

StellarAddress Asset::toSACAddress(const QString& networkPassphrase)
{
    return StellarAddress(StellarAddress::Type::CONTRACT, toSAC(networkPassphrase));
}

Asset *checkNotNull(Asset *asset, const char *error)
{
    if(!asset){
        throw std::runtime_error(error);
    }
    return asset;
}

QSTELLAR_END_NS
