#ifndef QSTELLAR_EXCEPTION_TOOMANYREQUESTSEXCEPTION_H
#define QSTELLAR_EXCEPTION_TOOMANYREQUESTSEXCEPTION_H

#include "networkexception.h"

namespace qstellar {
namespace exception {

/**
 * @brief HTTP 429 — too many requests.
 *
 * `retryAfter` is the value of the `Retry-After` response header (seconds),
 * or -1 if the header was absent.
 *
 * @see https://developers.stellar.org/docs/data/apis/horizon/api-reference/structure/rate-limiting
 */
class TooManyRequestsException : public NetworkException
{
public:
    explicit TooManyRequestsException(int retryAfter = -1)
        : NetworkException(QStringLiteral("The rate limit for the requesting IP address is over its allowed limit."),
                           429, QString()),
          m_retryAfter(retryAfter) {}

    /** Seconds the client should wait, or -1 if Retry-After was not provided. */
    int getRetryAfter() const { return m_retryAfter; }

private:
    int m_retryAfter;
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_TOOMANYREQUESTSEXCEPTION_H
