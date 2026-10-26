#ifndef CHECKACCOUNTREQUIRESMEMO_H
#define CHECKACCOUNTREQUIRESMEMO_H
#include <QObject>
#include "server.h"
#include "qstellar_namespace.h"

QSTELLAR_FWD(AccountResponse)

QSTELLAR_BEGIN_NS

/**
 * @brief The CheckAccountRequiresMemo class
 *
 * helper class to verify a list of accounts if required memo flag is active
 *
 * Server no longer uses it: Server::submitTransaction runs the SEP-29 check
 * itself, and reports which account asked for a memo, or why a lookup
 * failed, which this class's single error() signal could not tell apart.
 * Kept for source compatibility only; new code should not use it.
 *
 * In particular error() cannot say that a lookup timed out: Server reports
 * that as a ClientTimeoutException (no answer within
 * Response::replyTimeout()) or a RequestTimeoutException (Horizon's 504) on
 * the submission's response, and HorizonQml as kind "timeout". An account
 * whose answer cannot be read exactly (see Response::fillObject) is an
 * error() here, never a pass.
 */
class CheckAccountRequiresMemo : public QObject
{
    Q_OBJECT
    QList<QString> m_pendingCheckAddressMemos;
    int m_retry;
    bool processNotFound(AccountResponse *accountResponse);
public:
    explicit CheckAccountRequiresMemo(Server *server, QList<QString> pendingCheckAddressMemos);



    Server * server() const;


signals:

    void validated();
    void error();
private slots:
    void checkNext();
    void validateAccount();
    void validateAccountError();
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(CheckAccountRequiresMemo)

#endif // CHECKACCOUNTREQUIRESMEMO_H
