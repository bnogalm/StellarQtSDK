#ifndef CHECKACCOUNTREQUIRESMEMO_H
#define CHECKACCOUNTREQUIRESMEMO_H
#include <QObject>
#include "server.h"

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

#endif // CHECKACCOUNTREQUIRESMEMO_H
