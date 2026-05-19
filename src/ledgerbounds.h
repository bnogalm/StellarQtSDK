#ifndef LEDGERBOUNDS_H
#define LEDGERBOUNDS_H

#include <QtGlobal>

#include "xdr/stellartransaction.h"

/**
 * CAP-21 — restricts the ledger range a transaction is valid for.
 * maxLedger == 0 means no upper bound.
 */
class LedgerBounds
{
    quint32 m_minLedger;
    quint32 m_maxLedger;
public:
    LedgerBounds() : m_minLedger(0), m_maxLedger(0) {}
    LedgerBounds(quint32 minLedger, quint32 maxLedger)
        : m_minLedger(minLedger), m_maxLedger(maxLedger) {}
    explicit LedgerBounds(const stellar::LedgerBounds& xdr)
        : m_minLedger(xdr.minLedger), m_maxLedger(xdr.maxLedger) {}

    quint32 getMinLedger() const { return m_minLedger; }
    quint32 getMaxLedger() const { return m_maxLedger; }

    stellar::LedgerBounds toXdr() const
    {
        stellar::LedgerBounds out;
        out.minLedger = m_minLedger;
        out.maxLedger = m_maxLedger;
        return out;
    }

    bool operator==(const LedgerBounds& other) const
    {
        return m_minLedger == other.m_minLedger && m_maxLedger == other.m_maxLedger;
    }
    bool operator!=(const LedgerBounds& other) const { return !(*this == other); }
};

#endif // LEDGERBOUNDS_H
