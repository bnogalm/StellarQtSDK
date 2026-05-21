#include "auth.h"

#include <QCryptographicHash>
#include <memory>
#include <stdexcept>

#include "keypair.h"

namespace {
const char* kSep53Prefix = "Stellar Signed Message:\n";
}

QByteArray Auth::messageHash(const QByteArray& message)
{
    QByteArray combined;
    combined.append(kSep53Prefix);
    combined.append(message);
    return QCryptographicHash::hash(combined, QCryptographicHash::Sha256);
}

QByteArray Auth::signMessage(const QByteArray& message, KeyPair* keyPair)
{
    if (!keyPair) {
        throw std::runtime_error("keyPair cannot be null");
    }
    return keyPair->sign(messageHash(message));
}

bool Auth::verifyMessage(const QString& accountId,
                         const QByteArray& message,
                         const QByteArray& signature)
{
    std::unique_ptr<KeyPair> kp(KeyPair::fromAccountId(accountId));
    return kp->verify(messageHash(message), signature);
}
