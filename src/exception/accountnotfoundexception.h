#ifndef QSTELLAR_EXCEPTION_ACCOUNTNOTFOUNDEXCEPTION_H
#define QSTELLAR_EXCEPTION_ACCOUNTNOTFOUNDEXCEPTION_H

#include "networkexception.h"

namespace qstellar {
namespace exception {

/** @brief Account requested via /accounts/{id} does not exist (HTTP 404). */
class AccountNotFoundException : public NetworkException
{
public:
    explicit AccountNotFoundException(const QString& accountId)
        : NetworkException(QStringLiteral("Account not found, accountId: ") + accountId, 404, QString()),
          m_accountId(accountId) {}

    QString getAccountId() const { return m_accountId; }

private:
    QString m_accountId;
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_ACCOUNTNOTFOUNDEXCEPTION_H
