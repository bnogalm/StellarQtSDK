#ifndef HTTPDATE_H
#define HTTPDATE_H
#include <QByteArray>
#include <QtGlobal>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * The HTTP-date of a Date header (RFC 9110 5.6.7, RFC 7231 7.1.1.1), read
 * exactly, for Server::fetchTimebounds(). QDateTime::fromString with
 * Qt::RFC2822Date is not used: what it accepts of "GMT" and of the obsolete
 * forms is not the same on Qt 5.15 and Qt 6.
 */
class HttpDate
{
public:
    /**
     * The seconds since the Unix epoch that `value` names, in one of the
     * three forms HTTP allows:
     *  - IMF-fixdate, what every server sends: "Sun, 06 Nov 1994 08:49:37 GMT";
     *  - RFC 850: "Sunday, 06-Nov-94 08:49:37 GMT", its two-digit year read
     *    as RFC 7231 7.1.1.1 says against the current UTC year: the year
     *    with those last two digits that is at most 50 years ahead and at
     *    most 49 behind (in 2026, "76" is 2076 and "77" is 1977; in 2051,
     *    "00" is 2100). Years are compared, not instants;
     *  - asctime: "Sun Nov  6 08:49:37 1994".
     * Names are case-sensitive, as the grammar writes them. Space around the
     * value is ignored. Returns false, leaving `secondsSinceEpoch` alone, for
     * anything else: another form or zone, a date that does not exist, an
     * hour past 23, a minute past 59, a second past 60 (a leap second reads
     * as the next one), or a weekday that is not the date's.
     */
    static bool parse(const QByteArray& value, qint64* secondsSinceEpoch);

    /**
     * The same, an RFC 850 two-digit year read against `currentYear`
     * instead of the clock's: for tests, which must not depend on the date
     * they run.
     */
    static bool parse(const QByteArray& value, qint64* secondsSinceEpoch, int currentYear);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(HttpDate)

#endif // HTTPDATE_H
