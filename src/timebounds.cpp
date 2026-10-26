#include "timebounds.h"
#include <exception>

QSTELLAR_BEGIN_NS

namespace {
// TimePoints are uint64 on the wire and compared as such. Equal bounds
// are a valid one-second window that py and js accept; this refused them
// (with the message the wrong way round). maxTime 0 means unbounded.
void checkOrder(quint64 minTime, quint64 maxTime)
{
    if (maxTime != 0 && minTime > maxTime) {
        throw std::runtime_error("minTime must be <= maxTime");
    }
}
}

TimeBounds::TimeBounds(qint64 minTime, qint64 maxTime)
{
    // A caller's time is refused below zero, as js and py refuse it; only
    // decoding (fromTimePoints) takes a TimePoint of 2^63 or more, which
    // reads negative in a qint64.
    if (minTime < 0)
        throw std::runtime_error("min_time cannot be negative");
    if (maxTime < 0)
        throw std::runtime_error("max_time cannot be negative");
    checkOrder(static_cast<quint64>(minTime), static_cast<quint64>(maxTime));
    m_minTime = minTime;
    m_maxTime = maxTime;
}

TimeBounds::TimeBounds(quint64 minTime, quint64 maxTime, FromTimePoints)
{
    checkOrder(minTime, maxTime);
    m_minTime = minTime;
    m_maxTime = maxTime;
}

TimeBounds *TimeBounds::fromTimePoints(quint64 minTime, quint64 maxTime)
{
    return new TimeBounds(minTime, maxTime, FromTimePoints());
}

qint64 TimeBounds::getMinTime() const{
    return m_minTime;
}

qint64 TimeBounds::getMaxTime() const{
    return m_maxTime;
}

quint64 TimeBounds::getMinTimePoint() const{
    return m_minTime;
}

quint64 TimeBounds::getMaxTimePoint() const{
    return m_maxTime;
}

TimeBounds *TimeBounds::fromXdr(stellar::TimeBounds &timeBounds) {
    return fromTimePoints(timeBounds.minTime, timeBounds.maxTime);
}

stellar::TimeBounds TimeBounds::toXdr() {
    stellar::TimeBounds timeBounds;
    timeBounds.minTime = this->m_minTime;
    timeBounds.maxTime = this->m_maxTime;
    return timeBounds;
}

bool TimeBounds::equals(TimeBounds *other) const
{
    return *this == *other;
}

bool TimeBounds::operator==(const TimeBounds &other) const
{
    if (m_minTime != other.m_minTime) return false;
    return m_maxTime == other.m_maxTime;
}

QSTELLAR_END_NS
