#ifndef ASSETTYPENATIVE_H
#define ASSETTYPENATIVE_H
#include "asset.h"


/**
 * Represents Stellar native asset - <a href="https://www.stellar.org/developers/learn/concepts/assets.html" target="_blank">lumens (XLM)</a>
 * @see <a href="https://www.stellar.org/developers/learn/concepts/assets.html" target="_blank">Assets</a>
 */
class AssetTypeNative : public Asset
{
public:
    AssetTypeNative();


    QString getType();


    bool equals(Asset *object);

    uint hashCode();

    stellar::Asset toXdr();

    QString toString() const;
};

#endif // ASSETTYPENATIVE_H
