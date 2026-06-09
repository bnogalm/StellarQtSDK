#ifndef SIGNER_H
#define SIGNER_H
#include "xdr/stellarledger.h"
#include "keypair.h"
#include "qstellar_namespace.h"

// Transaction is not yet migrated into qstellar::; forward-declare it
// in global scope so the signature `preAuthTx(Transaction*)` below stays
// referring to ::Transaction.
QSTELLAR_FWD(Transaction)

QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(Signer)
// The Horizon JSON-deserialize type with the same name lives in
// `AccountResponseAttach::Signer` (responses/accountresponse.h), so the
// global alias is safe.
#ifndef STELLAR_QT_LEGACY_GLOBAL_NS
using ::qstellar::checkNotNull;
#endif

#endif // SIGNER_H
