#ifndef TIMEBOUNDS_H
#define TIMEBOUNDS_H
#include "xdr/stellartransaction.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * <p>TimeBounds represents the time interval that a transaction is valid.</p>
 * @see Transaction
 */
class TimeBounds
{
    stellar::TimePoint m_minTime;
    stellar::TimePoint m_maxTime; // 0 here means no maxTime
    struct FromTimePoints {};
    TimeBounds(quint64 minTime, quint64 maxTime, FromTimePoints);
public:
    /**
     * @param minTime 64bit Unix timestamp
     * @param maxTime 64bit Unix timestamp
     *
     * A negative time is refused, as js and py refuse it ("min_time cannot be
     * negative"): -1 was signed as the TimePoint 2^64 - 1, a transaction that
     * is never valid, and a negative maxTime as one that never expires. A
     * TimePoint of 2^63 or more, which a qint64 cannot hold, comes from
     * fromTimePoints().
     */
    TimeBounds(qint64 minTime, qint64 maxTime);

    /**
     * Bounds from the uint64 TimePoints of the wire, any of them: what
     * decoding a transaction (XDR or TxRep) gives. You own the result.
     */
    static TimeBounds* fromTimePoints(quint64 minTime, quint64 maxTime);


    /**
     * The minimum as the qint64 that 1.0 returns. A TimePoint is uint64: a
     * time of 2^63 or more reads negative here. Use getMinTimePoint() to
     * show or compare it.
     */
    qint64 getMinTime() const;

    /** The maximum as a qint64; see getMinTime(). 0 means no maximum. */
    qint64 getMaxTime() const;

    /** The minimum as the uint64 TimePoint core, js and py read. */
    quint64 getMinTimePoint() const;

    /** The maximum as the uint64 TimePoint core, js and py read. 0 means no maximum. */
    quint64 getMaxTimePoint() const;

    static TimeBounds* fromXdr(stellar::TimeBounds& timeBounds);

    stellar::TimeBounds toXdr();

    bool equals(TimeBounds * other) const;
    bool operator==(const TimeBounds &other) const;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(TimeBounds)

#endif // TIMEBOUNDS_H
