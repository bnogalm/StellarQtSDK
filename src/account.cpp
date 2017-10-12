#include "account.h"
#include "util.h"


Account::Account(KeyPair *keypair, quint64 sequenceNumber) {
    m_keyPair = checkNotNull(keypair, "keypair cannot be null");
    m_sequenceNumber = sequenceNumber;
}

KeyPair *Account::getKeypair() {
    return m_keyPair;
}

quint64 Account::getSequenceNumber() {
    return m_sequenceNumber;
}

quint64 Account::getIncrementedSequenceNumber() {
    return m_sequenceNumber + 1;
}

void Account::incrementSequenceNumber() {
    m_sequenceNumber++;
}
