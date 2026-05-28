#ifndef SOROBANCREDENTIALSSIGNER_H
#define SOROBANCREDENTIALSSIGNER_H

#include <QByteArray>
#include <QString>
#include <QtGlobal>

#include "xdr/sorobanops.h"

class KeyPair;

/**
 * CAP-46 — sign Soroban auth entries that use `SOROBAN_CREDENTIALS_ADDRESS`.
 *
 * Mirrors `Auth.authorizeEntry` in the Java SDK. Given an entry whose
 * credentials are SOROBAN_CREDENTIALS_ADDRESS, this produces the signed
 * variant by:
 *
 *   1. Updating `signatureExpirationLedger`.
 *   2. Building HashIDPreimage::SorobanAuthorization with
 *      { networkID, nonce, signatureExpirationLedger, rootInvocation }.
 *   3. Signing SHA-256(XDR(preimage)) with the supplied KeyPair.
 *   4. Encoding the signature into the canonical SCVal shape:
 *        SCV_VEC([ SCV_MAP({"public_key" → SCV_BYTES, "signature" → SCV_BYTES}) ])
 *   5. Stamping that SCVal back into `credentials.address.signature`.
 *
 * SOROBAN_CREDENTIALS_SOURCE_ACCOUNT entries are returned unchanged — the
 * source account's transaction signature implicitly covers them.
 */
class SorobanCredentialsSigner
{
public:
    /** Sign a copy of `entry` and return the signed entry. */
    static stellar::SorobanAuthorizationEntry authorizeEntry(
        const stellar::SorobanAuthorizationEntry& entry,
        KeyPair* signer,
        quint32 validUntilLedger,
        const QString& networkPassphrase);

    /** Build the payload that needs signing (32-byte SHA-256). Exposed so
     *  callers can route the actual signing through an HSM / external signer. */
    static QByteArray hashedSignaturePayload(
        const stellar::SorobanAuthorizationEntry& entry,
        quint32 validUntilLedger,
        const QString& networkPassphrase);

    /** Assemble the SCVal signature payload from a raw 64-byte ed25519
     *  signature + 32-byte public key. */
    static stellar::SCVal buildSignatureSCVal(const QByteArray& publicKey32,
                                              const QByteArray& signature64);
};

#endif // SOROBANCREDENTIALSSIGNER_H
