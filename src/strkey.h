#ifndef STRKEY_H
#define STRKEY_H
#include <QtGlobal>
#include <QString>
#include <QtEndian>
#include <exception>
#include <stdexcept>
#include <QDataStream>
#include "xdr/stellartransaction.h"

/**
 * Indicates that there was a problem decoding strkey encoded string.
 * @see KeyPair
 */
class FormatException :public std::exception {
};

class StrKey
{
public:
    StrKey();

    const static int ACCOUNT_ID_ADDRESS_LENGTH = 56;
    const static int MUXED_ACCOUNT_ADDRESS_LENGTH  = 69;

    enum class VersionByte : quint8 {
        ACCOUNT_ID = (6 << 3), // G
        MUXED_ACCOUNT = (12 << 3), //M
        SEED = (18 << 3), // S
        PRE_AUTH_TX = (19 << 3), // T
        SHA256_HASH = (23 << 3) // X

    };

    static QString encodeStellarAccountId(QByteArray data) {
        QByteArray encoded = encodeCheck(VersionByte::ACCOUNT_ID, data);
        return QString::fromLatin1(encoded);
    }

    static QString encodeStellarAccountId(const stellar::AccountID& accountID) {

          QByteArray data = QByteArray::fromRawData((const char*)(accountID.ed25519),sizeof(accountID.ed25519));
          return encodeStellarAccountId(data);
    }

    static QString encodeStellarAccountId(const stellar::Key& key) {
          QByteArray data = QByteArray::fromRawData((const char*)(key),sizeof(key));
          return encodeStellarAccountId(data);
    }

    static QString encodeStellarMuxedAccount(const stellar::MuxedAccount& muxedAccount);

    static stellar::AccountID muxedAccountToAccountId(const stellar::MuxedAccount& account) {
        stellar::AccountID accountID;
        accountID.type = stellar::PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
        switch(account.type)
        {
        case stellar::CryptoKeyType::KEY_TYPE_ED25519:
        {            
            memcpy(accountID.ed25519,account.ed25519,sizeof(accountID.ed25519));
            return accountID;
        }
        case stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519:
        {            
            memcpy(accountID.ed25519,account.med25519.ed25519,sizeof(accountID.ed25519));
            return accountID;
        }
        default:
        {
            throw std::runtime_error("invalid muxed account type");
        }
        }

    }

    static stellar::AccountID encodeToXDRAccountId(QString data) {
        stellar::AccountID accountID;
        if (data.length() != ACCOUNT_ID_ADDRESS_LENGTH)
            throw std::runtime_error("invalid address length");
        accountID.type = stellar::PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
        QByteArray decoded = decodeStellarAccountId(data);
        memcpy(accountID.ed25519,decoded.data(),sizeof(accountID.ed25519));
        decoded.fill('\0',decoded.length());
        return accountID;
    }

    static stellar::MuxedAccount encodeToXDRMuxedAccount(QString data) {
        stellar::MuxedAccount muxed;
        if (data.length() == ACCOUNT_ID_ADDRESS_LENGTH) {
            muxed.type= stellar::CryptoKeyType::KEY_TYPE_ED25519;
            QByteArray decoded = decodeStellarAccountId(data);
            memcpy(muxed.ed25519,decoded.data(),sizeof(muxed.ed25519));
            decoded.fill('\0',decoded.length());
            return muxed;        
        } else if (data.length() == MUXED_ACCOUNT_ADDRESS_LENGTH) {
            muxed.type= stellar::CryptoKeyType::KEY_TYPE_MUXED_ED25519;
            QByteArray decoded = decodeStellarMuxedAccount(data);            
            muxed.med25519.id = qFromBigEndian<quint64>(decoded.data()+sizeof(muxed.med25519.ed25519));            
            memcpy(muxed.med25519.ed25519,decoded.data(),sizeof(muxed.med25519.ed25519));
            decoded.fill('\0',decoded.length());
            return muxed;
        }
        throw std::runtime_error("invalid address length");
    }

    //encoded here is the ASCII encoded address, not the binary format
    static VersionByte decodeVersionByte(QByteArray encoded);
    static VersionByte decodeVersionByte(QString encoded);


    static QByteArray decodeStellarAccountId(QString data) {
        return decodeCheck(VersionByte::ACCOUNT_ID, data.toLatin1());
    }

    static QByteArray decodeStellarMuxedAccount(QString data) {
        return decodeCheck(VersionByte::MUXED_ACCOUNT, data.toLatin1());
    }

    static QByteArray encodeStellarSecretSeed(QByteArray data) {
        return encodeCheck(VersionByte::SEED, data);
    }

    static QByteArray decodeStellarSecretSeed(QByteArray data) {
        return decodeCheck(VersionByte::SEED, data);
    }

    static QString encodePreAuthTx(QByteArray data) {
        QByteArray encoded = encodeCheck(VersionByte::PRE_AUTH_TX, data);
        return QString::fromLatin1(encoded);
    }

    static QByteArray decodePreAuthTx(QString data) {
        return decodeCheck(VersionByte::PRE_AUTH_TX, data.toLatin1());
    }

    static QString encodeSha256Hash(QByteArray data) {
        QByteArray encoded = encodeCheck(VersionByte::SHA256_HASH, data);
        return QString::fromLatin1(encoded);
    }

    static QByteArray decodeSha256Hash(QString data) {
        return decodeCheck(VersionByte::SHA256_HASH, data.toLatin1());
    }

    static QByteArray encodeCheck(VersionByte versionByte, QByteArray data);

    static QByteArray decodeCheck(VersionByte versionByte, QByteArray encoded);

    static QByteArray calculateChecksum(QByteArray bytes);
};

#endif // STRKEY_H
