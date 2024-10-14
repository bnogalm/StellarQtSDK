#include "account.h"
#include "util.h"
#include "accountconverter.h"

Account::Account(KeyPair *keypair, qint64 sequenceNumber) {
    m_keyPair = checkNotNull(keypair, "keypair cannot be null");
    m_accountId = AccountConverter().encode(keypair->getAccountId());
    m_sequenceNumber = sequenceNumber;
}

Account::Account(QString accountId, qint64 sequenceNumber)
{
    m_keyPair = KeyPair::fromAccountId(AccountConverter(false).filter(accountId));
    m_accountId = AccountConverter().encode(checkNotNull(accountId, "accountId cannot be null"));
    m_sequenceNumber = sequenceNumber;
}

Account::~Account()
{
    if(m_keyPair)
        delete m_keyPair;
}

KeyPair *Account::getKeypair() {
    return m_keyPair;
}

QString Account::getAccountId() const
{
    return AccountConverter().decode(m_accountId);
}

qint64 Account::getSequenceNumber() {
    return m_sequenceNumber;
}

qint64 Account::getIncrementedSequenceNumber() {
    return m_sequenceNumber + 1;
}

void Account::incrementSequenceNumber() {
    m_sequenceNumber++;
}
