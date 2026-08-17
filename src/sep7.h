#ifndef SEP7_H
#define SEP7_H

#include <utility>

#include <QByteArray>
#include <QList>
#include <QString>

#include "qstellar_namespace.h"

QSTELLAR_FWD(KeyPair)

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
 */
class Sep7
{
public:
    static const QString SCHEME;            // "web+stellar"
    static const QString SIGNATURE_PREFIX;  // "stellar.sep.7 - URI Scheme"

    /** Builds the signing payload for a given (signature-stripped) URI. */
    static QByteArray signaturePayload(const QString& uriWithoutSignature);
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
    QString assetCode;          // empty for native
    QString assetIssuer;
    QString memo;               // already encoded for the URI (text / id / base64)
    QString memoType;           // MEMO_TEXT / MEMO_ID / MEMO_HASH / MEMO_RETURN
    QString callback;           // bare URL (the "url:" prefix is added on output)
    QString msg;                // <= 300 chars
    QString networkPassphrase;
    QString originDomain;
    QString signature;          // base64; set by sign()

    QString toUri() const;
    static Sep7Pay fromUri(const QString& uri);

    void sign(KeyPair* signer);                 // sets `signature`
    bool verify(KeyPair* signingKey) const;     // checks `signature`
};

/** `web+stellar:tx` request. */
class Sep7Tx
{
public:
    explicit Sep7Tx(QString envelopeXdrBase64);

    QString xdr;                // base64 TransactionEnvelope
    QList<Sep7Replacement> replace;
    QString callback;
    QString pubkey;
    QString msg;
    QString networkPassphrase;
    QString originDomain;
    QString signature;

    QString toUri() const;
    static Sep7Tx fromUri(const QString& uri);

    void sign(KeyPair* signer);
    bool verify(KeyPair* signingKey) const;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(Sep7)
QSTELLAR_ALIAS(Sep7Pay)
QSTELLAR_ALIAS(Sep7Tx)
QSTELLAR_ALIAS(Sep7Replacement)

#endif // SEP7_H
