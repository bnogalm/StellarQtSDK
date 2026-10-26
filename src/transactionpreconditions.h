#ifndef TRANSACTIONPRECONDITIONS_H
#define TRANSACTIONPRECONDITIONS_H

#include <QList>
#include <QtGlobal>

#include "xdr/stellartransaction.h"
#include "signerkey.h"
#include "timebounds.h"
#include "ledgerbounds.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

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
    bool m_v2 = false;             // written as PRECOND_V2 even when every V2 field is 0

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

    /**
     * Setting either makes the preconditions a PRECOND_V2, 0 included, as js
     * (value !== null) and py (is not None) do. An explicit 0 used to leave a
     * PRECOND_TIME: the same meaning, another envelope and hash than theirs.
     */
    void setMinSeqAge(quint64 age) { m_minSeqAge = age; m_v2 = true; }
    void setMinSeqLedgerGap(quint32 gap) { m_minSeqLedgerGap = gap; m_v2 = true; }
    void addExtraSigner(const SignerKey& key);

    TimeBounds* getTimeBounds() const { return m_timeBounds; }
    LedgerBounds* getLedgerBounds() const { return m_ledgerBounds; }
    qint64* getMinSeqNumber() const { return m_minSeqNumber; }
    quint64 getMinSeqAge() const { return m_minSeqAge; }
    quint32 getMinSeqLedgerGap() const { return m_minSeqLedgerGap; }
    const QList<SignerKey>& getExtraSigners() const { return m_extraSigners; }

    /**
     * True when any V2-only field is set, or when this came from (or was
     * asked to be) a PRECOND_V2: forces PRECOND_V2 emission.
     *
     * A V2 whose extra fields are all zero is a different envelope — and a
     * different hash — from the PRECOND_TIME with the same bounds. It used to
     * be written back as the latter: refused by the exact-reading gate, and
     * signed as a different transaction when it came from a TxRep document.
     */
    bool hasV2() const;

    /** Emit PRECOND_V2 even when no V2-only field is set. */
    void setV2(bool v2) { m_v2 = v2; }

    stellar::Preconditions toXdr() const;
    static TransactionPreconditions fromXdr(const stellar::Preconditions& xdr);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TransactionPreconditions)

#endif // TRANSACTIONPRECONDITIONS_H
