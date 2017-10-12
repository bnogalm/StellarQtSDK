#ifndef SIGNER_H
#define SIGNER_H
#include "xdr/stellarledger.h"
class KeyPair;
class Transaction;


/**
 * Signer is a helper class that creates {@link org.stellar.sdk.xdr.SignerKey} objects.
 */
class Signer
{
public:
    /**
     * Create <code>ed25519PublicKey</code> {@link org.stellar.sdk.xdr.SignerKey} from
     * a {@link org.stellar.sdk.KeyPair}
     * @param keyPair
     * @return org.stellar.sdk.xdr.SignerKey
     */
    static stellar::SignerKey ed25519PublicKey(KeyPair* keyPair);

    /**
     * Create <code>sha256Hash</code> {@link org.stellar.sdk.xdr.SignerKey} from
     * a sha256 hash of a preimage.
     * @param hash
     * @return org.stellar.sdk.xdr.SignerKey
     */
    static stellar::SignerKey sha256Hash(QByteArray hash);

    /**
     * Create <code>preAuthTx</code> {@link org.stellar.sdk.xdr.SignerKey} from
     * a {@link org.stellar.sdk.xdr.Transaction} hash.
     * @param tx
     * @return org.stellar.sdk.xdr.SignerKey
     */
    static stellar::SignerKey preAuthTx(Transaction *tx);

    /**
     * Create <code>preAuthTx</code> {@link org.stellar.sdk.xdr.SignerKey} from
     * a transaction hash.
     * @param hash
     * @return org.stellar.sdk.xdr.SignerKey
     */
    static stellar::SignerKey preAuthTx(QByteArray hash);

    static void fillUint256(QByteArray hash, stellar::uint256 &toFill);
};

void checkNotNull(stellar::SignerKey* key,const char *error);
#endif // SIGNER_H
