#ifndef MESSAGEAUTH_H
#define MESSAGEAUTH_H

#include <QByteArray>
#include <QString>

#include "keypair.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * SEP-53 — Stellar Signed Message authentication.
 *
 * The message to be signed/verified is hashed with a domain-separation
 * prefix ("Stellar Signed Message:\n") before being passed to ed25519.
 * This prevents signed messages from being mistaken for transaction
 * signatures.
 *
 * Renamed from `Auth` in 0.9.0 to avoid collision with generic auth
 * frameworks; the static helper class is otherwise unchanged.
 */
class MessageAuth
{
public:
    /** Returns the ed25519 signature (64 bytes) over the SEP-53 hash. */
    static QByteArray signMessage(const QByteArray& message, KeyPair* keyPair);

    /**
     * Verifies a SEP-53 signature.
     * @param accountId G-strkey of the signer
     * @param message original message bytes
     * @param signature 64-byte ed25519 signature
     * @return true iff the signature is valid for the given accountId / message
     */
    static bool verifyMessage(const QString& accountId,
                              const QByteArray& message,
                              const QByteArray& signature);

    /** SEP-53 domain-separated hash: SHA-256("Stellar Signed Message:\n" || message). */
    static QByteArray messageHash(const QByteArray& message);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(MessageAuth)

#endif // MESSAGEAUTH_H
