#ifndef TOID_H
#define TOID_H

#include <QtGlobal>
#include <QHash>
#include <QPair>
#include <QString>

#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * SEP-35 — TOID (Total Order ID) packed into a uint64:
 *   high 32 bits  = ledger sequence
 *   next 20 bits  = transaction index within the ledger
 *   low 12 bits   = operation index within the transaction
 *
 * Layout: [ ledger:32 | txIndex:20 | opIndex:12 ]
 *
 * Numbering, as Horizon's ids have it: txIndex is a transaction's order of
 * application in its ledger FROM 1, and opIndex an operation's place in its
 * transaction FROM 1. 0 stands for the whole: a transaction's own id has
 * opIndex 0 (operation 21623064391401473 is operation 1 of transaction
 * 21623064391401472), and a ledger's has txIndex and opIndex 0.
 *
 * Horizon writes ids and paging tokens as decimal text: toString() and
 * fromString() read and write that form.
 */
class TOID
{
    quint32 m_ledger;
    quint32 m_txIndex;
    quint32 m_opIndex;

public:
    static const quint32 MAX_TX_INDEX = 0xFFFFF;   // 20 bits
    static const quint32 MAX_OP_INDEX = 0xFFF;     // 12 bits
    /** Ledger sequences are int32 (Horizon, stellar/go): a larger one would
     *  set the sign bit and encode a negative id that sorts first. */
    static const quint32 MAX_LEDGER = 0x7FFFFFFF;

    TOID() : m_ledger(0), m_txIndex(0), m_opIndex(0) {}
    /** Throws std::runtime_error past MAX_LEDGER, MAX_TX_INDEX or MAX_OP_INDEX
     *  (stellar/go panics on a negative ledger). */
    TOID(quint32 ledger, quint32 txIndex, quint32 opIndex);

    /**
     * Decodes the 64-bit packed form. Never fails: any number splits into a
     * ledger, a transaction and an operation (as stellar/go's Parse), so use
     * it on ids from Horizon, not to check that a number is one.
     */
    static TOID fromInt64(qint64 packed);
    /** Encodes the 64-bit packed form; always positive for a TOID built with the constructor. */
    qint64 toInt64() const;

    /**
     * stellar/go's toid.AfterLedger: the id after everything `ledger` holds
     * (its last transaction and operation indexes), for a cursor that starts
     * past that ledger. Throws past MAX_LEDGER.
     */
    static TOID afterLedger(quint32 ledger);

    /**
     * stellar/go's toid.LedgerRangeInclusive: ledgers `from` to `to`, both
     * included, as ids: the first is where `from` starts (0 for ledger 1),
     * the second where `to + 1` starts, so compare it with <. Throws when
     * `from` > `to`, either is 0 (go: <= 0), or `to` is MAX_LEDGER, whose
     * next ledger has no id (go overflows there).
     */
    static QPair<qint64, qint64> ledgerRangeInclusive(quint32 from, quint32 to);

    /**
     * stellar/go's IncOperationOrder: the next operation index; past
     * MAX_OP_INDEX, operation 0 of the NEXT LEDGER, the transaction index
     * kept, exactly as go does it (a cursor that only has to move forward).
     * Throws when that would pass MAX_LEDGER.
     */
    void incOperationOrder();

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
    /** Horizon's order: by ledger, then transaction, then operation. */
    bool operator<(const TOID& other) const { return toUInt64() < other.toUInt64(); }

    /** The 64 bits as unsigned: the same order as the TOID's, whatever the ledger. */
    quint64 toUInt64() const { return static_cast<quint64>(toInt64()); }
    /** The id as Horizon writes it: decimal text. */
    QString toString() const { return QString::number(toInt64()); }
    /**
     * An id as Horizon writes it (decimal digits, 0 to 2^63 - 1, as an id
     * or a paging token). Throws std::runtime_error on anything else: a
     * sign, a space, a number past int64.
     */
    static TOID fromString(const QString& text);
};

// Qt 6 hashes with a size_t seed; Qt 5 with uint.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
inline size_t qHash(const TOID& id, size_t seed = 0)
#else
inline uint qHash(const TOID& id, uint seed = 0)
#endif
{
    // Qt's own qHash(quint64): this one hides it inside the namespace.
    return QT_PREPEND_NAMESPACE(qHash)(id.toUInt64(), seed);
}

QSTELLAR_END_NS

QSTELLAR_ALIAS(TOID)

#endif // TOID_H
