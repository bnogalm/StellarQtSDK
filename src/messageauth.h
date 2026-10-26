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
 * KeyPair::signMessage / verifyMessage do the same from a keypair, as js's
 * and py's Keypair.signMessage / sign_message.
 *
 * Each call takes bytes (QByteArray), text (QString, signed as UTF-8) or a
 * C string (its bytes, which for UTF-8 source text is the same).
 */
class MessageAuth
{
public:
    /** SEP-53's domain-separation prefix: "Stellar Signed Message:\n". */
    static constexpr const char* PREFIX = "Stellar Signed Message:\n";

    /** Returns the ed25519 signature (64 bytes) over the SEP-53 hash. */
    static QByteArray signMessage(const QByteArray& message, KeyPair* keyPair);
    /** A text message is signed as its UTF-8 bytes, as js and py sign a string. */
    static QByteArray signMessage(const QString& message, KeyPair* keyPair);
    static QByteArray signMessage(const char* message, KeyPair* keyPair);

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
    /** A text message is verified as its UTF-8 bytes. */
    static bool verifyMessage(const QString& accountId,
                              const QString& message,
                              const QByteArray& signature);
    static bool verifyMessage(const QString& accountId,
                              const char* message,
                              const QByteArray& signature);

    /** SEP-53 domain-separated hash: SHA-256("Stellar Signed Message:\n" || message). */
    static QByteArray messageHash(const QByteArray& message);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(MessageAuth)

#endif // MESSAGEAUTH_H
