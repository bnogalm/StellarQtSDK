#include "createaccountoperationresponse.h"
#include "../../keypair.h"

CreateAccountOperationResponse::CreateAccountOperationResponse(QNetworkReply *reply):OperationResponse(reply)
  ,m_accountKeypair(0),m_funderKeypair(0)
{

}

CreateAccountOperationResponse::~CreateAccountOperationResponse()
{
    if(m_accountKeypair)
        delete m_accountKeypair;
    if(m_funderKeypair)
        delete m_funderKeypair;
}

KeyPair &CreateAccountOperationResponse::getAccount() {
    if(!m_accountKeypair)
        m_accountKeypair = KeyPair::fromAccountId(m_account);
    return *m_accountKeypair;
}

QString CreateAccountOperationResponse::getStartingBalance() const{
    return m_startingBalance;
}

KeyPair &CreateAccountOperationResponse::getFunder() {
    if(!m_funderKeypair)
        m_funderKeypair = KeyPair::fromAccountId(m_funder);
    return *m_funderKeypair;
}

QString CreateAccountOperationResponse::account() const
{
    return m_account;
}

QString CreateAccountOperationResponse::funder() const
{
    return m_funder;
}

void CreateAccountOperationResponse::setAccount(QString account)
{
    //responses can get refilled in stream mode
    if(m_accountKeypair)
    {
        delete m_accountKeypair;
        m_accountKeypair=nullptr;
    }
    m_account = account;
}

void CreateAccountOperationResponse::setFunder(QString funder)
{
    //responses can get refilled in stream mode
    if(m_funderKeypair)
    {
        delete m_funderKeypair;
        m_funderKeypair=nullptr;
    }
    m_funder = funder;   
}
