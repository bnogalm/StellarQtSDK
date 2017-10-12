#include "accountmergeoperationresponse.h"
#include "../../keypair.h"
AccountMergeOperationResponse::AccountMergeOperationResponse(QNetworkReply *reply)
    :OperationResponse(reply)
    ,m_accountKeypair(0)
    ,m_intoKeypair(0)

{

}

AccountMergeOperationResponse::~AccountMergeOperationResponse()
{
    if(m_accountKeypair)
        delete m_accountKeypair;
    if(m_intoKeypair)
        delete m_intoKeypair;
}

QString AccountMergeOperationResponse::account() const
{
    return m_account;
}

QString AccountMergeOperationResponse::into() const
{
    return m_into;
}

KeyPair &AccountMergeOperationResponse::getAccount()
{
    if(!m_accountKeypair)
        m_accountKeypair = KeyPair::fromAccountId(m_account);
    return *m_accountKeypair;
}

KeyPair &AccountMergeOperationResponse::getInto()
{
    if(!m_intoKeypair)
        m_intoKeypair = KeyPair::fromAccountId(m_into);
    return *m_intoKeypair;
}

void AccountMergeOperationResponse::setAccount(QString account)
{
    m_account = account;
}

void AccountMergeOperationResponse::setInto(QString into)
{
    m_into = into;
}
