#ifndef TRANSACTIONPRECONDITIONS_H
#define TRANSACTIONPRECONDITIONS_H

#include <QList>
#include <QtGlobal>

#include "xdr/stellartransaction.h"
#include "signerkey.h"

class TimeBounds;
class LedgerBounds;

/**
 * CAP-21 — bundles all the optional preconditions a transaction can carry.
 *
 * The class owns the pointer fields (timeBounds, ledgerBounds, minSeqNumber)
 * and frees them in the destructor. extraSigners is held by value.
 *
 * Use `hasV2()` to decide whether the transaction needs the V2 envelope.
 * Use `toXdr()` to materialize a stellar::Preconditions union with the
 * minimal variant (NONE / TIME / V2) that fits the populated fields — same
 * logic the Java SDK applies.
 */
class TransactionPreconditions
{
    TimeBounds* m_timeBounds = nullptr;
    LedgerBounds* m_ledgerBounds = nullptr;
    qint64* m_minSeqNumber = nullptr;
    quint64 m_minSeqAge = 0;       // CAP-21 Duration (seconds). 0 = no constraint.
    quint32 m_minSeqLedgerGap = 0; // 0 = no constraint.
    QList<SignerKey> m_extraSigners;

public:
    static const int MAX_EXTRA_SIGNERS = 2;

    TransactionPreconditions() = default;
    TransactionPreconditions(const TransactionPreconditions& other);
    TransactionPreconditions& operator=(const TransactionPreconditions& other);
    /** Move ops — steal owned pointers from `other`, preserving identity. */
    TransactionPreconditions(TransactionPreconditions&& other) noexcept;
    TransactionPreconditions& operator=(TransactionPreconditions&& other) noexcept;
    ~TransactionPreconditions();

    /** Setters take ownership of the pointer (nullptr-safe). */
    void setTimeBounds(TimeBounds* tb);
    void setLedgerBounds(LedgerBounds* lb);
    void setMinSeqNumber(qint64* seq);

    void setMinSeqAge(quint64 age) { m_minSeqAge = age; }
    void setMinSeqLedgerGap(quint32 gap) { m_minSeqLedgerGap = gap; }
    void addExtraSigner(const SignerKey& key);

    TimeBounds* getTimeBounds() const { return m_timeBounds; }
    LedgerBounds* getLedgerBounds() const { return m_ledgerBounds; }
    qint64* getMinSeqNumber() const { return m_minSeqNumber; }
    quint64 getMinSeqAge() const { return m_minSeqAge; }
    quint32 getMinSeqLedgerGap() const { return m_minSeqLedgerGap; }
    const QList<SignerKey>& getExtraSigners() const { return m_extraSigners; }

    /** True when any V2-only field is set (forces PRECOND_V2 emission). */
    bool hasV2() const;

    stellar::Preconditions toXdr() const;
    static TransactionPreconditions fromXdr(const stellar::Preconditions& xdr);
};

#endif // TRANSACTIONPRECONDITIONS_H
