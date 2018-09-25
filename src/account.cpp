#include "account.h"
#include "util.h"


Account::Account(KeyPair *keypair, qint64 sequenceNumber) {
    m_keyPair = checkNotNull(keypair, "keypair cannot be null");
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

qint64 Account::getSequenceNumber() {
    return m_sequenceNumber;
}

qint64 Account::getIncrementedSequenceNumber() {
    return m_sequenceNumber + 1;
}

void Account::incrementSequenceNumber() {
    m_sequenceNumber++;
}
