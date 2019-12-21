#include "keypair.h"
#include <string.h>

#include "external/ed25519/src/ed25519.h"

#include <QDebug>
#include <cstring>
#include <QDateTime>
#include <QRandomGenerator>
#include "slip10.h"


void memset_s(char * data,int value, int size)
{
    memset(data,value,size);
    *(volatile char *)data = *(volatile char *)data;
}

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

KeyPair::KeyPair(KeyPair &keypair):KeyPair(keypair.m_publicKey,keypair.m_privateKey){

}

KeyPair::~KeyPair()
{
    if(m_publicKey) delete[] m_publicKey;
    if(m_privateKey)
    {
        memset_s((char*)m_privateKey,0,keyLength*2);
        delete[] m_privateKey;
    }
}


KeyPair::KeyPair(quint8 *publicKey, quint8 *privateKey)
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
    QByteArray decoded = StrKey::decodeStellarSecretSeed(charSeed);
    KeyPair* keypair = fromSecretSeed(decoded);
    return keypair;
}

KeyPair* KeyPair::fromSecretSeed(QByteArray seed) {

    if(seed.size()!=keyLength){
        throw std::runtime_error("you seems using an user encoded string as bytearray seed");
    }
    quint8 publicKey[keyLength] ;
    quint8 privateKey[keyLength*2] ;
    memset(publicKey,0,keyLength);
    memset(privateKey,0,keyLength*2);



    ed25519_create_keypair((uchar *) publicKey,(uchar *)privateKey, (uchar*)seed.data());
    auto kp = new KeyPair(publicKey, privateKey);
    kp->m_secretSeed =seed;
    memset_s((char*)privateKey,0,keyLength*2);//yeah, for sure nobody will never ever show the private key anywhere... we should convert all the QByteArray and QString to secure versions, but still it may be converted to other memory shape that will not be cleaned
    return  kp;
}

KeyPair *KeyPair::fromAccountId(QString accountId) {
    QByteArray decoded = StrKey::decodeStellarAccountId(accountId);
    return fromPublicKey(decoded);
}

KeyPair *KeyPair::fromPublicKey(QByteArray publicKey) {
    return new KeyPair((quint8*)publicKey.data());
}

KeyPair *KeyPair::fromBip39Seed(QByteArray bip39Seed, int accountNumber) {
    return KeyPair::fromSecretSeed(SLIP10::deriveEd25519PrivateKey(bip39Seed, QVector<int>()<< 44<< 148<< accountNumber));
}


KeyPair *KeyPair::random() {
    QByteArray seed;
    seed.resize(keyLength);
    //you MUST mix random generated keypair with some other source of random as random_device is not random in many platforms
    //and even if they are randoms you shouldnt trust anybody... so mix them, if they are random, they will stay random.
    QRandomGenerator randomDevice = QRandomGenerator::securelySeeded();
    randomDevice.fillRange((quint32*)seed.data(),keyLength/sizeof(quint32));
    return fromSecretSeed(seed);
}

KeyPair *KeyPair::random(QByteArray rand)
{
    if(rand.size()!=32)
        throw std::runtime_error("rand should be 32 random bytes");
    QByteArray seed;
    seed.resize(keyLength);
    QRandomGenerator randomDevice = QRandomGenerator::securelySeeded();
    randomDevice.fillRange((quint32*)seed.data(),keyLength/sizeof(quint32));
    for(int i=0;i<keyLength;i++){
        seed[i] = rand[i] ^ seed[i];
    }
    return fromSecretSeed(seed);
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
    try {
        QByteArray publicKeyBytesStream;
        QDataStream xdrOutputStream(&publicKeyBytesStream,QIODevice::WriteOnly);
        xdrOutputStream<< this->getXdrPublicKey();
        QByteArray signatureHintBytes = publicKeyBytesStream.mid(publicKeyBytesStream.length()-4,4);
        stellar::SignatureHint signatureHint;
        memcpy(signatureHint.signatureHint,signatureHintBytes.data(),4);
        return signatureHint;
    } catch (std::exception e) {
        //throw new AssertionError(e);
    }
    return stellar::SignatureHint();
}

stellar::PublicKey KeyPair::getXdrPublicKey() {
    stellar::PublicKey publicKey;
    publicKey.type = stellar::PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
    memcpy(publicKey.ed25519,this->getPublicKey().data(),keyLength);
    return publicKey;
}

stellar::SignerKey KeyPair::getXdrSignerKey() {
    stellar::SignerKey signerKey;
    signerKey.type = stellar::SignerKeyType::SIGNER_KEY_TYPE_ED25519;
    memcpy(signerKey.ed25519,this->getPublicKey().data(),keyLength);
    return signerKey;
}

KeyPair *KeyPair::fromXdrPublicKey(stellar::PublicKey& key) {
    return new KeyPair(key.ed25519);
}

KeyPair *KeyPair::fromXdrSignerKey(stellar::SignerKey key) {
    return new KeyPair(key.ed25519);
}

QByteArray KeyPair::sign(QByteArray data) {
    if (!m_privateKey) {
        throw std::runtime_error("KeyPair does not contain secret key. Use KeyPair::fromSecretSeed method to create a new KeyPair with a secret key.");
    }
    try {
        quint8 signature[64];
        memset(signature,0,64);
        ed25519_sign(signature,(const uchar*) data.constData(), data.length(), this->m_publicKey, this->m_privateKey);
        return QByteArray((char*)signature,64);
    } catch (std::exception e) {
        throw std::runtime_error("error signing");
    }
    return QByteArray();
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

bool KeyPair::verify(QByteArray data, QByteArray signature) {
    if(signature.size()>=64)
        return ed25519_verify((uchar*)signature.data(),(uchar*)data.data(),data.length(),this->m_publicKey);
    return false;
}

bool KeyPair::equals(const KeyPair *obj) const{
    if((obj->m_privateKey && !this->m_privateKey) || (!obj->m_privateKey &&this->m_privateKey))
        return false;
    if(obj->m_privateKey&& this->m_privateKey)
    {
        if(memcmp(obj->m_privateKey,this->m_privateKey,KeyPair::keyLength)!=0)
            return false;
    }
    if(obj->m_publicKey&& this->m_publicKey)
    {
        if(memcmp(obj->m_publicKey,this->m_publicKey,KeyPair::keyLength)!=0)
            return false;
    }
    return true;
}

bool KeyPair::operator==(const KeyPair &other) const
{
    return this->equals(&other);
}
