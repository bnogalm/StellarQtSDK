#include "assetregistry.h"

#include <stdexcept>

#include "asset.h"
#include "assettypenative.h"
#include "assettypecreditalphanum.h"

QSTELLAR_BEGIN_NS

AssetRegistry::AssetRegistry()
{
}

AssetRegistry::~AssetRegistry()
{
    clear();
}

Asset* AssetRegistry::assetFor(const QString& canonicalForm)
{
    auto it = m_assets.constFind(canonicalForm);
    if (it != m_assets.constEnd())
        return it.value();

    Asset* asset = Asset::create(canonicalForm);   // throws on a malformed form
    m_assets.insert(canonicalForm, asset);
    return asset;
}

Asset* AssetRegistry::copyOf(const QString& canonicalForm) const
{
    return Asset::create(canonicalForm);
}

QString AssetRegistry::canonicalFormOf(Asset* asset)
{
    if (!asset)
        throw std::runtime_error("asset cannot be null");
    if (dynamic_cast<AssetTypeNative*>(asset))
        return QStringLiteral("native");
    auto* alpha = dynamic_cast<AssetTypeCreditAlphaNum*>(asset);
    if (!alpha)
        throw std::runtime_error("asset has no canonical form (pool shares are not classic assets)");
    return alpha->getCode() + ":" + alpha->getIssuer().getAccountId();
}

bool AssetRegistry::isValid(const QString& canonicalForm)
{
    try {
        Asset* probe = Asset::create(canonicalForm);
        delete probe;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void AssetRegistry::clear()
{
    for (Asset* a : m_assets)
        delete a;
    m_assets.clear();
}

QSTELLAR_END_NS
