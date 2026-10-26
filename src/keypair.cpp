#include "keypair.h"
#include "util.h"
#include <string.h>

#include "external/ed25519/src/ed25519.h"

#include <QDebug>
#include <cstring>
#include <stdexcept>
#include <QDateTime>
#include <QRandomGenerator>
#include "slip10.h"
#include "messageauth.h"
#include <algorithm>


// memset_s is a free function (NOT a member). Keep it in global scope so
// it does not conflict with namespacing.
//
// Secure wipe: write through a volatile pointer byte-by-byte so the compiler
// cannot elide the store as dead (the buffer is freed right after). The old
// implementation called plain memset() and only touched byte 0 through a
// volatile cast, which left the optimizer free to drop the whole memset.
void memset_s(char * data,int value, int size)
{
    if (!data || size <= 0) return;
    volatile char * p = reinterpret_cast<volatile char *>(data);
    while (size--)
        *p++ = static_cast<char>(value);
}

// Best-effort secure wipe of a QByteArray holding secret material. Only wipes
// in place when this is the sole owner: calling data() on a shared (COW) buffer
// would detach and deep-copy, wiping just the copy while the real bytes survive.
static void wipeSecret(QByteArray & secret)
{
    if (secret.isEmpty()) return;
    if (secret.isDetached())
        memset_s(secret.data(), 0, static_cast<int>(secret.size()));
    secret.clear();
}

QSTELLAR_BEGIN_NS

KeyPair* checkNotNull(KeyPair* keypair, const char *error)
{
    if(!keypair|| ! keypair->m_publicKey){
        throw std::runtime_error(error);
    }
    return keypair;
}

KeyPair::KeyPair():m_publicKey(nullptr),m_privateKey(nullptr)
{

}

// FIX §1.7: deep copy (delegates to the raw-pointer ctor).
KeyPair::KeyPair(const KeyPair &keypair):KeyPair(keypair.m_publicKey,keypair.m_privateKey){
    m_secretSeed = keypair.m_secretSeed;
}

// FIX §1.7: assignment op (Rule of Three). Without this, the default
// shallow copy caused double-free when releasing raw pointers.
KeyPair& KeyPair::operator=(const KeyPair &other) {
    if (this == &other) return *this;
    // Release the current state
    if (m_publicKey) {
        delete[] m_publicKey;
        m_publicKey = nullptr;
    }
    if (m_privateKey) {
        memset_s((char*)m_privateKey, 0, keyLength*2);
        delete[] m_privateKey;
        m_privateKey = nullptr;
    }
    // Deep copy
    if (other.m_publicKey) {
        m_publicKey = new quint8[keyLength];
        memcpy(m_publicKey, other.m_publicKey, keyLength);
    }
    if (other.m_privateKey) {
        m_privateKey = new quint8[keyLength*2];
        memcpy(m_privateKey, other.m_privateKey, keyLength*2);
    }
    wipeSecret(m_secretSeed);
    m_secretSeed = other.m_secretSeed;
    return *this;
}

KeyPair::~KeyPair()
{
    if(m_publicKey) delete[] m_publicKey;
    if(m_privateKey)
    {
        memset_s((char*)m_privateKey,0,keyLength*2);
        delete[] m_privateKey;
    }
    wipeSecret(m_secretSeed);
}


KeyPair::KeyPair(const quint8 *publicKey,const quint8 *privateKey)
{
    m_publicKey = new quint8[keyLength];
    memcpy(m_publicKey,publicKey,keyLength);
    if(privateKey){
        m_privateKey = new quint8[keyLength*2];
        memcpy(m_privateKey,privateKey,keyLength*2);
    }
    else{
        m_privateKey=nullptr;
    }
}

KeyPair::KeyPair(QString accountId)
{
    QByteArray decoded = StrKey::decodeStellarAccountId(accountId);
    m_publicKey = new quint8[keyLength];
    memcpy(m_publicKey,decoded.data(),keyLength);
    m_privateKey=nullptr;
}

bool KeyPair::canSign() {

    return m_privateKey != nullptr;
}

KeyPair *KeyPair::fromSecretSeed(QString seed) {
    QByteArray charSeed = seed.toLatin1();
    QByteArray decoded;
    try {
        decoded = StrKey::decodeStellarSecretSeed(charSeed);
    } catch (...) {
        charSeed.fill('\0');
        throw;
    }
    charSeed.fill('\0');
    KeyPair* keypair = fromSecretSeed(decoded);
    decoded.fill('\0');   // the keypair holds its own copy
    return keypair;
}

