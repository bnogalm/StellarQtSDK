#ifndef QSTELLAR_EXCEPTION_BADRESPONSEEXCEPTION_H
#define QSTELLAR_EXCEPTION_BADRESPONSEEXCEPTION_H

#include "networkexception.h"
#include "../responses/problem.h"

namespace qstellar {
namespace exception {

/**
 * @brief HTTP 5xx — server-side error.
 *
 * If the response body was application/problem+json the parsed Problem is
 * attached and owned by this exception.
 */
class BadResponseException : public NetworkException
{
public:
    /** Takes ownership of `problem` (may be nullptr). */
    BadResponseException(int code, const QString& body, Problem* problem)
        : NetworkException(QStringLiteral("Bad Response."), code, body),
          m_problem(problem) {}

    ~BadResponseException() override { delete m_problem; }

    BadResponseException(const BadResponseException&) = delete;
    BadResponseException& operator=(const BadResponseException&) = delete;

    const Problem* getProblem() const { return m_problem; }

private:
    Problem* m_problem;
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_BADRESPONSEEXCEPTION_H
