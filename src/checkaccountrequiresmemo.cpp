#include "checkaccountrequiresmemo.h"
#include "keypair.h"
#include <QTimer>


#define MAX_CHECK_ACCOUNT_RETRY 2
/**
 * ACCOUNT_REQUIRES_MEMO_VALUE is the base64 encoding of "1".
 * SEP 29 uses this value to define transaction memo requirements for incoming payments.
 */
const QString ACCOUNT_REQUIRES_MEMO_VALUE = "MQ==";

/**
 * ACCOUNT_REQUIRES_MEMO_KEY is the data name described in SEP 29.
 */
const QString ACCOUNT_REQUIRES_MEMO_KEY = "config.memo_required";

CheckAccountRequiresMemo::CheckAccountRequiresMemo(Server *server, QList<QString> pendingCheckAddressMemos) : QObject(server)
  ,m_pendingCheckAddressMemos(pendingCheckAddressMemos)
  ,m_retry(0)
{

    QTimer::singleShot(0,this,&CheckAccountRequiresMemo::checkNext);
}

Server *CheckAccountRequiresMemo::server() const
{
    return static_cast<Server*>(parent());
}


void CheckAccountRequiresMemo::checkNext()
{
    QString destination = m_pendingCheckAddressMemos.last();
    Server *server = static_cast<Server*>(this->parent());    
    KeyPair * keyCheck = KeyPair::fromAccountId(destination);
    AccountResponse* accountResponse = server->accounts().account(keyCheck);
    delete keyCheck;


    connect(accountResponse, &Response::ready,this, &CheckAccountRequiresMemo::validateAccount);
    connect(accountResponse, &Response::error,this, &CheckAccountRequiresMemo::validateAccountError);

}
bool CheckAccountRequiresMemo::processNotFound(AccountResponse* accountResponse)
{
    if(accountResponse->lastErrorCode()==404|| accountResponse->getStatus()==404)//not found is considered valid destination
    {
        m_pendingCheckAddressMemos.removeLast();
        if(!m_pendingCheckAddressMemos.isEmpty())
            checkNext();
        else
        {
            emit validated();
            this->deleteLater();
        }
        return true;
    }
    return false;
}
void CheckAccountRequiresMemo::validateAccount()
{
    AccountResponse* accountResponse= static_cast<AccountResponse*>(sender());
    accountResponse->deleteLater();
    auto data = accountResponse->getData();
    if(data.get(ACCOUNT_REQUIRES_MEMO_KEY)==ACCOUNT_REQUIRES_MEMO_VALUE)
    {
        //account requires data
        emit error();
        this->deleteLater();
    }
    else if(!processNotFound(accountResponse))//not found is considered valid destination, "valid" errors like 203 are reported, and content is "not found data".
    {
        if(m_pendingCheckAddressMemos.size()<=1)
        {
            emit validated();
            this->deleteLater();
        }
        else
        {
            m_pendingCheckAddressMemos.removeLast();

            checkNext();
        }
    }

}

void CheckAccountRequiresMemo::validateAccountError()
{
    AccountResponse* accountResponse= static_cast<AccountResponse*>(sender());

    accountResponse->deleteLater();    
    if(!processNotFound(accountResponse))//not found is considered valid destination
    {
        m_retry++;
        if(m_retry>=MAX_CHECK_ACCOUNT_RETRY)
        {
            emit error();
            this->deleteLater();
        }
        else{
            checkNext();
        }
    }

}
