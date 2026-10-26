#include "messageauth.h"

#include <QCryptographicHash>
#include <memory>
#include <stdexcept>

#include "keypair.h"

QSTELLAR_BEGIN_NS

QByteArray MessageAuth::messageHash(const QByteArray& message)
{
    QByteArray combined;
    combined.append(PREFIX);
    combined.append(message);
    return QCryptographicHash::hash(combined, QCryptographicHash::Sha256);
}

QByteArray MessageAuth::signMessage(const QByteArray& message, KeyPair* keyPair)
{
    if (!keyPair) {
        throw std::runtime_error("keyPair cannot be null");
    }
    return keyPair->sign(messageHash(message));
}

bool MessageAuth::verifyMessage(const QString& accountId,
                                const QByteArray& message,
                                const QByteArray& signature)
{
    std::unique_ptr<KeyPair> kp(KeyPair::fromAccountId(accountId));
    return kp->verify(messageHash(message), signature);
}

QByteArray MessageAuth::signMessage(const QString& message, KeyPair* keyPair)
{
    return signMessage(message.toUtf8(), keyPair);
}

QByteArray MessageAuth::signMessage(const char* message, KeyPair* keyPair)
{
    return signMessage(QByteArray(message), keyPair);
}

bool MessageAuth::verifyMessage(const QString& accountId, const QString& message, const QByteArray& signature)
{
    return verifyMessage(accountId, message.toUtf8(), signature);
}

bool MessageAuth::verifyMessage(const QString& accountId, const char* message, const QByteArray& signature)
{
    return verifyMessage(accountId, QByteArray(message), signature);
}

QSTELLAR_END_NS
