#ifndef KEYPAIR_H
#define KEYPAIR_H

#include <QtGlobal>
#include "strkey.h"
#include "util.h"
#include "xdr/stellartransaction.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class Opertion;

class KeyPair
{
public:
    const static int keyLength = 32;
private:
    quint8 *m_publicKey;
    quint8 *m_privateKey;
    QByteArray m_secretSeed;
    friend KeyPair* checkNotNull(KeyPair* keypair, const char *error);
    friend class Operation;
public:
    KeyPair();
    /**
     * @brief KeyPair constructor to make a deep copy
     * @param keypair
     */
    // FIX §1.7: const-correct copy ctor + assignment op (Rule of Three).
    KeyPair(const KeyPair &keypair);
    KeyPair& operator=(const KeyPair &other);

    ~KeyPair();


    /**
     * Creates a new KeyPair from the given public and private keys.
     * or if privateKey not defined (null) creates a KeyPair to verify signatures from a public address
     * @param publicKey
     * @param privateKey
     */
    KeyPair(const quint8* publicKey, const quint8 *privateKey=nullptr);

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
      * The keypair of a secret seed in its strkey form (S...), as js's and
      * py's Keypair.fromSecret / from_secret. Throws std::runtime_error on
      * anything else, without repeating it.
      *
      * The seed is the account: whoever reads it controls the funds. Keep it
      * out of logs, messages and source code. The SDK wipes the copies it
      * makes here; a QString it is given cannot be wiped by anyone, so hold
      * seeds as briefly as possible. You own the returned KeyPair.
      */
     static KeyPair* fromSecretSeed(QString seed);

     /**
      * The keypair of a raw 32-byte secret seed (not its S... strkey, which
      * fromSecretSeed(QString) reads; given here, it is refused saying so).
      * The keypair keeps its own copy: the caller may wipe `seed`. You own
      * the returned KeyPair.
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
      * @param bip39Seed     The output of BIP0039 (64 bytes); anything outside
      *                      the 16 to 64 bytes BIP-32 allows throws
      *                      std::runtime_error
      * @param accountNumber The number of the account, 0 to 2^31 - 1. As in
      *                      the Java SDK, a number with the top bit set (a
      *                      negative int) is taken as already hardened:
      *                      100 | 0x80000000 is account 100 and -2^31 is
      *                      account 0. fromMnemonic refuses a negative
      *                      number, and the path overload an index of 2^31
      *                      or more.
      * @return KeyPair with secret
      */
     static KeyPair *fromBip39Seed(QByteArray bip39Seed, int accountNumber);

     /**
      * The key at a SLIP-10 `path` of a BIP-39 seed, for another derivation
      * than SEP-5's m/44'/148'/n' (which fromBip39Seed(seed, n) is): "m"
      * followed by hardened indexes, each written n' or nH, as in
      * "m/44'/148'/0'". Ed25519 has hardened children only, so an index
      * without ' or H is refused rather than silently hardened, as is an
      * index of 2^31 or more. You own the returned KeyPair.
      */
     static KeyPair *fromBip39Seed(QByteArray bip39Seed, const QString& path);

     /**
      * The account `accountNumber` (m/44'/148'/accountNumber') of a BIP-39
      * phrase, as py-stellar-sdk's Keypair.from_mnemonic_phrase: the phrase is
      * checked (every word in the list, 12/15/18/21/24 words, the checksum) and
      * refused with std::runtime_error otherwise; `passphrase` is the optional
      * BIP-39 passphrase. A negative `accountNumber` is refused too: it named
      * another account (-2^31 gave account 0, -1 account 2^31 - 1), and py
      * derives yet another, non-hardened key for it. You own the returned
      * KeyPair.
      */
     static KeyPair *fromMnemonic(const QString& words, const QString& passphrase = QString(),
                                  int accountNumber = 0,
                                  Util::SupportedMnemonicLanguages language = Util::SupportedMnemonicLanguages::EN);

     /**
      * A new keypair from 32 bytes of the operating system's cryptographically
      * secure generator (QRandomGenerator::system(): BCryptGenRandom on
      * Windows, getrandom or /dev/urandom elsewhere), as js's Keypair.random
      * (tweetnacl's randomBytes) and py's Keypair.random (os.urandom). This
      * is the way to create an account's key. You own the returned KeyPair.
      */
     static KeyPair* random();

     /**
      * SEP-53: the signature of `message` (64 bytes), as js's
      * Keypair.signMessage and py's sign_message; text is signed as UTF-8.
      * Needs the secret seed. See MessageAuth.
      */
     QByteArray signMessage(const QByteArray& message);
     QByteArray signMessage(const QString& message);
     QByteArray signMessage(const char* message);
     /**
      * SEP-53: true when `signature` is this key's over `message`. It is
      * verify() over SEP-53's hash of the message, so it is false for the
      * small-order and non-canonical keys and signatures verify() refuses.
      */
     bool verifyMessage(const QByteArray& message, const QByteArray& signature) const;
     bool verifyMessage(const QString& message, const QByteArray& signature) const;
     bool verifyMessage(const char* message, const QByteArray& signature) const;
     /**
      * As random(), with `rand` (exactly 32 bytes) XORed into the system
      * generator's bytes: for a caller that has entropy of its own to add
      * (a hardware generator, dice). The result is never weaker than
      * random(), whatever `rand` holds. Throws on another size. You own the
      * returned KeyPair.
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

     stellar::MuxedAccount getXdrMutexPublicKey();

     stellar::SignerKey getXdrSignerKey();

     static KeyPair* fromXdrPublicKey(const stellar::PublicKey& key);

     static KeyPair* fromXdrMutexPublicKey(const stellar::MuxedAccount &key);

     static KeyPair* fromXdrSignerKey(const stellar::SignerKey key);

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
      * CAP-40 — signs an arbitrary payload and produces a DecoratedSignature
      * whose `hint` is the standard hint XORed with the last 4 bytes of the
      * payload (right-padded with zeros if shorter than 4 bytes).
      */
     stellar::DecoratedSignature signPayloadDecorated(QByteArray signerPayload);

     /**
      * True when `signature` is this key's ed25519 signature of `data`;
      * false otherwise, it does not throw. As stellar-core (libsodium's
      * crypto_sign_verify_detached) it is false for a signature that is not
      * exactly 64 bytes, whose S is not below the group order or whose R is
      * of small order, and for a public key of small order (GAAQ...HV4,
      * GAAAA...WHF and the rest of libsodium's blocklist) or not canonical
      * (y >= p, a second account id for one key). With a small-order key
      * anybody can make a signature that verifies, no secret needed. py
      * 14.1.0 refuses all of these, and js 17.1.0 too except a small-order
      * R: there R = the identity with S = h * a (which only the key holder
      * can make) verifies. Core wins, the network refuses that signature.
      * SEP-7, SEP-10, SEP-45 and SEP-53 verify through here.
      */
     bool verify(QByteArray data, QByteArray signature);

     bool equals(const KeyPair* obj) const;

     bool operator==(const KeyPair &other) const;

};

KeyPair* checkNotNull(KeyPair* keypair, const char *error);

QSTELLAR_END_NS

QSTELLAR_ALIAS(KeyPair)


#endif // KEYPAIR_H
