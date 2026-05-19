#include "transactionbuilder.h"
#include "transaction.h"
#include "util.h"
#include <QDateTime>
#include <QDebug>
#include <limits>

quint32 TransactionBuilder::s_defaultOperationFee = TransactionBuilder::BASE_FEE;

TransactionBuilder::TransactionBuilder(AccountConverter accountConverter,
                                       TransactionBuilderAccount* sourceAccount,
                                       Network* network)
{
    m_accountConverter = accountConverter;
    m_sourceAccount = checkNotNull(sourceAccount, "sourceAccount cannot be null");
    m_memo = nullptr;
    m_timeBounds = nullptr;
#ifdef STELLAR_QT_AUTOSET_BASE_FEE
    m_baseFee = s_defaultOperationFee;
#else
    m_baseFee = 0;
#endif
    m_timeoutSet = false;
    m_network = network;
}

// Java-aligned constructor: muxed accounts always on, Network explicit.
TransactionBuilder::TransactionBuilder(TransactionBuilderAccount* sourceAccount, Network* network)
    : TransactionBuilder(AccountConverter().enableMuxed(),
                         sourceAccount,
                         checkNotNull(network, "network cannot be null"))
{
}

TransactionBuilder::TransactionBuilder(TransactionBuilder& other)
    : m_accountConverter(other.m_accountConverter)
    , m_sourceAccount(other.m_sourceAccount)
    , m_network(other.m_network)
    , m_memo(other.m_memo)
    , m_timeBounds(other.m_timeBounds)
    , m_operations(other.m_operations)
    , m_timeoutSet(other.m_timeoutSet)
    , m_baseFee(other.m_baseFee)
{
    other.m_memo = nullptr;
    other.m_timeBounds = nullptr;
    other.m_operations.clear();
}

TransactionBuilder::~TransactionBuilder()
{
    // Does not destroy the source account.
    if (m_memo)
        delete m_memo;
    if (m_timeBounds)
        delete m_timeBounds;
    for (Operation* o : m_operations) {
        delete o;
    }
}

int TransactionBuilder::getOperationsCount()
{
    return m_operations.size();
}

void TransactionBuilder::setDefaultOperationFee(quint32 opFee)
{
    if (opFee < TransactionBuilder::BASE_FEE) {
        throw std::runtime_error(
            QString("DefaultOperationFee cannot be smaller than the BASE_FEE (\" %1 \"): %2")
                .arg(TransactionBuilder::BASE_FEE).arg(opFee).toStdString());
    }
    s_defaultOperationFee = opFee;
}

TransactionBuilder& TransactionBuilder::addOperation(Operation* operation)
{
    checkNotNull(operation, "operation cannot be null");
    m_operations.append(operation);
    return *this;
}

TransactionBuilder& TransactionBuilder::addMemo(Memo* memo)
{
    if (this->m_memo) {
        throw std::runtime_error("Memo has been already added.");
    }
    m_memo = checkNotNull(memo, "memo cannot be null");
    return *this;
}

TransactionBuilder& TransactionBuilder::addTimeBounds(TimeBounds* timeBounds)
{
    if (this->m_timeBounds) {
        throw std::runtime_error("TimeBounds has been already added.");
    }
    checkNotNull(reinterpret_cast<intptr_t>(timeBounds), "timeBounds cannot be null");
    m_timeBounds = timeBounds;
    m_timeoutSet = true;
    return *this;
}

TransactionBuilder& TransactionBuilder::setTimeout(qint64 timeout)
{
    if (m_timeBounds && m_timeBounds->getMaxTime() > 0) {
        throw std::runtime_error("TimeBounds.max_time has been already set - setting timeout would overwrite it.");
    }
    if (timeout < 0) {
        throw std::runtime_error("timeout cannot be negative");
    }
    m_timeoutSet = true;
    if (timeout > 0) {
        qint64 timeoutTimestamp = QDateTime::currentMSecsSinceEpoch() / 1000L + timeout;
        if (!m_timeBounds) {
            m_timeBounds = new TimeBounds(0, timeoutTimestamp);
        } else {
            qint64 min = m_timeBounds->getMinTime();
            delete m_timeBounds;
            m_timeBounds = new TimeBounds(min, timeoutTimestamp);
        }
    }
    return *this;
}

TransactionBuilder& TransactionBuilder::setBaseFee(quint32 baseFee)
{
    if (baseFee < TransactionBuilder::BASE_FEE) {
        throw std::runtime_error(
            QString("BaseFee cannot be smaller than the BASE_FEE (\" %1 \"): %2")
                .arg(TransactionBuilder::BASE_FEE).arg(baseFee).toStdString());
    }
    m_baseFee = baseFee;
    return *this;
}

Transaction* TransactionBuilder::build()
{
    // Ensure setTimeout was called or maxTime is set.
    if ((!m_timeBounds || (m_timeBounds && m_timeBounds->getMaxTime() == 0)) && !m_timeoutSet) {
        throw std::runtime_error("TimeBounds has to be set or you must call setTimeout(TIMEOUT_INFINITE).");
    }

    if (m_baseFee == 0) {
#ifdef STELLAR_QT_AUTOSET_BASE_FEE
        qDebug() << "[TransactionBuilder] The `baseFee` parameter of `TransactionBuilder` is required. Setting to BASE_FEE="
                 << TransactionBuilder::BASE_FEE
                 << ". Future versions of this library will error if not provided.";
        m_baseFee = TransactionBuilder::BASE_FEE;
#else
        throw std::runtime_error("The `baseFee` parameter of `TransactionBuilder` is required.");
#endif
    }

    // FIX §7.1: promote to qint64 to avoid 32-bit overflow in
    // totalFee = numOps * baseFee.
    if (m_operations.length() > 0
        && m_baseFee > static_cast<quint32>(std::numeric_limits<qint64>::max() / m_operations.length())) {
        throw std::runtime_error("transaction fee overflows qint64");
    }
    qint64 totalFee = static_cast<qint64>(m_operations.length()) * static_cast<qint64>(m_baseFee);

    Transaction* transaction = new Transaction(
        m_accountConverter,
        m_sourceAccount->getKeypair()->getAccountId(),
        totalFee,
        m_sourceAccount->getIncrementedSequenceNumber(),
        m_operations,
        m_memo,
        m_timeBounds,
        m_network);

    // Bump sequence only after Transaction ctor succeeded.
    m_sourceAccount->incrementSequenceNumber();

    // Resources now owned by the Transaction.
    m_memo = nullptr;
    m_timeBounds = nullptr;
    m_operations.clear();

    return transaction;
}
