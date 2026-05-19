#ifndef QSTELLAR_EXCEPTION_REQUESTTIMEOUTEXCEPTION_H
#define QSTELLAR_EXCEPTION_REQUESTTIMEOUTEXCEPTION_H

#include "networkexception.h"
#include "../responses/problem.h"

namespace qstellar {
namespace exception {

/**
 * @brief Request timed out.
 *
 * Raised when:
 *  - the server returned HTTP 504 (Gateway Timeout); or
 *  - the local QNetworkReply timed out client-side.
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
