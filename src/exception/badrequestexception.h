#ifndef QSTELLAR_EXCEPTION_BADREQUESTEXCEPTION_H
#define QSTELLAR_EXCEPTION_BADREQUESTEXCEPTION_H

#include "networkexception.h"
#include "../responses/problem.h"

namespace qstellar {
namespace exception {

/**
 * @brief HTTP 4xx (typically 400) — client-side error.
 *
 * If the response body was application/problem+json the parsed Problem is
 * attached and owned by this exception.
 */
class BadRequestException : public NetworkException
{
public:
    /** Takes ownership of `problem` (may be nullptr). */
    BadRequestException(int code, const QString& body, Problem* problem)
        : NetworkException(QStringLiteral("Bad Request."), code, body),
          m_problem(problem) {}

    ~BadRequestException() override { delete m_problem; }

    BadRequestException(const BadRequestException&) = delete;
    BadRequestException& operator=(const BadRequestException&) = delete;

    /** Borrowed pointer; lifetime tied to the exception. */
    const Problem* getProblem() const { return m_problem; }

private:
    Problem* m_problem;
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_BADREQUESTEXCEPTION_H
