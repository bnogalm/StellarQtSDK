#ifndef SEP7_H
#define SEP7_H

#include <utility>

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "qstellar_namespace.h"

QSTELLAR_FWD(KeyPair)
QSTELLAR_FWD(AbstractTransaction)
QSTELLAR_FWD(Network)

QSTELLAR_BEGIN_NS

/**
 * SEP-7 — URI scheme to facilitate delegated signing (`web+stellar:`).
 *
 * Mirrors `py-stellar-sdk`'s `sep/stellar_uri.py` field-for-field so the URIs
 * are interoperable with other SDKs / wallets. Two request types:
 *   - `Sep7Pay`  → `web+stellar:pay?destination=...`
 *   - `Sep7Tx`   → `web+stellar:tx?xdr=...`
 *
 * Both can be signed by the origin domain's `URI_REQUEST_SIGNING_KEY`. The
 * signing payload is `35×0x00 + 0x04 + "stellar.sep.7 - URI Scheme" + uri`
 * (the URI with any `&signature=...` removed), ed25519-signed and base64 in
 * the `signature` query parameter.
 *
 * A request read with fromUri() is verified against the URI exactly as it was
 * received, which is what SEP-7 signs; its `signature` must be the last
 * parameter (nothing after it is covered), and a parameter given twice is
 * refused. fromUri() applies the same rules as toUri(), so verify() answers
 * true or false and never throws.
 *
 * Where this departs from py, on purpose: py has no verify() (it signs
 * its own to_uri()), and verify() here checks the URI received; a raw '+'
 * in a base64 value (xdr, signature, a hash memo) stays a '+' where py
 * reads a space and breaks the value; an empty `memo=` is an empty text
 * memo, which py writes but cannot read back; the destination and the
 * amount, which py takes as any text, are checked (a G... or M..., more
 * than zero); Sep7Tx::fromUri(uri) reads an absent network_passphrase as
 * the public network (SEP-7), where py's from_uri(uri, None) refuses it;
 * a MEMO_ID is decimal digits only, as js Memo.id takes it, where py
 * reads what int() reads ("+5", "%205" and "1_0" are 5, 5 and 10).
 * verifyOrigin() and Sep7OriginCheck have no py counterpart.
 */
class Sep7
{
public:
    static const QString SCHEME;            // "web+stellar"
    static const QString SIGNATURE_PREFIX;  // "stellar.sep.7 - URI Scheme"

    /** Builds the signing payload for a given (signature-stripped) URI. */
    static QByteArray signaturePayload(const QString& uriWithoutSignature);

    /**
     * The origin_domain a `pay` or `tx` request names, empty when none,
     * in its ACE form: an internationalised name comes back as xn--...,
     * the form verifyOrigin() checks and the one to show a user or compare
     * with a known domain. example.com spelled with a Cyrillic a (U+0430)
     * would pass for example.com; its ACE form xn--exmple-4nf.com does
     * not. The `originDomain` field of a parsed request keeps what the URI
     * says, as py. Throws std::runtime_error on what is not such a request.
     */
    static QString originDomain(const QString& uri);

    /**
     * SEP-7's check of a request's origin, given its origin_domain's
     * stellar.toml (read with StellarTomlResolver, or Sep7OriginCheck does
     * both): the request names an origin_domain, which is a fully qualified
     * domain name (SEP-7's step 3), carries a signature, and that signature
     * is the toml's URI_REQUEST_SIGNING_KEY's. Returns the key; throws
     * std::runtime_error saying which of these fails. A wallet shows the
     * origin_domain to its user only once this passed, and as originDomain()
     * returns it (the ACE form), never the Unicode one.
     */
    static QString verifyOrigin(const QString& uri, const QVariantMap& originToml);

    /**
     * True when `domain` is a fully qualified domain name, as SEP-7 asks of
     * an origin_domain: two labels at least (not "localhost"), each 1 to 63
     * letters, digits or '-' not at either end, 253 characters at most, and
     * a last label that is not a number (not an IP address). Nothing else:
     * no port, path, user or trailing dot. An internationalised name is
     * checked in its ACE form (xn--...).
     */
    static bool isFullyQualifiedDomainName(const QString& domain);
};

