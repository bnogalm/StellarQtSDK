#include "timebounds.h"
#include <exception>

TimeBounds::TimeBounds(qint64 minTime, qint64 maxTime)
{
    if(maxTime > 0 && minTime >= maxTime) {
        throw std::runtime_error("minTime must be >= maxTime");
    }

    m_minTime = minTime;
    m_maxTime = maxTime;
}

qint64 TimeBounds::getMinTime() const{
    return m_minTime;
}

qint64 TimeBounds::getMaxTime() const{
    return m_maxTime;
}

TimeBounds *TimeBounds::fromXdr(stellar::TimeBounds &timeBounds) {
    return new TimeBounds(
                timeBounds.minTime,
                timeBounds.maxTime
                );
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
