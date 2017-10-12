#include "signer.h"

#include "keypair.h"
#include "transaction.h"


stellar::SignerKey Signer::ed25519PublicKey(KeyPair *keyPair) {
    checkNotNull(keyPair, "keyPair cannot be null");
    return keyPair->getXdrSignerKey();
}

stellar::SignerKey Signer::sha256Hash(QByteArray hash) {
    stellar::SignerKey signerKey;
    checkNotNull(hash, "hash cannot be null");
    signerKey.type = stellar::SignerKeyType::SIGNER_KEY_TYPE_HASH_X;
    Signer::fillUint256(hash,signerKey.hashX);
    return signerKey;
}

stellar::SignerKey Signer::preAuthTx(Transaction* tx) {
    stellar::SignerKey signerKey;
    checkNotNull(tx, "tx cannot be null");
    signerKey.type = stellar::SignerKeyType::SIGNER_KEY_TYPE_PRE_AUTH_TX;
    Signer::fillUint256(tx->hash(),signerKey.preAuthTx);
    return signerKey;
}

stellar::SignerKey Signer::preAuthTx(QByteArray hash) {
    stellar::SignerKey signerKey;
    checkNotNull(hash, "hash cannot be null");
    signerKey.type = stellar::SignerKeyType::SIGNER_KEY_TYPE_PRE_AUTH_TX;
    Signer::fillUint256(hash,signerKey.preAuthTx);
    return signerKey;
}

void Signer::fillUint256(QByteArray hash,  stellar::uint256 &toFill) {
    if (hash.size() != 32) {
        throw std::runtime_error("hash must be 32 bytes long");
    }
    memcpy(toFill,hash.data(),32);
}

void checkNotNull(stellar::SignerKey *key, const char *error)
{
    if(!key)
    {
        throw std::runtime_error(error);
    }
}
