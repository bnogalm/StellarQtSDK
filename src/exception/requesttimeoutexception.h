#ifndef QSTELLAR_EXCEPTION_REQUESTTIMEOUTEXCEPTION_H
#define QSTELLAR_EXCEPTION_REQUESTTIMEOUTEXCEPTION_H

#include "networkexception.h"
#include "../responses/problem.h"

namespace qstellar {
namespace exception {

/**
 * @brief Horizon timed the request out: it answered HTTP 504 (Gateway
 * Timeout). A submitted transaction may still be applied.
 *
 * When the SDK itself gives up waiting (Response::replyTimeout()), the error
 * is a ClientTimeoutException instead (connectionerrorexception.h), which is
 * a ConnectionErrorException: this comment used to say it was this class,
 * but it never was. A caller that handles "timed out, the transaction may
 * still land" must catch both.
 */
class RequestTimeoutException : public NetworkException
{
public:
    /** Takes ownership of `problem` (may be nullptr). */
    RequestTimeoutException(int code, const QString& body, Problem* problem = nullptr)
        : NetworkException(QStringLiteral("Request timed out."), code, body),
          m_problem(problem) {}

    ~RequestTimeoutException() override { delete m_problem; }

    RequestTimeoutException(const RequestTimeoutException&) = delete;
    RequestTimeoutException& operator=(const RequestTimeoutException&) = delete;

    const Problem* getProblem() const { return m_problem; }

private:
    Problem* m_problem;
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_REQUESTTIMEOUTEXCEPTION_H