KeyPair* KeyPair::fromSecretSeed(QByteArray seed) {

    if(seed.size()!=keyLength){
        // Not repeated: it may be a seed.
        if (seed.size() == 56 && seed.startsWith('S'))
            throw std::runtime_error("this is a secret seed's strkey (S...): read it with fromSecretSeed(QString)");
        throw std::runtime_error("a raw secret seed is 32 bytes");
    }
    quint8 publicKey[keyLength] ;
    quint8 privateKey[keyLength*2] ;
    memset(publicKey,0,keyLength);
    memset(privateKey,0,keyLength*2);



    ed25519_create_keypair((uchar *) publicKey,(uchar *)privateKey, (uchar*)seed.data());
    auto kp = new KeyPair(publicKey, privateKey);
    // Its own copy, not the caller's buffer shared: a caller that wipes its
    // seed (random(), fromMnemonic) wipes its own bytes, not the key's.
    kp->m_secretSeed = QByteArray(seed.constData(), seed.size());
    memset_s((char*)privateKey,0,keyLength*2);//yeah, for sure nobody will never ever show the private key anywhere... we should convert all the QByteArray and QString to secure versions, but still it may be converted to other memory shape that will not be cleaned
    return  kp;
}

KeyPair *KeyPair::fromAccountId(QString accountId) {
    QByteArray decoded = StrKey::decodeStellarAccountId(accountId);
    return fromPublicKey(decoded);
}

KeyPair *KeyPair::fromPublicKey(QByteArray publicKey) {
    // The ctor memcpys 32 bytes: with a shorter QByteArray it read past the
    // end of the buffer and left a half-formed key, with no warning at all.
    if (publicKey.size() != 32) {
        throw std::runtime_error("public key must be exactly 32 bytes");
    }
    return new KeyPair((quint8*)publicKey.data());
}

KeyPair* KeyPair::fromMnemonic(const QString& words, const QString& passphrase, int accountNumber,
                               Util::SupportedMnemonicLanguages language)
{
    // SEP-5 accounts are 0 to 2^31 - 1. A negative int reached
    // fromBip39Seed's "top bit already hardened" rule and came back as
    // another account.
    if (accountNumber < 0)
        throw std::runtime_error("an account number is 0 to 2^31 - 1");
    QByteArray seed = Util::mnemonicToBIP39Seed(words, passphrase, language);
    KeyPair* kp = KeyPair::fromBip39Seed(seed, accountNumber);
    seed.fill('\0');
    return kp;
}

KeyPair *KeyPair::fromBip39Seed(QByteArray bip39Seed, const QString& path)
{
    const QStringList parts = path.split(QLatin1Char('/'));
    if (parts.isEmpty() || parts.first() != QLatin1String("m"))
        throw std::runtime_error("a derivation path starts with \"m\", as in m/44'/148'/0'");
    QVector<int> indexes;
    for (int i = 1; i < parts.size(); ++i) {
        QString part = parts.at(i);
        if (!part.endsWith(QLatin1Char('\'')) && !part.endsWith(QLatin1Char('H')))
            throw std::runtime_error("ed25519 derives hardened children only: write each index as n' (or nH)");
        part.chop(1);
        bool ok = false;
        const qint64 index = part.toLongLong(&ok);
        // Digits only: toLongLong takes a sign and spaces too.
        const bool digits = !part.isEmpty() && std::all_of(part.cbegin(), part.cend(),
                                                           [](QChar c) { return c >= QLatin1Char('0') && c <= QLatin1Char('9'); });
        if (!ok || !digits || index >= 0x80000000LL)
            throw std::runtime_error("a derivation index is a number below 2^31");
        indexes.append(int(index));
    }
    if (bip39Seed.size() < 16 || bip39Seed.size() > 64)
        throw std::runtime_error("BIP-39 seed must be 16 to 64 bytes (BIP-39 gives 64)");
    QByteArray key = SLIP10::deriveEd25519PrivateKey(bip39Seed, indexes);
    KeyPair* keypair = KeyPair::fromSecretSeed(key);
    key.fill('\0');
    return keypair;
}

KeyPair *KeyPair::fromBip39Seed(QByteArray bip39Seed, int accountNumber) {
    // (An accountNumber with the top bit set is taken as already hardened:
    // 100 | 0x80000000 is account 100, as in the Java SDK.)
    // BIP-32/SLIP-10 seeds are 128 to 512 bits (BIP-39's are 512). An empty
    // or truncated one, e.g. from a hex decode that failed, derived a key
    // anybody can derive from the same nothing.
    if (bip39Seed.size() < 16 || bip39Seed.size() > 64)
        throw std::runtime_error("BIP-39 seed must be 16 to 64 bytes (BIP-39 gives 64)");
    QByteArray key = SLIP10::deriveEd25519PrivateKey(bip39Seed, QVector<int>()<< 44<< 148<< accountNumber);
    KeyPair* keypair = KeyPair::fromSecretSeed(key);
    key.fill('\0');   // the account's secret; the keypair holds its own copy
    return keypair;
}


