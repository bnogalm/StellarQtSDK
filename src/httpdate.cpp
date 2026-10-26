#include "httpdate.h"

#include <QDate>
#include <QDateTime>
#include <cstring>

QSTELLAR_BEGIN_NS

namespace {

const char* const SHORT_DAYS[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
const char* const LONG_DAYS[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
                                  "Saturday", "Sunday" };
const char* const MONTHS[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/** Julian day of 1970-01-01. */
const qint64 EPOCH_JULIAN_DAY = 2440588;

/** The number written by `count` digits from `from`, or -1. */
int digits(const QByteArray& s, int from, int count)
{
    if (from < 0 || from + count > s.size())
        return -1;
    int value = 0;
    for (int i = from; i < from + count; ++i) {
        const char c = s.at(i);
        if (c < '0' || c > '9')
            return -1;
        value = value * 10 + (c - '0');
    }
    return value;
}

/** 1 (January) to 12 for the three letters at `from`, or 0. */
int month(const QByteArray& s, int from)
{
    const QByteArray name = s.mid(from, 3);
    for (int i = 0; i < 12; ++i)
        if (name == MONTHS[i])
            return i + 1;
    return 0;
}

/** 1 (Monday) to 7 for `name` among `names`, as QDate::dayOfWeek counts, or 0. */
int weekday(const QByteArray& name, const char* const* names)
{
    for (int i = 0; i < 7; ++i)
        if (name == names[i])
            return i + 1;
    return 0;
}

/** "HH:MM:SS" at `from` as seconds into the day, or -1. */
int timeOfDay(const QByteArray& s, int from)
{
    if (from + 8 > s.size() || s.at(from + 2) != ':' || s.at(from + 5) != ':')
        return -1;
    const int hour = digits(s, from, 2);
    const int minute = digits(s, from + 3, 2);
    const int second = digits(s, from + 6, 2);
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 60)
        return -1;
    return hour * 3600 + minute * 60 + second;
}

bool finish(int year, int monthNumber, int day, int dayOfWeek, int seconds, qint64* out)
{
    if (year < 1 || monthNumber == 0 || day < 1 || dayOfWeek == 0 || seconds < 0)
        return false;
    const QDate date(year, monthNumber, day);
    if (!date.isValid() || date.dayOfWeek() != dayOfWeek)
        return false;
    *out = (date.toJulianDay() - EPOCH_JULIAN_DAY) * 86400 + seconds;
    return true;
}

} // namespace

bool HttpDate::parse(const QByteArray& value, qint64* secondsSinceEpoch)
{
    return parse(value, secondsSinceEpoch, QDateTime::currentDateTimeUtc().date().year());
}

bool HttpDate::parse(const QByteArray& value, qint64* secondsSinceEpoch, int currentYear)
{
    const QByteArray s = value.trimmed();
    qint64 result = 0;

    // IMF-fixdate: "Sun, 06 Nov 1994 08:49:37 GMT".
    if (s.size() == 29 && s.at(3) == ',' && s.at(4) == ' ' && s.at(7) == ' ' && s.at(11) == ' '
            && s.at(16) == ' ' && s.at(25) == ' ' && s.mid(26) == "GMT") {
        if (!finish(digits(s, 12, 4), month(s, 8), digits(s, 5, 2),
                    weekday(s.left(3), SHORT_DAYS), timeOfDay(s, 17), &result))
            return false;
        *secondsSinceEpoch = result;
        return true;
    }

    // asctime: "Sun Nov  6 08:49:37 1994" (a one-digit day after a space).
    if (s.size() == 24 && s.at(3) == ' ' && s.at(7) == ' ' && s.at(10) == ' ' && s.at(19) == ' ') {
        const int day = s.at(8) == ' ' ? digits(s, 9, 1) : digits(s, 8, 2);
        if (!finish(digits(s, 20, 4), month(s, 4), day,
                    weekday(s.left(3), SHORT_DAYS), timeOfDay(s, 11), &result))
            return false;
        *secondsSinceEpoch = result;
        return true;
    }

    // RFC 850: "Sunday, 06-Nov-94 08:49:37 GMT".
    const int comma = s.indexOf(',');
    if (comma >= 6 && s.size() == comma + 24) {
        const QByteArray rest = s.mid(comma);
        if (rest.at(1) != ' ' || rest.at(4) != '-' || rest.at(8) != '-' || rest.at(11) != ' '
                || rest.at(20) != ' ' || rest.mid(21) != "GMT")
            return false;
        const int twoDigits = digits(rest, 9, 2);
        if (twoDigits < 0)
            return false;
        // RFC 7231 7.1.1.1: a year more than 50 years in the future is the
        // most recent past year with the same last two digits; otherwise the
        // future one is kept, so a year is never more than 49 years behind
        // when the next century's is at most 50 ahead.
        int year = currentYear - currentYear % 100 + twoDigits;
        if (year > currentYear + 50)
            year -= 100;
        else if (year < currentYear - 49)
            year += 100;
        if (!finish(year, month(rest, 5), digits(rest, 2, 2),
                    weekday(s.left(comma), LONG_DAYS), timeOfDay(rest, 12), &result))
            return false;
        *secondsSinceEpoch = result;
        return true;
    }
    return false;
}

QSTELLAR_END_NS
