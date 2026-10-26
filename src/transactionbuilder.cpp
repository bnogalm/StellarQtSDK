#include "transactionbuilder.h"
#include "transaction.h"
#include "util.h"
#include <QDateTime>
#include <QDebug>
#include <limits>
#include <stdexcept>

QSTELLAR_BEGIN_NS

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
    , m_sorobanData(other.m_sorobanData)
    , m_timeoutSet(other.m_timeoutSet)
    , m_baseFee(other.m_baseFee)
    , m_baseFeeSet(other.m_baseFeeSet)
{
    // "move-like" copy: transfer ownership of owned resources so the original
    // can be safely destroyed. m_preconditions was already moved above, so the
    // original no longer has the time bounds its timeout stood for: it must be
    // given a new one before it builds, not sign with none.
    other.m_memo = nullptr;
    other.m_operations.clear();
    other.m_sorobanData.reset();
    other.m_preconditions = TransactionPreconditions();
    other.m_timeoutSet = false;
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

void TransactionBuilder::setDefaultOperationFee(qint64 opFee)
{
    // qint64, like setBaseFee: as a quint32, -5 arrived as 4294967291 and
    // became the fee of every transaction built without setBaseFee.
    if (opFee < TransactionBuilder::BASE_FEE) {
        throw std::runtime_error(
            QString("DefaultOperationFee cannot be smaller than the BASE_FEE (\" %1 \"): %2")
                .arg(TransactionBuilder::BASE_FEE).arg(opFee).toStdString());
    }
    if (opFee > static_cast<qint64>(std::numeric_limits<quint32>::max())) {
        throw std::runtime_error(
            QString("DefaultOperationFee cannot exceed 4294967295 stroops: %1")
                .arg(opFee).toStdString());
    }
    s_defaultOperationFee = static_cast<quint32>(opFee);
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
    // != 0, not > 0: a TimePoint is uint64, and a maxTime of 2^63 or more reads
    // as negative in the qint64 that holds it — and was silently overwritten.
    if (current && current->getMaxTime() != 0) {
        throw std::runtime_error("TimeBounds.max_time has been already set - setting timeout would overwrite it.");
    }
    if (timeout < 0) {
        throw std::runtime_error("timeout cannot be negative");
    }
    // TIMEOUT_INFINITE over a minTime: js writes {0, 0}, dropping the
    // caller's minTime, and py refuses any timeout once bounds are set. The
    // minTime was kept here, a third transaction. Refused, as py does.
    if (timeout == 0 && current && current->getMinTime() != 0) {
        throw std::runtime_error("TimeBounds.min_time has been already set - "
                                 "an infinite timeout would overwrite it.");
    }
    const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000L;
    if (now > 0 && timeout > std::numeric_limits<qint64>::max() - now) {
        throw std::runtime_error("timeout is too large");
    }
    m_timeoutSet = true;
    if (timeout > 0) {
        qint64 timeoutTimestamp = now + timeout;
        if (!current) {
            m_preconditions.setTimeBounds(new TimeBounds(0, timeoutTimestamp));
        } else {
            m_preconditions.setTimeBounds(TimeBounds::fromTimePoints(current->getMinTimePoint(),
                                                                     static_cast<quint64>(timeoutTimestamp)));
        }
    } else if (!current) {
        // TIMEOUT_INFINITE is {0, 0}, as js-stellar-sdk and java write it. With
        // no bounds at all the envelope was PRECOND_NONE: the same transaction
        // with a different hash from theirs (a pre-auth-tx signer computed
        // there did not match). An existing {0, 0} is left as it is.
        m_preconditions.setTimeBounds(new TimeBounds(0, 0));
    }
    return *this;
}

TransactionBuilder& TransactionBuilder::addPreconditions(const TransactionPreconditions& preconditions)
{
    // The whole bundle is replaced, time bounds included (as in java). So the
    // timeout counts as handled only if these preconditions carry time bounds:
    // setTimeout(180) then addPreconditions(minSeqAge only) used to keep the
    // "handled" flag without the bounds, and signed a transaction that never
    // expires. Now build() asks for a timeout again.
    m_preconditions = preconditions;
    m_timeoutSet = m_preconditions.getTimeBounds() != nullptr;
    return *this;
}

TransactionBuilder& TransactionBuilder::setLedgerBounds(LedgerBounds* ledgerBounds)
{
    // A maximum of 0 means no maximum. Otherwise the minimum cannot pass it:
    // js refuses it ("min_ledger cannot be greater than max_ledger") and core
    // rejects the transaction as malformed. The builder owns the bounds, so a
    // refused one is freed.
    if (ledgerBounds && ledgerBounds->getMaxLedger() != 0
            && ledgerBounds->getMinLedger() > ledgerBounds->getMaxLedger()) {
        delete ledgerBounds;
        throw std::runtime_error("min_ledger cannot be greater than max_ledger");
    }
    m_preconditions.setLedgerBounds(ledgerBounds);
    return *this;
}

TransactionBuilder& TransactionBuilder::setMinSeqNumber(qint64 seqNum)
{
    m_preconditions.setMinSeqNumber(new qint64(seqNum));
    return *this;
}

TransactionBuilder& TransactionBuilder::setMinSeqAge(qint64 minSeqAge)
{
    if (minSeqAge < 0)
        throw std::runtime_error("min_account_sequence_age cannot be negative");
    m_preconditions.setMinSeqAge(static_cast<quint64>(minSeqAge));
    return *this;
}

TransactionBuilder& TransactionBuilder::setMinSeqLedgerGap(qint64 gap)
{
    if (gap < 0)
        throw std::runtime_error("min_account_sequence_ledger_gap cannot be negative");
    if (gap > static_cast<qint64>(std::numeric_limits<quint32>::max()))
        throw std::runtime_error("min_account_sequence_ledger_gap cannot exceed 4294967295");
    m_preconditions.setMinSeqLedgerGap(static_cast<quint32>(gap));
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

TransactionBuilder& TransactionBuilder::setBaseFee(qint64 baseFee)
{
    if (baseFee < 0) {
        throw std::runtime_error(
            QString("BaseFee cannot be negative: %1").arg(baseFee).toStdString());
    }
    if (baseFee > static_cast<qint64>(std::numeric_limits<quint32>::max())) {
        throw std::runtime_error(
            QString("BaseFee cannot exceed 4294967295 stroops: %1").arg(baseFee).toStdString());
    }
    m_baseFee = static_cast<quint32>(baseFee);
    m_baseFeeSet = true;
    return *this;
}

Transaction* TransactionBuilder::build()
{
    // Checked first, before anything is handed over: the Transaction
    // constructor refused an empty list only after the time bounds had been
    // moved into it, and a retry then signed a transaction with none.
    if (m_operations.isEmpty()) {
        throw std::runtime_error("At least one operation required");
    }

    // No sequence number follows INT64_MAX. Account refuses to go past it; a
    // TransactionBuilderAccount of the caller's own that wraps handed back
    // INT64_MIN, and the transaction was signed with it.
    if (m_sourceAccount->getSequenceNumber() == std::numeric_limits<qint64>::max()) {
        throw std::runtime_error("the source account's sequence number is at its maximum "
                                 "(9223372036854775807): no further transaction can be built for it");
    }

    // Ensure setTimeout was called or maxTime is set.
    TimeBounds* tb = m_preconditions.getTimeBounds();
    if ((!tb || (tb && tb->getMaxTime() == 0)) && !m_timeoutSet) {
        throw std::runtime_error("TimeBounds has to be set or you must call setTimeout(TIMEOUT_INFINITE).");
    }

    if (m_baseFee == 0 && !m_baseFeeSet) {
#ifdef STELLAR_QT_AUTOSET_BASE_FEE
        qDebug() << "[TransactionBuilder] The `baseFee` parameter of `TransactionBuilder` is required. Setting to BASE_FEE="
                 << TransactionBuilder::BASE_FEE
                 << ". Future versions of this library will error if not provided.";
        m_baseFee = TransactionBuilder::BASE_FEE;
#else
        throw std::runtime_error("The `baseFee` parameter of `TransactionBuilder` is required.");
#endif
    }

    // An envelope holds at most MAX_OPS_PER_TX operations. The XDR array drops
    // anything past that without a word, so a 101-payment batch was signed and
    // accepted as 100 payments, and the 101st never happened.
    if (m_operations.length() > stellar::MAX_OPS_PER_TX) {
        throw std::runtime_error(
            QString("a transaction carries at most %1 operations, not %2")
                .arg(stellar::MAX_OPS_PER_TX).arg(m_operations.length()).toStdString());
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
    // The envelope's fee field is a uint32. Transaction::toV1Xdr used to assign
    // the qint64 total into it unchecked, so a total past 4294967295 was
    // signed truncated while getFee() still reported the full amount: the
    // transaction the caller was shown was not the one that went out.
    if (totalFee > static_cast<qint64>(std::numeric_limits<quint32>::max())) {
        throw std::runtime_error(
            QString("transaction fee %1 exceeds the largest fee a transaction can carry "
                    "(4294967295 stroops)").arg(totalFee).toStdString());
    }

    Transaction* transaction = nullptr;
    try {
        transaction = new Transaction(
            m_accountConverter,
            m_sourceAccount->getAccountId(),
            totalFee,
            m_sourceAccount->getIncrementedSequenceNumber(),
            m_operations,
            m_memo,
            std::move(m_preconditions),
            m_network);
    } catch (...) {
        // The preconditions were handed over and are gone with the failed
        // constructor: the time bounds with them. A retry must be given a
        // timeout again rather than sign with none.
        m_preconditions = TransactionPreconditions();
        m_timeoutSet = false;
        throw;
    }
    // Hand the Soroban data to the new Transaction.
    transaction->m_sorobanData = m_sorobanData;

    // Bump sequence only after Transaction ctor succeeded.
    m_sourceAccount->incrementSequenceNumber();

    // Resources now owned by the Transaction.
    m_memo = nullptr;
    // m_preconditions was already moved-from above; ensure it's reset for reuse.
    // The time bounds went with them, so the next build() needs its own
    // timeout: the flag used to survive, and a second build() on the same
    // builder was signed with no time bounds at all.
    m_preconditions = TransactionPreconditions();
    m_timeoutSet = false;
    m_operations.clear();
    m_sorobanData.reset();

    return transaction;
}

QSTELLAR_END_NS
