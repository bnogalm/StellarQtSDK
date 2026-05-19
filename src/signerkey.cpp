#include "signerkey.h"

#include <cstring>
#include <stdexcept>

#include "keypair.h"
#include "strkey.h"

SignerKey::SignerKey()
{
    m_xdr.type = stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519;
    std::memset(m_xdr.ed25519, 0, sizeof(m_xdr.ed25519));
}

SignerKey SignerKey::ed25519PublicKey(QString accountId)
{
    QByteArray raw = StrKey::decodeStellarAccountId(accountId);
    if (raw.size() != 32) {
        throw std::runtime_error("invalid ed25519 account id");
    }
    stellar::SignerKey xdr;
    xdr.type = stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519;
    std::memcpy(xdr.ed25519, raw.constData(), 32);
    return SignerKey(xdr);
}

SignerKey SignerKey::ed25519PublicKey(KeyPair* keyPair)
{
    if (!keyPair) throw std::runtime_error("keyPair cannot be null");
    return SignerKey(keyPair->getXdrSignerKey());
}

SignerKey SignerKey::preAuthTx(QByteArray hash)
{
    if (hash.size() != 32) throw std::runtime_error("preAuthTx hash must be 32 bytes");
    stellar::SignerKey xdr;
    xdr.type = stellar::SignerKeyType::SIGNER_KEY_TYPE_PRE_AUTH_TX;
    std::memcpy(xdr.preAuthTx, hash.constData(), 32);
    return SignerKey(xdr);
}

SignerKey SignerKey::sha256Hash(QByteArray hash)
{
    if (hash.size() != 32) throw std::runtime_error("sha256 hash must be 32 bytes");
    stellar::SignerKey xdr;
    xdr.type = stellar::SignerKeyType::SIGNER_KEY_TYPE_HASH_X;
    std::memcpy(xdr.hashX, hash.constData(), 32);
    return SignerKey(xdr);
}

SignerKey SignerKey::ed25519SignedPayload(QString accountId, QByteArray payload)
{
    QByteArray raw = StrKey::decodeStellarAccountId(accountId);
    if (raw.size() != 32) {
        throw std::runtime_error("invalid ed25519 account id");
    }
    if (payload.isEmpty() || payload.size() > 64) {
        throw std::runtime_error("signed payload length must be in [1, 64]");
    }
    stellar::SignerKey xdr;
    xdr.type = stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD;
    std::memcpy(xdr.ed25519SignedPayload.ed25519, raw.constData(), 32);
    xdr.ed25519SignedPayload.payloadLen = static_cast<quint32>(payload.size());
    std::memset(xdr.ed25519SignedPayload.payload, 0, sizeof(xdr.ed25519SignedPayload.payload));
    std::memcpy(xdr.ed25519SignedPayload.payload, payload.constData(), payload.size());
    return SignerKey(xdr);
}

SignerKey SignerKey::fromXdr(const stellar::SignerKey& xdr)
{
    return SignerKey(xdr);
}

SignerKey SignerKey::fromString(QString strkey)
{
    if (strkey.isEmpty()) throw std::runtime_error("empty strkey");
    QChar prefix = strkey.at(0);
    if (prefix == 'G') {
        return ed25519PublicKey(strkey);
    } else if (prefix == 'T') {
        return preAuthTx(StrKey::decodePreAuthTx(strkey));
    } else if (prefix == 'X') {
        return sha256Hash(StrKey::decodeSha256Hash(strkey));
    } else if (prefix == 'P') {
        auto pair = StrKey::decodeSignedPayload(strkey);
        // Build the ed25519 account id strkey from the raw key, then use the factory.
        QString accountId = StrKey::encodeStellarAccountId(pair.first);
        return ed25519SignedPayload(accountId, pair.second);
    }
    throw std::runtime_error("unsupported strkey prefix");
}

QString SignerKey::toString() const
{
    switch (m_xdr.type) {
    case stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519:
        return StrKey::encodeStellarAccountId(
            QByteArray(reinterpret_cast<const char*>(m_xdr.ed25519), 32));
    case stellar::SignerKeyType::SIGNER_KEY_TYPE_PRE_AUTH_TX:
        return StrKey::encodePreAuthTx(
            QByteArray(reinterpret_cast<const char*>(m_xdr.preAuthTx), 32));
    case stellar::SignerKeyType::SIGNER_KEY_TYPE_HASH_X:
        return StrKey::encodeSha256Hash(
            QByteArray(reinterpret_cast<const char*>(m_xdr.hashX), 32));
    case stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD:
        return StrKey::encodeSignedPayload(
            QByteArray(reinterpret_cast<const char*>(m_xdr.ed25519SignedPayload.ed25519), 32),
            QByteArray(reinterpret_cast<const char*>(m_xdr.ed25519SignedPayload.payload),
                       static_cast<int>(m_xdr.ed25519SignedPayload.payloadLen)));
    }
    throw std::runtime_error("unknown signer key type");
}

QByteArray SignerKey::getEd25519() const
{
    if (m_xdr.type == stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519) {
        return QByteArray(reinterpret_cast<const char*>(m_xdr.ed25519), 32);
    }
    if (m_xdr.type == stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD) {
        return QByteArray(
            reinterpret_cast<const char*>(m_xdr.ed25519SignedPayload.ed25519), 32);
    }
    throw std::runtime_error("SignerKey is not an ed25519 variant");
}

QByteArray SignerKey::getPreAuthTx() const
{
    if (m_xdr.type != stellar::SignerKeyType::SIGNER_KEY_TYPE_PRE_AUTH_TX) {
        throw std::runtime_error("SignerKey is not preAuthTx");
    }
    return QByteArray(reinterpret_cast<const char*>(m_xdr.preAuthTx), 32);
}

QByteArray SignerKey::getHashX() const
{
    if (m_xdr.type != stellar::SignerKeyType::SIGNER_KEY_TYPE_HASH_X) {
        throw std::runtime_error("SignerKey is not hashX");
    }
    return QByteArray(reinterpret_cast<const char*>(m_xdr.hashX), 32);
}

QByteArray SignerKey::getSignedPayload() const
{
    if (m_xdr.type != stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519_SIGNED_PAYLOAD) {
        throw std::runtime_error("SignerKey is not signedPayload");
    }
    return QByteArray(reinterpret_cast<const char*>(m_xdr.ed25519SignedPayload.payload),
                      static_cast<int>(m_xdr.ed25519SignedPayload.payloadLen));
}
