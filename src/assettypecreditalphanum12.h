#ifndef ASSETTYPECREDITALPHANUM12_H
#define ASSETTYPECREDITALPHANUM12_H

#include "assettypecreditalphanum.h"


/**
 * Represents all assets with codes 5-12 characters long.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/assets.html" target="_blank">Assets</a>
 */
class AssetTypeCreditAlphaNum12 : public AssetTypeCreditAlphaNum
{
public:

    /**
     * Class constructor
     * @param code Asset code
     * @param issuer Asset issuer
     */
    AssetTypeCreditAlphaNum12(QString code, KeyPair *issuer);
    AssetTypeCreditAlphaNum12(QString code,const KeyPair &issuer);


    QString getType();


    stellar::Asset toXdr();
};

#endif // ASSETTYPECREDITALPHANUM12_H
