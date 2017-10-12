#ifndef ASSETTYPECREDITALPHANUM4_H
#define ASSETTYPECREDITALPHANUM4_H

#include "assettypecreditalphanum.h"
/**
 * Represents all assets with codes 1-4 characters long.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/assets.html" target="_blank">Assets</a>
 */
class AssetTypeCreditAlphaNum4: public AssetTypeCreditAlphaNum
{
public:
    /**
     * Class constructor
     * @param code Asset code
     * @param issuer Asset issuer
     */
    AssetTypeCreditAlphaNum4(QString code, KeyPair* issuer);
    AssetTypeCreditAlphaNum4(QString code, const KeyPair &issuer);

    QString getType();


    stellar::Asset toXdr();
};

#endif // ASSETTYPECREDITALPHANUM4_H
