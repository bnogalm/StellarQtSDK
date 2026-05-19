#ifndef QSTELLAR_EXCEPTION_CONNECTIONERROREXCEPTION_H
#define QSTELLAR_EXCEPTION_CONNECTIONERROREXCEPTION_H

#include "networkexception.h"

namespace qstellar {
namespace exception {

/** @brief Connection-level failure (DNS, refused, TLS handshake, …). */
class ConnectionErrorException : public NetworkException
{
public:
    explicit ConnectionErrorException(const QString& details = QString())
        : NetworkException(QStringLiteral("Connection error: %1").arg(details), -1, QString()) {}
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_CONNECTIONERROREXCEPTION_H
