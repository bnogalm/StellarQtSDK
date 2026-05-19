#ifndef SIGNERKEY_H
#define SIGNERKEY_H

#include <QByteArray>
#include <QString>

#include "xdr/stellartypes.h"

class KeyPair;

/**
 * Unified value-type SignerKey covering all 4 variants:
 *   ed25519 (account id), pre-auth tx, sha256 hash, ed25519 + signed payload (CAP-40).
 *
 * Each factory returns a fully constructed SignerKey by value. The class is
 * cheap to copy (wraps a POD-like stellar::SignerKey).
 */
class SignerKey
{
    stellar::SignerKey m_xdr;
public:
    SignerKey();
    explicit SignerKey(const stellar::SignerKey& xdr) : m_xdr(xdr) {}

    static SignerKey ed25519PublicKey(QString accountId);
    static SignerKey ed25519PublicKey(KeyPair* keyPair);
    static SignerKey preAuthTx(QByteArray hash);
    static SignerKey sha256Hash(QByteArray hash);
    static SignerKey ed25519SignedPayload(QString accountId, QByteArray payload);
    static SignerKey fromXdr(const stellar::SignerKey& xdr);
    /** Parses any of G / T / X / P strkeys. */
    static SignerKey fromString(QString strkey);

    stellar::SignerKeyType getType() const { return m_xdr.type; }
    stellar::SignerKey toXdr() const { return m_xdr; }

    /** Returns the strkey representation: G / T / X / P. */
    QString toString() const;

    /** Returns the underlying 32-byte ed25519 pubkey for ED25519 / SIGNED_PAYLOAD variants. */
    QByteArray getEd25519() const;
    /** PRE_AUTH_TX variant — 32-byte hash. */
    QByteArray getPreAuthTx() const;
    /** HASH_X variant — 32-byte hash. */
    QByteArray getHashX() const;
    /** SIGNED_PAYLOAD variant — variable-length payload (1..64 bytes). */
    QByteArray getSignedPayload() const;

    bool operator==(const SignerKey& other) const { return m_xdr == other.m_xdr; }
    bool operator!=(const SignerKey& other) const { return !(*this == other); }
};

#endif // SIGNERKEY_H
