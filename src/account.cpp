#include "account.h"
#include "util.h"
#include "accountconverter.h"
#include <limits>
#include <stdexcept>

QSTELLAR_BEGIN_NS

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

// At INT64_MAX the +1 overflowed, and build() signed sequence INT64_MIN.
qint64 Account::getIncrementedSequenceNumber() {
    if (m_sequenceNumber == std::numeric_limits<qint64>::max())
        throw std::runtime_error("the account sequence number is at its maximum "
                                 "(9223372036854775807): no further transaction can be built for it");
    return m_sequenceNumber + 1;
}

void Account::incrementSequenceNumber() {
    if (m_sequenceNumber == std::numeric_limits<qint64>::max())
        throw std::runtime_error("the account sequence number is at its maximum "
                                 "(9223372036854775807)");
    ++m_sequenceNumber;
}

void Account::decrementSequenceNumber() {
    if (m_sequenceNumber == std::numeric_limits<qint64>::min())
        throw std::runtime_error("the account sequence number is at its minimum");
    --m_sequenceNumber;
}

QSTELLAR_END_NS
