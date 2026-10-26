#include "toid.h"

#include <stdexcept>

QSTELLAR_BEGIN_NS

TOID::TOID(quint32 ledger, quint32 txIndex, quint32 opIndex)
    : m_ledger(ledger), m_txIndex(txIndex), m_opIndex(opIndex)
{
    if (ledger > MAX_LEDGER) {
        throw std::runtime_error("TOID ledger exceeds 2147483647 (it would encode a negative id)");
    }
    if (txIndex > MAX_TX_INDEX) {
        throw std::runtime_error("TOID txIndex exceeds 20-bit range");
    }
    if (opIndex > MAX_OP_INDEX) {
        throw std::runtime_error("TOID opIndex exceeds 12-bit range");
    }
}

TOID TOID::fromInt64(qint64 packed)
{
    const quint64 u = static_cast<quint64>(packed);
    const quint32 ledger  = static_cast<quint32>((u >> 32) & 0xFFFFFFFFULL);
    const quint32 txIndex = static_cast<quint32>((u >> 12) & 0xFFFFFULL);
    const quint32 opIndex = static_cast<quint32>(u & 0xFFFULL);
    // Not through the constructor: decoding takes any 64 bits, a negative id
    // (ledger past 2^31 - 1) included.
    TOID out;
    out.m_ledger = ledger;
    out.m_txIndex = txIndex;
    out.m_opIndex = opIndex;
    return out;
}

qint64 TOID::toInt64() const
{
    const quint64 u = (static_cast<quint64>(m_ledger) << 32)
                    | (static_cast<quint64>(m_txIndex) << 12)
                    | static_cast<quint64>(m_opIndex);
    return static_cast<qint64>(u);
}

TOID TOID::afterLedger(quint32 ledger)
{
    return TOID(ledger, MAX_TX_INDEX, MAX_OP_INDEX);
}

QPair<qint64, qint64> TOID::ledgerRangeInclusive(quint32 from, quint32 to)
{
    if (from > to)
        throw std::runtime_error("TOID ledger range: from is after to");
    if (from == 0 || to == 0)
        throw std::runtime_error("TOID ledger range: ledgers start at 1");
    if (to >= MAX_LEDGER)
        throw std::runtime_error("TOID ledger range: the ledger after `to` has no id");
    const qint64 start = from > 1 ? TOID(from, 0, 0).toInt64() : 0;
    return qMakePair(start, TOID(to + 1, 0, 0).toInt64());
}

void TOID::incOperationOrder()
{
    if (m_opIndex < MAX_OP_INDEX) {
        ++m_opIndex;
        return;
    }
    if (m_ledger >= MAX_LEDGER)
        throw std::runtime_error("TOID: the next operation would be past the last ledger an id can hold");
    m_opIndex = 0;
    ++m_ledger;
}

TOID TOID::fromString(const QString& text)
{
    bool digits = !text.isEmpty() && text.size() <= 19;
    for (const QChar c : text)
        digits = digits && c >= QLatin1Char('0') && c <= QLatin1Char('9');
    bool ok = false;
    const qint64 value = digits ? text.toLongLong(&ok) : 0;
    if (!digits || !ok)
        throw std::runtime_error("a TOID is decimal digits, 0 to 9223372036854775807");
    return fromInt64(value);
}

QSTELLAR_END_NS
