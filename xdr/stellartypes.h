#ifndef STELLARTYPES_H
#define STELLARTYPES_H
#include <QtGlobal>
#include "xdrhelper.h"


namespace stellar
{
    using namespace xdr;
    typedef quint8 Hash[32];
    XDR_SERIALIZER(Hash)

    typedef quint8 uint256[32];

    enum class CryptoKeyType  : qint32
    {
        KEY_TYPE_ED25519 = 0,
        KEY_TYPE_PRE_AUTH_TX = 1,
        KEY_TYPE_HASH_X = 2,
        // MUXED enum values for supported type are derived from the enum values
        // above by ORing them with 0x100
        KEY_TYPE_MUXED_ED25519 = 0x100
    };
    inline bool isMuxed(CryptoKeyType type)
    {

        return static_cast<qint32>(type)&static_cast<qint32>(CryptoKeyType::KEY_TYPE_MUXED_ED25519);
    }

    enum class PublicKeyType : qint32
    {
        PUBLIC_KEY_TYPE_ED25519 = static_cast<qint32>(CryptoKeyType::KEY_TYPE_ED25519)
    };

    enum class SignerKeyType : qint32
    {
        SIGNER_KEY_TYPE_ED25519 = static_cast<qint32>(CryptoKeyType::KEY_TYPE_ED25519),
        SIGNER_KEY_TYPE_PRE_AUTH_TX = static_cast<qint32>(CryptoKeyType::KEY_TYPE_PRE_AUTH_TX),  // SHA-256 Hash of TransactionSignaturePayload structure
        SIGNER_KEY_TYPE_HASH_X = static_cast<qint32>(CryptoKeyType::KEY_TYPE_HASH_X)
    };

    typedef quint8 Key[32];

    struct PublicKey
    {
        PublicKeyType type;
        Key ed25519;

        static PublicKey from(Key k,PublicKeyType t=PublicKeyType::PUBLIC_KEY_TYPE_ED25519)
        {
            PublicKey pk;
            pk.type=t;
            memcpy(pk.ed25519,k,sizeof(pk.ed25519));
            return pk;
        }
        static PublicKey from(QByteArray k,PublicKeyType t=PublicKeyType::PUBLIC_KEY_TYPE_ED25519)
        {
            PublicKey pk;
            pk.type=t;
            memcpy(pk.ed25519,k.data(),sizeof(pk.ed25519));
            return pk;
        }
    };

    inline QDataStream &operator<<(QDataStream &out, const  PublicKey &obj) {
        out << obj.type;
        switch(obj.type){
        case PublicKeyType::PUBLIC_KEY_TYPE_ED25519:
            out << obj.ed25519; break;
        default: out << obj.ed25519; break;
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  PublicKey &obj) {
        in >> obj.type;
        switch(obj.type){
        case PublicKeyType::PUBLIC_KEY_TYPE_ED25519:
            in >> obj.ed25519; break;
        }
       return in;
    }

    struct SignerKey
    {
        SignerKeyType type;
        union{
        uint256 ed25519;
        uint256 preAuthTx;
        uint256 hashX;
        };

        bool operator==(const SignerKey &other) const
        {
            if(other.type==type)
            {
                switch(type){
                case SignerKeyType::SIGNER_KEY_TYPE_ED25519:
                    return memcmp(ed25519,other.ed25519,sizeof(ed25519))==0;
                case SignerKeyType::SIGNER_KEY_TYPE_PRE_AUTH_TX:
                    return memcmp(preAuthTx,other.preAuthTx,sizeof(preAuthTx))==0;
                case SignerKeyType::SIGNER_KEY_TYPE_HASH_X:
                    return memcmp(hashX,other.hashX,sizeof(hashX))==0;
                }
            }
            return false;
        }
    };
    inline QDataStream &operator<<(QDataStream &out, const  SignerKey &obj) {
        out << obj.type;
        switch(obj.type){
        case SignerKeyType::SIGNER_KEY_TYPE_ED25519:
            out << obj.ed25519; break;
        case SignerKeyType::SIGNER_KEY_TYPE_PRE_AUTH_TX:
            out << obj.preAuthTx; break;
        case SignerKeyType::SIGNER_KEY_TYPE_HASH_X:
            out << obj.hashX; break;
        default: break;
        }
       return out;
    }

    inline QDataStream &operator>>(QDataStream &in,  SignerKey &obj) {
        in >> obj.type;
        switch(obj.type){
        case SignerKeyType::SIGNER_KEY_TYPE_ED25519:
            in >> obj.ed25519; break;
        case SignerKeyType::SIGNER_KEY_TYPE_PRE_AUTH_TX:
            in >> obj.preAuthTx; break;
        case SignerKeyType::SIGNER_KEY_TYPE_HASH_X:
            in >> obj.hashX; break;
        default: break;
        }
       return in;
    }

    typedef Array<quint8,64> Signature;
    //typedef quint8 SignatureHint[4];
    struct SignatureHint{
        quint8 signatureHint[4];
        bool operator==(const SignatureHint& other) const
        {
            return getInt()  ==  other.getInt();
        }
        bool operator<(const SignatureHint& other) const
        {
            return getInt() < other.getInt();
        }
        qint32 getInt() const
        {
            return *((qint32*)signatureHint);
        }
    };



    XDR_SERIALIZER(SignatureHint)
    typedef PublicKey NodeID;


    struct alignas(4) Curve25519Secret
    {
        Key key;
    };
    XDR_SERIALIZER(Curve25519Secret)


    struct alignas(4) Curve25519Public
    {
        Key key;
    };
    XDR_SERIALIZER(Curve25519Public)


    struct alignas(4) HmacSha256Key
    {
        Key key;
    };
    XDR_SERIALIZER(HmacSha256Key)


    struct alignas(4) HmacSha256Mac
    {
        Key mac;
    };
    XDR_SERIALIZER(HmacSha256Mac)


}
#endif // STELLARTYPES_H
