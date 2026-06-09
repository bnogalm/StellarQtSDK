#ifndef TOID_H
#define TOID_H

#include <QtGlobal>

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * SEP-35 — TOID (Total Order ID) packed into a uint64:
 *   high 32 bits  = ledger sequence
 *   next 20 bits  = transaction index within the ledger
 *   low 12 bits   = operation index within the transaction
 *
 * Layout: [ ledger:32 | txIndex:20 | opIndex:12 ]
 */
class TOID
{
    quint32 m_ledger;
    quint32 m_txIndex;
    quint32 m_opIndex;

public:
    static const quint32 MAX_TX_INDEX = 0xFFFFF;   // 20 bits
    static const quint32 MAX_OP_INDEX = 0xFFF;     // 12 bits

    TOID() : m_ledger(0), m_txIndex(0), m_opIndex(0) {}
    TOID(quint32 ledger, quint32 txIndex, quint32 opIndex);

    /** Decodes the 64-bit packed form. */
    static TOID fromInt64(qint64 packed);
    /** Encodes the 64-bit packed form. The result is positive (top bit = 0) when ledger < 2^31. */
    qint64 toInt64() const;

    quint32 getLedger() const { return m_ledger; }
    quint32 getTxIndex() const { return m_txIndex; }
    quint32 getOpIndex() const { return m_opIndex; }

    bool operator==(const TOID& other) const
    {
        return m_ledger == other.m_ledger
            && m_txIndex == other.m_txIndex
            && m_opIndex == other.m_opIndex;
    }
    bool operator!=(const TOID& other) const { return !(*this == other); }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TOID)

#endif // TOID_H
