#ifndef STRKEY_H
#define STRKEY_H
#include <QtGlobal>
#include <QString>
#include <QtEndian>
#include <exception>
#include <QDataStream>


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


    enum class VersionByte : quint8 {
        ACCOUNT_ID = (6 << 3), // G
        SEED = (18 << 3), // S
        PRE_AUTH_TX = (19 << 3), // T
        SHA256_HASH = (23 << 3) // X
    };


    static QString encodeStellarAccountId(QByteArray data) {
        QByteArray encoded = encodeCheck(VersionByte::ACCOUNT_ID, data);
        return QString::fromLatin1(encoded);
    }

    static QByteArray decodeStellarAccountId(QString data) {
        return decodeCheck(VersionByte::ACCOUNT_ID, data.toLatin1());
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
