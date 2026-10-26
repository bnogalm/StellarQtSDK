#ifndef ASSET_H
#define ASSET_H
#include <exception>
#include <stdexcept>
#include <string>
#include "xdr/stellarledgerentries.h"
#include "stellaraddress.h"
#include "keypair.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * An asset code that is not one the network takes: 1 to 4 (credit_alphanum4)
 * or 5 to 12 (credit_alphanum12) ASCII letters and digits (stellar-core's
 * isAssetValid; js and py refuse the same). A std::runtime_error, so what()
 * says which code and why (it was a bare std::exception: "Unknown
 * exception").
 */
class AssetCodeLengthInvalidException: public std::runtime_error
{
public:
    explicit AssetCodeLengthInvalidException(const std::string& message =
        "an asset code is 1 to 4 (credit_alphanum4) or 5 to 12 (credit_alphanum12) ASCII letters and digits")
        : std::runtime_error(message) {}
};

/**
 * Throws AssetCodeLengthInvalidException unless `code` is `min` to `max`
 * ASCII letters and digits. Only the length was checked: "US D", "US$" and
 * "\u00e9" were signed, and core refuses them as malformed.
 */
inline void checkAssetCode(const QString& code, int min, int max, const char* type)
{
    bool ok = code.size() >= min && code.size() <= max;
    for (const QChar c : code) {
        const ushort u = c.unicode();
        ok = ok && ((u >= '0' && u <= '9') || (u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z'));
    }
    if (!ok)
        throw AssetCodeLengthInvalidException(
            QStringLiteral("a %1 asset code is %2 to %3 ASCII letters and digits, not \"%4\"")
                .arg(QLatin1String(type)).arg(min).arg(max).arg(code).toStdString());
}

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
    /**
     * The asset Horizon names by type, code and issuer: "native" with no
     * code and no issuer (empty or null), "credit_alphanum4" with a code of
     * 1 to 4 ASCII letters or digits, or "credit_alphanum12" with one of 5 to
     * 12, and the issuer's account id. Throws std::runtime_error naming the
     * type and the code for anything else (and whatever KeyPair::fromAccountId
     * throws for an issuer that is not an account id): every type but
     * "native" used to become the asset the code's length made, so
     * "credit_alphanum12" with "USD" was an alphanum4 asset, and a type
     * nobody knows an issued asset.
     */
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

    /**
     * CAP-46-7 — returns the SAC (Stellar Asset Contract) 32-byte contract id
     * for this asset on the given network. Equivalent to Java SDK's
     * `Asset::toContractId(networkPassphrase)`.
     */
    QByteArray toSAC(const QString& networkPassphrase);

    /** Convenience: returns the SAC contract id wrapped in a C-strkey StellarAddress. */
    StellarAddress toSACAddress(const QString& networkPassphrase);
};

Asset* checkNotNull(Asset* asset, const char *error);

QSTELLAR_END_NS

QSTELLAR_ALIAS(Asset)
QSTELLAR_ALIAS(AssetCodeLengthInvalidException)

#endif // ASSET_H
