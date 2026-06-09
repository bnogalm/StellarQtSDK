#include "transactionpreconditions.h"

#include <stdexcept>

#include "timebounds.h"
#include "ledgerbounds.h"

QSTELLAR_BEGIN_NS

TransactionPreconditions::TransactionPreconditions(const TransactionPreconditions& other)
{
    *this = other;
}

TransactionPreconditions::TransactionPreconditions(TransactionPreconditions&& other) noexcept
    : m_timeBounds(other.m_timeBounds)
    , m_ledgerBounds(other.m_ledgerBounds)
    , m_minSeqNumber(other.m_minSeqNumber)
    , m_minSeqAge(other.m_minSeqAge)
    , m_minSeqLedgerGap(other.m_minSeqLedgerGap)
    , m_extraSigners(std::move(other.m_extraSigners))
{
    other.m_timeBounds = nullptr;
    other.m_ledgerBounds = nullptr;
    other.m_minSeqNumber = nullptr;
    other.m_minSeqAge = 0;
    other.m_minSeqLedgerGap = 0;
}

TransactionPreconditions& TransactionPreconditions::operator=(TransactionPreconditions&& other) noexcept
{
    if (this == &other) return *this;
    delete m_timeBounds;
    delete m_ledgerBounds;
    delete m_minSeqNumber;
    m_timeBounds = other.m_timeBounds;
    m_ledgerBounds = other.m_ledgerBounds;
    m_minSeqNumber = other.m_minSeqNumber;
    m_minSeqAge = other.m_minSeqAge;
    m_minSeqLedgerGap = other.m_minSeqLedgerGap;
    m_extraSigners = std::move(other.m_extraSigners);
    other.m_timeBounds = nullptr;
    other.m_ledgerBounds = nullptr;
    other.m_minSeqNumber = nullptr;
    other.m_minSeqAge = 0;
    other.m_minSeqLedgerGap = 0;
    return *this;
}

TransactionPreconditions& TransactionPreconditions::operator=(const TransactionPreconditions& other)
{
    if (this == &other) return *this;
    delete m_timeBounds; m_timeBounds = nullptr;
    delete m_ledgerBounds; m_ledgerBounds = nullptr;
    delete m_minSeqNumber; m_minSeqNumber = nullptr;
    if (other.m_timeBounds) {
        m_timeBounds = new TimeBounds(other.m_timeBounds->getMinTime(),
                                      other.m_timeBounds->getMaxTime());
    }
    if (other.m_ledgerBounds) {
        m_ledgerBounds = new LedgerBounds(*other.m_ledgerBounds);
    }
    if (other.m_minSeqNumber) {
        m_minSeqNumber = new qint64(*other.m_minSeqNumber);
    }
    m_minSeqAge = other.m_minSeqAge;
    m_minSeqLedgerGap = other.m_minSeqLedgerGap;
    m_extraSigners = other.m_extraSigners;
    return *this;
}

TransactionPreconditions::~TransactionPreconditions()
{
    delete m_timeBounds;
    delete m_ledgerBounds;
    delete m_minSeqNumber;
}

void TransactionPreconditions::setTimeBounds(TimeBounds* tb)
{
    if (m_timeBounds != tb) {
        delete m_timeBounds;
        m_timeBounds = tb;
    }
}

void TransactionPreconditions::setLedgerBounds(LedgerBounds* lb)
{
    if (m_ledgerBounds != lb) {
        delete m_ledgerBounds;
        m_ledgerBounds = lb;
    }
}

void TransactionPreconditions::setMinSeqNumber(qint64* seq)
{
    if (m_minSeqNumber != seq) {
        delete m_minSeqNumber;
        m_minSeqNumber = seq;
    }
}

void TransactionPreconditions::addExtraSigner(const SignerKey& key)
{
    if (m_extraSigners.size() >= MAX_EXTRA_SIGNERS) {
        throw std::runtime_error("at most 2 extra signers are allowed (CAP-21)");
    }
    m_extraSigners.append(key);
}

bool TransactionPreconditions::hasV2() const
{
    return m_ledgerBounds != nullptr
        || m_minSeqNumber != nullptr
        || m_minSeqAge != 0
        || m_minSeqLedgerGap != 0
        || !m_extraSigners.isEmpty();
}

stellar::Preconditions TransactionPreconditions::toXdr() const
{
    stellar::Preconditions out;

    if (hasV2()) {
        stellar::PreconditionsV2& v2 = out.fillV2();
        if (m_timeBounds) {
            stellar::TimeBounds& tb = v2.timeBounds.filler();
            tb.minTime = static_cast<quint64>(m_timeBounds->getMinTime());
            tb.maxTime = static_cast<quint64>(m_timeBounds->getMaxTime());
        }
        if (m_ledgerBounds) {
            stellar::LedgerBounds& lb = v2.ledgerBounds.filler();
            lb.minLedger = m_ledgerBounds->getMinLedger();
            lb.maxLedger = m_ledgerBounds->getMaxLedger();
        }
        if (m_minSeqNumber) {
            v2.minSeqNum.filler() = *m_minSeqNumber;
        }
        v2.minSeqAge = m_minSeqAge;
        v2.minSeqLedgerGap = m_minSeqLedgerGap;
        for (const SignerKey& sk : m_extraSigners) {
            v2.extraSigners.append(sk.toXdr());
        }
    } else if (m_timeBounds) {
        stellar::TimeBounds& tb = out.fillTimeBounds();
        tb.minTime = static_cast<quint64>(m_timeBounds->getMinTime());
        tb.maxTime = static_cast<quint64>(m_timeBounds->getMaxTime());
    }
    // else: PRECOND_NONE (default-constructed).
    return out;
}

TransactionPreconditions TransactionPreconditions::fromXdr(const stellar::Preconditions& xdr)
{
    TransactionPreconditions out;
    switch (xdr.type) {
    case stellar::PreconditionType::PRECOND_NONE:
        break;
    case stellar::PreconditionType::PRECOND_TIME:
        out.setTimeBounds(new TimeBounds(
            static_cast<qint64>(xdr.timeBounds.minTime),
            static_cast<qint64>(xdr.timeBounds.maxTime)));
        break;
    case stellar::PreconditionType::PRECOND_V2: {
        const stellar::PreconditionsV2& v2 = xdr.v2;
        if (v2.timeBounds.filled) {
            out.setTimeBounds(new TimeBounds(
                static_cast<qint64>(v2.timeBounds.value.minTime),
                static_cast<qint64>(v2.timeBounds.value.maxTime)));
        }
        if (v2.ledgerBounds.filled) {
            out.setLedgerBounds(new LedgerBounds(
                v2.ledgerBounds.value.minLedger,
                v2.ledgerBounds.value.maxLedger));
        }
        if (v2.minSeqNum.filled) {
            out.setMinSeqNumber(new qint64(v2.minSeqNum.value));
        }
        out.setMinSeqAge(v2.minSeqAge);
        out.setMinSeqLedgerGap(v2.minSeqLedgerGap);
        for (const stellar::SignerKey& sk : v2.extraSigners.value) {
            out.addExtraSigner(SignerKey::fromXdr(sk));
        }
        break;
    }
    }
    return out;
}

QSTELLAR_END_NS
