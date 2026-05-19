#ifndef QSTELLAR_EXCEPTION_NETWORKEXCEPTION_H
#define QSTELLAR_EXCEPTION_NETWORKEXCEPTION_H

#include "sdkexception.h"

namespace qstellar {
namespace exception {

/**
 * @brief Base class for exceptions raised by failed network operations.
 *
 * Holds the HTTP status code (or -1 when not applicable) and the raw body.
 */
class NetworkException : public SdkException
{
public:
    NetworkException(int code, const QString& body)
        : SdkException(), m_code(code), m_body(body) {}

    NetworkException(const QString& message, int code, const QString& body)
        : SdkException(message), m_code(code), m_body(body) {}

    int     getCode() const { return m_code; }
    QString getBody() const { return m_body; }

private:
    int     m_code;
    QString m_body;
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_NETWORKEXCEPTION_H
