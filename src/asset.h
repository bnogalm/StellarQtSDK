#ifndef ASSET_H
#define ASSET_H
#include <exception>
#include "xdr/stellarledgerentries.h"
class AssetCodeLengthInvalidException: public std::exception
{
};
class KeyPair;

/**
 * @brief The Asset class
 * The Asset class in the test are used directly as parameters in the request, they will not be deleted, so don't use it as an example.
 * In an application using this SDK, there should be an Asset database/list/pool that is used to take already created Asset into requests.
 */
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
    static Asset* createNonNativeAsset(QString code, QString issuer);
    static Asset* create(QString type, QString code, QString issuer);


    ///
    /// \brief create non native asset, same as createNonNativeAsset
    /// \param code
    /// \param issuer
    /// \return asset
    ///
    static Asset* create(QString code, QString issuer);
    /**
     * Parses an asset string and returns the equivalent Asset instance.
     * The asset string is expected to either be "native" or a string of the form "CODE:ISSUER"
     *
     * @param canonicalForm Canonical string representation of an asset
     */
    static Asset* create(QString canonicalForm);
    /**
     * Generates Asset object from a given XDR object
     * @param xdr XDR object
     */
    static Asset* fromXdr(const stellar::Asset &xdr);

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

    virtual QString toString() const = 0;
};

Asset* checkNotNull(Asset* asset, const char *error);



#endif // ASSET_H
