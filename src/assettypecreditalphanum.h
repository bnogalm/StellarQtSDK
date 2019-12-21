#ifndef ASSETTYPECREDITALPHANUM_H
#define ASSETTYPECREDITALPHANUM_H

#include "asset.h"

/**
 * Base class for AssetTypeCreditAlphaNum4 and AssetTypeCreditAlphaNum12 subclasses.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/assets.html" target="_blank">Assets</a>
 */
class AssetTypeCreditAlphaNum : public Asset
{

protected:
    QString m_code;
    KeyPair *m_issuer;
public:
    /**
     * @brief AssetTypeCreditAlphaNum
     * @param code
     * @param issuer, issuer is used without making a copy
     */
    AssetTypeCreditAlphaNum(QString code, KeyPair* issuer);
    /**
     * @brief AssetTypeCreditAlphaNum
     * @param code
     * @param issuer, makes a copy
     */
    AssetTypeCreditAlphaNum(QString code, const KeyPair &issuer);
    virtual ~AssetTypeCreditAlphaNum();
    /**
     * Returns asset code
     */
    QString getCode() {
        return m_code;
    }

    /**
     * Returns asset issuer
     */
    const KeyPair &getIssuer();

    uint hashCode();

    bool equals(Asset* object);
};

#endif // ASSETTYPECREDITALPHANUM_H