KeyPair *KeyPair::random() {
    // FIX §2.8: use the system RNG (crypto-secure). The old comment
    // promised manual entropy mixing that the implementation never did.
    QByteArray seed;
    seed.resize(keyLength);
    QRandomGenerator *randomDevice = QRandomGenerator::system();
    randomDevice->fillRange((quint32*)seed.data(), keyLength/sizeof(quint32));
    KeyPair* key = fromSecretSeed(seed);
    seed.fill('\0');   // the key holds its own copy
    return key;
}

KeyPair *KeyPair::random(QByteArray rand)
{
    if(rand.size()!=32)
        throw std::runtime_error("rand should be 32 random bytes");
    QByteArray seed;
    seed.resize(keyLength);
    QRandomGenerator *randomDevice = QRandomGenerator::system();
    randomDevice->fillRange((quint32*)seed.data(), keyLength/sizeof(quint32));
    for(int i=0;i<keyLength;i++){
        seed[i] = rand[i] ^ seed[i];
    }
    KeyPair* key = fromSecretSeed(seed);
    seed.fill('\0');   // the key holds its own copy
    return key;
}

QByteArray KeyPair::signMessage(const QByteArray& message)
{
    return MessageAuth::signMessage(message, this);
}

QByteArray KeyPair::signMessage(const QString& message)
{
    return MessageAuth::signMessage(message, this);
}

QByteArray KeyPair::signMessage(const char* message)
{
    return MessageAuth::signMessage(message, this);
}

bool KeyPair::verifyMessage(const QByteArray& message, const QByteArray& signature) const
{
    return MessageAuth::verifyMessage(getAccountId(), message, signature);
}

bool KeyPair::verifyMessage(const QString& message, const QByteArray& signature) const
{
    return MessageAuth::verifyMessage(getAccountId(), message, signature);
}

bool KeyPair::verifyMessage(const char* message, const QByteArray& signature) const
{
    return MessageAuth::verifyMessage(getAccountId(), message, signature);
}

QString KeyPair::getAccountId() const {
    return StrKey::encodeStellarAccountId(QByteArray((const char*)m_publicKey,keyLength));
}

QString KeyPair::getSecretSeed() const {
    return StrKey::encodeStellarSecretSeed(m_secretSeed);
}

QByteArray KeyPair::getPublicKey() const {
    return QByteArray((char*)m_publicKey,keyLength );
}

stellar::SignatureHint KeyPair::getSignatureHint() {
    // FIX §2.6: the silent try/catch returned an uninitialized hint,
    // producing signatures with garbage hint bytes. Let exceptions propagate.
    QByteArray publicKeyBytesStream;
    QDataStream xdrOutputStream(&publicKeyBytesStream, QIODevice::WriteOnly);
    xdrOutputStream << this->getXdrPublicKey();
    QByteArray signatureHintBytes = publicKeyBytesStream.mid(publicKeyBytesStream.length()-4, 4);
    stellar::SignatureHint signatureHint;
    memcpy(signatureHint.signatureHint, signatureHintBytes.data(), 4);
    return signatureHint;
}

stellar::PublicKey KeyPair::getXdrPublicKey() {
    stellar::PublicKey publicKey;
    publicKey.type = stellar::PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
    memcpy(publicKey.ed25519,m_publicKey,keyLength);
    return publicKey;
}

stellar::MuxedAccount KeyPair::getXdrMutexPublicKey()
{
    stellar::MuxedAccount mutexPublicKey;
    mutexPublicKey.type = stellar::CryptoKeyType::KEY_TYPE_ED25519;
    memcpy(mutexPublicKey.ed25519,m_publicKey,keyLength);
    return mutexPublicKey;
}

stellar::SignerKey KeyPair::getXdrSignerKey() {
    stellar::SignerKey signerKey;
    signerKey.type = stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519;
    memcpy(signerKey.ed25519,m_publicKey,keyLength);
    return signerKey;
}

KeyPair *KeyPair::fromXdrPublicKey(const stellar::PublicKey &key) {
    // PublicKey currently only has type=PUBLIC_KEY_TYPE_ED25519.
    if (key.type != stellar::PublicKeyType::PUBLIC_KEY_TYPE_ED25519)
        throw std::runtime_error("unsupported public key type");
    return new KeyPair(key.ed25519);
}

