#include "toid.h"

#include <stdexcept>

TOID::TOID(quint32 ledger, quint32 txIndex, quint32 opIndex)
    : m_ledger(ledger), m_txIndex(txIndex), m_opIndex(opIndex)
{
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
    return TOID(ledger, txIndex, opIndex);
}

qint64 TOID::toInt64() const
{
    const quint64 u = (static_cast<quint64>(m_ledger) << 32)
                    | (static_cast<quint64>(m_txIndex) << 12)
                    | static_cast<quint64>(m_opIndex);
    return static_cast<qint64>(u);
}
