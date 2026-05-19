#ifndef QSTELLAR_EXCEPTION_INVALIDSEP10CHALLENGEEXCEPTION_H
#define QSTELLAR_EXCEPTION_INVALIDSEP10CHALLENGEEXCEPTION_H

#include "sdkexception.h"

namespace qstellar {
namespace exception {

/** @brief Raised when a SEP-10 challenge fails validation. */
class InvalidSep10ChallengeException : public SdkException
{
public:
    explicit InvalidSep10ChallengeException(const QString& message)
        : SdkException(message) {}
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_INVALIDSEP10CHALLENGEEXCEPTION_H
