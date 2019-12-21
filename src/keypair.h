#ifndef KEYPAIR_H
#define KEYPAIR_H

#include <QtGlobal>
#include "strkey.h"
#include "xdr/stellartransaction.h"
class Opertion;

class KeyPair
{

    const static int keyLength = 32;
    quint8 *m_publicKey;
    quint8 *m_privateKey;
    QByteArray m_secretSeed;
    friend KeyPair* checkNotNull(KeyPair* keypair, const char *error);
    friend class Operation;
public:
    KeyPair();
    /**
     * @brief KeyPair constructor to make a copy
     * @param keypair
     */
    KeyPair(KeyPair &keypair);

    ~KeyPair();


    /**
     * Creates a new KeyPair from the given public and private keys.
     * or if privateKey not defined (null) creates a KeyPair to verify signatures from a public address
     * @param publicKey
     * @param privateKey
     */
    KeyPair(quint8* publicKey,quint8* privateKey=nullptr);

    /** Creates a new KeyPair from the given strkey encoded Stellar account ID.
     * Some requesters and API classes requires KeyPair class, if KeyPair is not going to be used any longer, this constructor allow to build a keypair on the stack
     * @param publicKey encoded
     * */
    KeyPair(QString accountId);
    /**
      * Returns true if this Keypair is capable of signing
      */
     bool canSign();


     /**
      * <strong>Insecure</strong> Creates a new Stellar KeyPair from a strkey encoded Stellar secret seed.
      * This method is <u>insecure</u>. Use only if you are aware of security implications.
      * @see <a href="http://docs.oracle.com/javase/1.5.0/docs/guide/security/jce/JCERefGuide.html#PBEEx" target="_blank">Using Password-Based Encryption</a>
      * @param seed The strkey encoded Stellar secret seed.
      * @return {@link KeyPair} You take ownership of the destruction of this object
      */
     static KeyPair* fromSecretSeed(QString seed);

     /**
      * Creates a new Stellar keypair from a raw 32 byte secret seed.
      * @param seed The 32 byte secret seed.
      * @return {@link KeyPair}
      */
     static KeyPair *fromSecretSeed(QByteArray seed);

     /**
      * Creates a new Stellar KeyPair from a strkey encoded Stellar account ID.
      * @param accountId The strkey encoded Stellar account ID.
      * @return {@link KeyPair} You take ownership of the destruction of this object
      */
     static KeyPair* fromAccountId(QString accountId);

     /**
      * Creates a new Stellar keypair from a 32 byte address.
      * @param publicKey The 32 byte public key.
      * @return {@link KeyPair} You take ownership of the destruction of this object
      */
     static KeyPair* fromPublicKey(QByteArray publicKey);

     /**
      * Finds the KeyPair for the path m/44'/148'/accountNumber' using the method described in
      * <a href="https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0005.md">SEP-0005</a>.
      *
      * @param bip39Seed     The output of BIP0039
      * @param accountNumber The number of the account
      * @return KeyPair with secret
      */
     static KeyPair *fromBip39Seed(QByteArray bip39Seed, int accountNumber);

     /**
      * Generates a random Stellar keypair.
      * DON'T USE IT, just for prototyping or tests
      * This method is not recomended. You should mix std::random_device values with some other accesible noise.
      * @return a random Stellar keypair. You take ownership of the destruction of this object
      */
     static KeyPair* random();
     /**
      * @brief random
      * @param rand 32 random bytes. rand will be mixed with std::random_device, it should be safer
      * You should generate a new rand for each account. To generate it you can use any source like sensors or direct human input, but take care about devices without
      * such sensors.
      * @return a random Stellar keypair. You take ownership of the destruction of this object
      */
     static KeyPair* random(QByteArray rand);


     /**
      * Returns the human readable account ID encoded in strkey.
      */
     QString getAccountId() const;

     /**
      * Returns the human readable secret seed encoded in strkey.
      */
     QString getSecretSeed() const;

     QByteArray getPublicKey() const;

     stellar::SignatureHint getSignatureHint();

     stellar::PublicKey getXdrPublicKey();

     stellar::SignerKey getXdrSignerKey();

     static KeyPair* fromXdrPublicKey(stellar::PublicKey& key);

     static KeyPair* fromXdrSignerKey(stellar::SignerKey key);

     /**
      * Sign the provided data with the keypair's private key.
      * @param data The data to sign.
      * @return signed bytes, null if the private key for this keypair is null.
      */
     QByteArray sign(QByteArray data);

     /**
      * Sign the provided data with the keypair's private key and returns {@link DecoratedSignature}.
      * @param data
      */
     stellar::DecoratedSignature signDecorated(QByteArray data);

     /**
      * Verify the provided data and signature match this keypair's public key.
      * @param data The data that was signed.
      * @param signature The signature.
      * @return True if they match, false otherwise.
      * @throws RuntimeException
      */
     bool verify(QByteArray data, QByteArray signature);

     bool equals(const KeyPair* obj) const;

     bool operator==(const KeyPair &other) const;

};

KeyPair* checkNotNull(KeyPair* keypair, const char *error);


#endif // KEYPAIR_H
