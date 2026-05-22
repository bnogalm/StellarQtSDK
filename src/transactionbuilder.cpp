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
    , m_preconditions(std::move(other.m_preconditions))
    , m_operations(other.m_operations)
    , m_timeoutSet(other.m_timeoutSet)
    , m_baseFee(other.m_baseFee)
{
    // "move-like" copy: transfer ownership of owned resources so the original
    // can be safely destroyed. m_preconditions was already moved above.
    other.m_memo = nullptr;
    other.m_operations.clear();
}

TransactionBuilder::~TransactionBuilder()
{
    // Does not destroy the source account.
    if (m_memo)
        delete m_memo;
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
    if (m_preconditions.getTimeBounds()) {
        throw std::runtime_error("TimeBounds has been already added.");
    }
    checkNotNull(reinterpret_cast<intptr_t>(timeBounds), "timeBounds cannot be null");
    m_preconditions.setTimeBounds(timeBounds);
    m_timeoutSet = true;
    return *this;
}

TransactionBuilder& TransactionBuilder::setTimeout(qint64 timeout)
{
    TimeBounds* current = m_preconditions.getTimeBounds();
    if (current && current->getMaxTime() > 0) {
        throw std::runtime_error("TimeBounds.max_time has been already set - setting timeout would overwrite it.");
    }
    if (timeout < 0) {
        throw std::runtime_error("timeout cannot be negative");
    }
    m_timeoutSet = true;
    if (timeout > 0) {
        qint64 timeoutTimestamp = QDateTime::currentMSecsSinceEpoch() / 1000L + timeout;
        if (!current) {
            m_preconditions.setTimeBounds(new TimeBounds(0, timeoutTimestamp));
        } else {
            qint64 min = current->getMinTime();
            m_preconditions.setTimeBounds(new TimeBounds(min, timeoutTimestamp));
        }
    }
    return *this;
}

TransactionBuilder& TransactionBuilder::addPreconditions(const TransactionPreconditions& preconditions)
{
    m_preconditions = preconditions;
    // Match the legacy invariant: presence of TimeBounds (with or without
    // a real maxTime) flags the timeout as deliberately handled.
    if (m_preconditions.getTimeBounds()) {
        m_timeoutSet = true;
    }
    return *this;
}

TransactionBuilder& TransactionBuilder::setLedgerBounds(LedgerBounds* ledgerBounds)
{
    m_preconditions.setLedgerBounds(ledgerBounds);
    return *this;
}

TransactionBuilder& TransactionBuilder::setMinSeqNumber(qint64 seqNum)
{
    m_preconditions.setMinSeqNumber(new qint64(seqNum));
    return *this;
}

TransactionBuilder& TransactionBuilder::setMinSeqAge(quint64 minSeqAge)
{
    m_preconditions.setMinSeqAge(minSeqAge);
    return *this;
}

TransactionBuilder& TransactionBuilder::setMinSeqLedgerGap(quint32 gap)
{
    m_preconditions.setMinSeqLedgerGap(gap);
    return *this;
}

TransactionBuilder& TransactionBuilder::addExtraSigner(const SignerKey& key)
{
    m_preconditions.addExtraSigner(key);
    return *this;
}

TransactionBuilder& TransactionBuilder::setSorobanData(const stellar::SorobanTransactionData& data)
{
    m_sorobanData = QSharedPointer<stellar::SorobanTransactionData>::create(data);
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
    TimeBounds* tb = m_preconditions.getTimeBounds();
    if ((!tb || (tb && tb->getMaxTime() == 0)) && !m_timeoutSet) {
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
    // CAP-46 — Soroban resourceFee is added on top of the base inclusion fee.
    if (m_sorobanData) {
        const qint64 rfee = m_sorobanData->resourceFee;
        if (rfee < 0) {
            throw std::runtime_error("Soroban resourceFee must be non-negative");
        }
        if (totalFee > std::numeric_limits<qint64>::max() - rfee) {
            throw std::runtime_error("transaction fee overflows qint64 when adding Soroban resource fee");
        }
        totalFee += rfee;
    }

    Transaction* transaction = new Transaction(
        m_accountConverter,
        m_sourceAccount->getKeypair()->getAccountId(),
        totalFee,
        m_sourceAccount->getIncrementedSequenceNumber(),
        m_operations,
        m_memo,
        std::move(m_preconditions),
        m_network);
    // Hand the Soroban data to the new Transaction.
    transaction->m_sorobanData = m_sorobanData;

    // Bump sequence only after Transaction ctor succeeded.
    m_sourceAccount->incrementSequenceNumber();

    // Resources now owned by the Transaction.
    m_memo = nullptr;
    // m_preconditions was already moved-from above; ensure it's reset for reuse.
    m_preconditions = TransactionPreconditions();
    m_operations.clear();
    m_sorobanData.reset();

    return transaction;
}
