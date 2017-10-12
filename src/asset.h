#ifndef ASSET_H
#define ASSET_H
#include <exception>
#include "xdr/stellarledgerentries.h"
class AssetCodeLengthInvalidException: public std::exception
{
};
class KeyPair;
class Asset
{
public:
    virtual ~Asset(){}
    /**
     * Creates one of AssetTypeCreditAlphaNum4 or AssetTypeCreditAlphaNum12 object based on a <code>code</code> length
     * @param code Asset code
     * @param issuer Asset issuer
     */
    static Asset* createNonNativeAsset(QString code, KeyPair* issuer);
    static Asset* createNonNativeAsset(QString code, const KeyPair& issuer);//makes a copy of issuer

    /**
     * Generates Asset object from a given XDR object
     * @param xdr XDR object
     */
    static Asset* fromXdr(stellar::Asset& xdr);

    /**
     * Returns asset type. Possible types:
     * <ul>
     *   <li><code>native</code></li>
     *   <li><code>credit_alphanum4</code></li>
     *   <li><code>credit_alphanum12</code></li>
     * </ul>
     */
    virtual QString getType() = 0;
    virtual bool equals(Asset* object);

    /**
     * Generates XDR object from a given Asset object
     */
    virtual stellar::Asset toXdr() = 0;
};

Asset* checkNotNull(Asset* asset, const char *error);



#endif // ASSET_H
