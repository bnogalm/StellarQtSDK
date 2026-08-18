#include "sorobancredentialssigner.h"

#include <QDataStream>
#include <stdexcept>

#include "keypair.h"
#include "util.h"
#include "scval/scv.h"
#include "xdr/stellarledgerentries.h"

QSTELLAR_BEGIN_NS


QByteArray SorobanCredentialsSigner::hashedSignaturePayload(
    const stellar::SorobanAuthorizationEntry& entry,
    quint32 validUntilLedger,
    const QString& networkPassphrase)
{
    const bool isV2 =
        entry.credentials.type == stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2;
    if (entry.credentials.type != stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS
        && !isV2) {
        throw std::runtime_error(
            "SorobanCredentialsSigner: only SOROBAN_CREDENTIALS_ADDRESS / _ADDRESS_V2 "
            "entries need signing");
    }

    const QByteArray networkId = Util::hash(networkPassphrase.toUtf8());
    if (networkId.size() != 32) {
        throw std::runtime_error("SorobanCredentialsSigner: networkId hash must be 32 bytes");
    }

    // HashIDPreimage layout.
    //   ENVELOPE_TYPE_SOROBAN_AUTHORIZATION (= 9, CAP-46):
    //     uint32 type, Hash networkID, int64 nonce,
    //     uint32 signatureExpirationLedger, SorobanAuthorizedInvocation invocation
    //   ENVELOPE_TYPE_SOROBAN_AUTHORIZATION_WITH_ADDRESS (= 10, CAP-71):
    //     the same, with SCAddress address inserted between
    //     signatureExpirationLedger and invocation. Binding the signer's
    //     address into the preimage is the whole point of ADDRESS_V2: it stops
    //     a signature being replayed under a different account that happens to
    //     share the private key.
    // Core recomputes this preimage from the entry's OWN
    // signatureExpirationLedger, so signing a different value produces a
    // signature that fails auth on-chain with nothing local to catch it. That
    // is reachable through the documented external-signer flow: hash for
    // ledger Y, stamp the signature in, ship an entry still saying 0. Require
    // the caller to stamp the entry first; authorizeEntry already does.
    if (entry.credentials.address.signatureExpirationLedger != validUntilLedger) {
        throw std::runtime_error(
            "SorobanCredentialsSigner: entry.credentials.address.signatureExpirationLedger "
            "must equal validUntilLedger - stamp the entry before hashing");
    }

    QByteArray xdr;
    QDataStream s(&xdr, QIODevice::WriteOnly);
    s << (isV2 ? stellar::EnvelopeType::ENVELOPE_TYPE_SOROBAN_AUTHORIZATION_WITH_ADDRESS
                : stellar::EnvelopeType::ENVELOPE_TYPE_SOROBAN_AUTHORIZATION);
    s.writeRawData(networkId.constData(), 32);
    s << entry.credentials.address.nonce;
    s << validUntilLedger;
    if (isV2) s << entry.credentials.address.address;
    s << entry.rootInvocation;
    return Util::hash(xdr);
}

stellar::SCVal SorobanCredentialsSigner::buildSignatureSCVal(
    const QByteArray& publicKey32,
    const QByteArray& signature64)
{
    if (publicKey32.size() != 32) {
        throw std::runtime_error("buildSignatureSCVal: public key must be 32 bytes");
    }
    if (signature64.size() != 64) {
        throw std::runtime_error("buildSignatureSCVal: signature must be 64 bytes");
    }

    QList<stellar::SCMapEntry> entries;
    {
        stellar::SCMapEntry e;
        e.key = Scv::toSymbol(QStringLiteral("public_key"));
        e.val = Scv::toBytes(publicKey32);
        entries.append(e);
    }
    {
        stellar::SCMapEntry e;
        e.key = Scv::toSymbol(QStringLiteral("signature"));
        e.val = Scv::toBytes(signature64);
        entries.append(e);
    }
    stellar::SCVal map = Scv::toMap(entries);
    return Scv::toVec(QList<stellar::SCVal>{map});
}

stellar::SorobanAuthorizationEntry SorobanCredentialsSigner::authorizeEntry(
    const stellar::SorobanAuthorizationEntry& entry,
    KeyPair* signer,
    quint32 validUntilLedger,
    const QString& networkPassphrase)
{
    stellar::SorobanAuthorizationEntry clone = entry;
    if (clone.credentials.type
        == stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_WITH_DELEGATES) {
        // A delegate chain cannot be signed by a single keypair; producing an
        // unsigned clone here would look like success and only fail later at
        // simulate. Say so instead.
        throw std::runtime_error(
            "authorizeEntry: SOROBAN_CREDENTIALS_ADDRESS_WITH_DELEGATES is not supported");
    }
    if (clone.credentials.type != stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS
        && clone.credentials.type != stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS_V2) {
        // SOURCE_ACCOUNT credentials are covered by the outer tx signature.
        return clone;
    }
    if (!signer) {
        throw std::runtime_error("authorizeEntry: signer is null");
    }

    clone.credentials.address.signatureExpirationLedger = validUntilLedger;

    const QByteArray payload = hashedSignaturePayload(clone, validUntilLedger, networkPassphrase);
    const QByteArray signature = signer->sign(payload);
    clone.credentials.address.signature =
        buildSignatureSCVal(signer->getPublicKey(), signature);

    return clone;
}
QSTELLAR_END_NS