/** A `replace` entry for a `tx` request (SEP-11 txrep field substitution). */
struct Sep7Replacement
{
    QString field;      // txrep tx field name, e.g. "sourceAccount"
    QString reference;  // reference identifier, e.g. "X"
    QString hint;       // human hint for the reference

    Sep7Replacement() = default;
    Sep7Replacement(QString f, QString r, QString h)
        : field(std::move(f)), reference(std::move(r)), hint(std::move(h)) {}
    bool operator==(const Sep7Replacement& o) const
    { return field == o.field && reference == o.reference && hint == o.hint; }
};

/** `web+stellar:pay` request. */
class Sep7Pay
{
public:
    explicit Sep7Pay(QString destination);

    // Fields (public for a simple value-builder; all optional except destination).
    QString destination;
    QString amount;
    QString assetCode;          // empty for native ("XLM" with no issuer is native too, as py)
    QString assetIssuer;
    QString memo;               // already encoded for the URI (text / id / base64)
    QString memoType;           // MEMO_TEXT / MEMO_ID / MEMO_HASH / MEMO_RETURN
    QString callback;           // bare URL (the "url:" prefix is added on output)
    QString msg;                // <= 300 chars
    QString networkPassphrase;
    QString originDomain;       // as the URI spells it; show Sep7::originDomain(uri)
    QString signature;          // base64; set by sign()

    QString toUri() const;
    static Sep7Pay fromUri(const QString& uri);

    void sign(KeyPair* signer);                 // sets `signature`
    bool verify(KeyPair* signingKey) const;     // checks `signature`

private:
    QString m_received;         // the URI fromUri() read, as received
    QStringList m_receivedFields;
    QStringList fields() const;
};

/** `web+stellar:tx` request. */
class Sep7Tx
{
public:
    explicit Sep7Tx(QString envelopeXdrBase64);

    QString xdr;                // base64 TransactionEnvelope, checked to decode
    QList<Sep7Replacement> replace;
    QString callback;
    QString pubkey;
    QString msg;
    QString networkPassphrase;
    QString originDomain;       // as the URI spells it; show Sep7::originDomain(uri)
    QString signature;

    /** Throws when `xdr` is not a whole TransactionEnvelope (or fee bump). */
    QString toUri() const;
    static Sep7Tx fromUri(const QString& uri);
    /**
     * As py's from_uri(uri, network_passphrase): refused when the URI names
     * another network than `expectedNetworkPassphrase`; a URI that names
     * none is read for the expected network, which networkPassphrase then
     * holds (and toUri() writes, as py's to_uri()). SEP-7 reads an absent
     * network_passphrase as the public network, as fromUri(uri) does. An
     * empty `expectedNetworkPassphrase` is py's None: nothing to check, and
     * a URI that names no network is refused. verify() still checks the URI
     * as received.
     */
    static Sep7Tx fromUri(const QString& uri, const QString& expectedNetworkPassphrase);

    /** The network the envelope is for: network_passphrase, or the public
     *  network when it is absent (SEP-7). */
    QString effectiveNetworkPassphrase() const;

    /**
     * The envelope, decoded for `network`, which the caller keeps alive and
     * which has to be effectiveNetworkPassphrase()'s: a transaction signed
     * for another network than the one the request names is refused. The
     * caller owns the result.
     */
    AbstractTransaction* transaction(Network* network) const;

    void sign(KeyPair* signer);
    bool verify(KeyPair* signingKey) const;

private:
    QString m_received;
    QStringList m_receivedFields;
    QStringList fields() const;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Sep7)
QSTELLAR_ALIAS(Sep7Pay)
QSTELLAR_ALIAS(Sep7Tx)
QSTELLAR_ALIAS(Sep7Replacement)

#endif // SEP7_H
