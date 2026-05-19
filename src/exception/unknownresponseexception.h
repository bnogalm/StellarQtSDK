#ifndef QSTELLAR_EXCEPTION_UNKNOWNRESPONSEEXCEPTION_H
#define QSTELLAR_EXCEPTION_UNKNOWNRESPONSEEXCEPTION_H

#include "networkexception.h"

namespace qstellar {
namespace exception {

/**
 * @brief Server returned an unparseable / unexpected response.
 *
 * Used both for:
 *  - HTTP status codes without a specific handler (e.g. 418, 451, ...).
 *  - 2xx responses whose body could not be parsed as the expected JSON.
 */
class UnknownResponseException : public NetworkException
{
public:
    UnknownResponseException(int code, const QString& body)
        : NetworkException(QStringLiteral("Unknown response from server: [%1] %2").arg(code).arg(body),
                           code, body) {}
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_UNKNOWNRESPONSEEXCEPTION_H
