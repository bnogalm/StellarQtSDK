#ifndef QSTELLAR_EXCEPTION_ACCOUNTREQUIRESMEMOEXCEPTION_H
#define QSTELLAR_EXCEPTION_ACCOUNTREQUIRESMEMOEXCEPTION_H

#include "sdkexception.h"

namespace qstellar {
namespace exception {

/**
 * @brief SEP-29 — destination account requires a memo and none was set.
 *
 * @see https://github.com/stellar/stellar-protocol/blob/master/ecosystem/sep-0029.md
 */
class AccountRequiresMemoException : public SdkException
{
public:
    AccountRequiresMemoException(const QString& message,
                                 const QString& accountId,
                                 int operationIndex)
        : SdkException(message), m_accountId(accountId), m_operationIndex(operationIndex) {}

    QString getAccountId()     const { return m_accountId; }
    int     getOperationIndex() const { return m_operationIndex; }

private:
    QString m_accountId;
    int     m_operationIndex;
};

} // namespace exception
} // namespace qstellar

#endif // QSTELLAR_EXCEPTION_ACCOUNTREQUIRESMEMOEXCEPTION_H
