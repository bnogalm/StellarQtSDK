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
    if (entry.credentials.type != stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS) {
        throw std::runtime_error(
            "SorobanCredentialsSigner: only SOROBAN_CREDENTIALS_ADDRESS entries need signing");
    }

    const QByteArray networkId = Util::hash(networkPassphrase.toUtf8());
    if (networkId.size() != 32) {
        throw std::runtime_error("SorobanCredentialsSigner: networkId hash must be 32 bytes");
    }

    // HashIDPreimage::SorobanAuthorization layout (CAP-46):
    //   uint32 type (= ENVELOPE_TYPE_SOROBAN_AUTHORIZATION = 9)
    //   Hash   networkID
    //   int64  nonce
    //   uint32 signatureExpirationLedger
    //   SorobanAuthorizedInvocation invocation
    QByteArray xdr;
    QDataStream s(&xdr, QIODevice::WriteOnly);
    s << stellar::EnvelopeType::ENVELOPE_TYPE_SOROBAN_AUTHORIZATION;
    s.writeRawData(networkId.constData(), 32);
    s << entry.credentials.address.nonce;
    s << validUntilLedger;
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
    if (clone.credentials.type != stellar::SorobanCredentialsType::SOROBAN_CREDENTIALS_ADDRESS) {
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
