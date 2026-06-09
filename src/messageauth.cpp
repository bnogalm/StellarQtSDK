#include "messageauth.h"

#include <QCryptographicHash>
#include <memory>
#include <stdexcept>

#include "keypair.h"

namespace {
const char* kSep53Prefix = "Stellar Signed Message:\n";
}

QSTELLAR_BEGIN_NS

QByteArray MessageAuth::messageHash(const QByteArray& message)
{
    QByteArray combined;
    combined.append(kSep53Prefix);
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

QSTELLAR_END_NS
