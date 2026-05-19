#ifndef QSTELLAR_EXCEPTION_UNEXPECTEDEXCEPTION_H
#define QSTELLAR_EXCEPTION_UNEXPECTEDEXCEPTION_H

#include "sdkexception.h"

namespace qstellar {
namespace exception {

/** @brief Raised on "should never happen" conditions — programming errors. */
class UnexpectedException : public SdkException
{
public:
    explicit UnexpectedException(const QString& message)
        : SdkException(QStringLiteral("Unexpected: ") + message) {}
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_UNEXPECTEDEXCEPTION_H