KeyPair *KeyPair::fromXdrMutexPublicKey(const stellar::MuxedAccount &key)
{
    // FIX §2.4: discriminate on the union tag. Muxed bytes live in
    // key.med25519.ed25519; reading key.ed25519 for a muxed input yielded
    // the wrong destination account.
    switch (key.type) {
    case stellar::CryptoKeyType::KEY_TYPE_ED25519:
        return new KeyPair(key.ed25519);
    case stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519:
        // muxedId is dropped here — KeyPair models the base account only.
        // Use MuxedAccount (arriving in 0.8.0) when the muxedId matters.
        return new KeyPair(key.med25519.ed25519);
    default:
        throw std::runtime_error("unsupported muxed account key type");
    }
}

KeyPair *KeyPair::fromXdrSignerKey(const stellar::SignerKey key) {
    // FIX §2.5: PRE_AUTH_TX / HASH_X aren't ed25519 accounts; deriving a
    // G-strkey from their bytes was identity confusion. Unified SignerKey
    // class is coming in 0.7.0; for now only ED25519 is accepted.
    if (key.type != stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519)
        throw std::runtime_error("KeyPair only supports SIGNER_KEY_TYPE_ED25519; use SignerKey for PRE_AUTH_TX/HASH_X");
    return new KeyPair(key.ed25519);
}

QByteArray KeyPair::sign(QByteArray data) {
    if (!m_privateKey) {
        throw std::runtime_error("KeyPair does not contain secret key. Use KeyPair::fromSecretSeed method to create a new KeyPair with a secret key.");
    }
    // FIX §2.7: ed25519_sign is C and doesn't throw; the try/catch only
    // hid std::bad_alloc from the QByteArray. Dropped.
    quint8 signature[64];
    memset(signature, 0, 64);
    ed25519_sign(signature, (const uchar*) data.constData(), data.length(), this->m_publicKey, this->m_privateKey);
    return QByteArray((char*)signature, 64);
}

stellar::DecoratedSignature KeyPair::signDecorated(QByteArray data) {
    QByteArray signatureBytes = this->sign(data);
    stellar::Signature signature;
    signature.set((uchar*)signatureBytes.data(),signatureBytes.length());


    stellar::DecoratedSignature decoratedSignature;
    decoratedSignature.hint = this->getSignatureHint();
    decoratedSignature.signature = signature;
    return decoratedSignature;
}

stellar::DecoratedSignature KeyPair::signPayloadDecorated(QByteArray signerPayload) {
    stellar::DecoratedSignature signature = this->signDecorated(signerPayload);
    // CAP-40: hint = pubkey_hint XOR last 4 bytes of payload (zero-padded).
    quint8 payloadTail[4] = {0, 0, 0, 0};
    const int n = signerPayload.size();
    if (n >= 4) {
        std::memcpy(payloadTail,
                    signerPayload.constData() + (n - 4),
                    4);
    } else if (n > 0) {
        std::memcpy(payloadTail, signerPayload.constData(), n);
    }
    for (int i = 0; i < 4; ++i) {
        signature.hint.signatureHint[i] = signature.hint.signatureHint[i] ^ payloadTail[i];
    }
    return signature;
}

bool KeyPair::verify(QByteArray data, QByteArray signature) {
    // An ed25519 signature is exactly 64 bytes. A longer one verified with
    // its tail ignored, so a padded blob passed as the signature (js and py
    // answer false for any other length).
    if (signature.size() != 64)
        return false;
    return ed25519_verify((uchar*)signature.data(),(uchar*)data.data(),data.length(),this->m_publicKey);
}

bool KeyPair::equals(const KeyPair *obj) const{
    if (!obj) return false;
    if ((obj->m_privateKey && !this->m_privateKey) || (!obj->m_privateKey && this->m_privateKey))
        return false;
    if (obj->m_privateKey && this->m_privateKey)
    {
        // FIX §2.3: m_privateKey is keyLength*2 bytes (ed25519 expanded
        // private key = secret + pubkey). Comparing just keyLength gave
        // false positives.
        if (memcmp(obj->m_privateKey, this->m_privateKey, KeyPair::keyLength * 2) != 0)
            return false;
    }
    if (obj->m_publicKey && this->m_publicKey)
    {
        if (memcmp(obj->m_publicKey, this->m_publicKey, KeyPair::keyLength) != 0)
            return false;
    }
    return true;
}

bool KeyPair::operator==(const KeyPair &other) const
{
    return this->equals(&other);
}

QSTELLAR_END_NS
