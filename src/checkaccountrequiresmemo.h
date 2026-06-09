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
