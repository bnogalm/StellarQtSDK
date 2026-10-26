#include "predicate.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <initializer_list>
#include <limits>
#include "qtcompat.h"
#include "responses/response.h"

QSTELLAR_BEGIN_NS

namespace {

[[noreturn]] void refuse(const QString& field, const QString& why)
{
    throw std::runtime_error((field.isEmpty() ? why : field + QStringLiteral(": ") + why).toStdString());
}

QString member(const QString& path, const QString& key)
{
    return path.isEmpty() ? key : path + QLatin1Char('.') + key;
}

QString element(const QString& field, int index)
{
    return field + QLatin1Char('[') + QString::number(index) + QLatin1Char(']');
}

/** A JSON value as an error message shows it (as the Horizon readers do). */
QString describe(const QJsonValue& value)
{
    switch (value.type()) {
    case QJsonValue::Null: return QStringLiteral("null");
    case QJsonValue::Bool: return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    case QJsonValue::Double:
        return QStringLiteral("the number ") + QString::number(value.toDouble(), 'g', 17);
    case QJsonValue::String: {
        QString text = value.toString();
        if (text.size() > 40)
            text = text.left(40) + QStringLiteral("...");
        return QStringLiteral("the text \"") + text + QLatin1Char('"');
    }
    case QJsonValue::Array: return QStringLiteral("an array");
    case QJsonValue::Object: return QStringLiteral("an object");
    default: return QStringLiteral("nothing");
    }
}

/** Core's validatePredicate: a claimant's predicate is level 1, and none
 *  deeper than level 4 is valid. */
const int kMaxDepth = 4;

const qint64 kMaxSeconds = std::numeric_limits<qint64>::max();

bool isLeapYear(qint64 year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

/** Days from 1970-01-01 to a proleptic Gregorian date, for year >= 0
 *  (H. Hinnant's days_from_civil). */
qint64 daysFromCivil(qint64 year, int month, int day)
{
    year -= month <= 2 ? 1 : 0;
    const qint64 era = (year >= 0 ? year : year - 399) / 400;
    const qint64 yearOfEra = year - era * 400;
    const qint64 dayOfYear = (153 * (month > 2 ? month - 3 : month + 9) + 2) / 5 + day - 1;
    const qint64 dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
    return era * 146097 + dayOfEra - 719468;
}

/**
 * The time `seconds` (>= 0) as Horizon writes abs_before: RFC 3339 in UTC,
 * the year in at least 4 digits, with a '+' before it past 9999 (stellar/go
 * xdr/json.go, iso8601Time). Qt's own ISO 8601 stops at the year 9999.
 */
QString horizonTime(qint64 seconds)
{
    const qint64 days = seconds / 86400;
    const qint64 secondOfDay = seconds % 86400;
    // H. Hinnant's civil_from_days, for days >= 0.
    const qint64 z = days + 719468;
    const qint64 era = z / 146097;
    const qint64 dayOfEra = z - era * 146097;
    const qint64 yearOfEra = (dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096) / 365;
    const qint64 dayOfYear = dayOfEra - (365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100);
    const qint64 mp = (5 * dayOfYear + 2) / 153;
    const int day = static_cast<int>(dayOfYear - (153 * mp + 2) / 5 + 1);
    const int month = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
    const qint64 year = yearOfEra + era * 400 + (month <= 2 ? 1 : 0);
    const QLatin1Char zero('0');
    const QString text = QStringLiteral("%1-%2-%3T%4:%5:%6Z")
                             .arg(year, 4, 10, zero)
                             .arg(month, 2, 10, zero)
                             .arg(day, 2, 10, zero)
                             .arg(secondOfDay / 3600, 2, 10, zero)
                             .arg(secondOfDay / 60 % 60, 2, 10, zero)
                             .arg(secondOfDay % 60, 2, 10, zero);
    return year > 9999 ? QStringLiteral("+") + text : text;
}

/**
 * The second Horizon's abs_before text names. It used to go through
 * QDateTime::fromString(Qt::ISODate), which reads no year past 9999 (such a
 * time came out invalid, and so as another time), and reads a time with no
 * zone as the computer's local time.
 */
qint64 horizonSeconds(const QJsonValue& value, const QString& field)
{
    const QString expected = QStringLiteral(
        "expected a UTC time as Horizon writes it (2020-09-28T17:57:04Z, +10000-01-01T00:00:00Z "
        "past the year 9999), found ");
    if (!value.isString())
        refuse(field, expected + describe(value));
    const QString text = value.toString();
    int i = text.startsWith(QLatin1Char('+')) ? 1 : 0;
    qint64 year = 0;
    int digits = 0;
    for (; i < text.size() && text.at(i) >= QLatin1Char('0') && text.at(i) <= QLatin1Char('9'); ++i) {
        if (++digits > 12) // past any year 2^63-1 seconds reaches
            refuse(field, describe(value) + QStringLiteral(" is past the last time a claim predicate holds, ")
                              + horizonTime(kMaxSeconds));
        year = year * 10 + (text.at(i).unicode() - '0');
    }
    // After the year, exactly -MM-DDTHH:MM:SSZ.
    const QString rest = text.mid(i);
    static const char shape[] = "-00-00T00:00:00Z";
    bool shaped = digits >= 4 && rest.size() == 16;
    for (int k = 0; shaped && k < 16; ++k) {
        const QChar c = rest.at(k);
        shaped = shape[k] == '0' ? (c >= QLatin1Char('0') && c <= QLatin1Char('9'))
                                 : c == QLatin1Char(shape[k]);
    }
    if (!shaped)
        refuse(field, expected + describe(value));
    auto two = [&rest](int at) {
        return (rest.at(at).unicode() - '0') * 10 + (rest.at(at + 1).unicode() - '0');
    };
    const int month = two(1), day = two(4), hour = two(7), minute = two(10), second = two(13);
    static const int monthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month < 1 || month > 12 || day < 1
            || day > monthDays[month - 1] + (month == 2 && isLeapYear(year) ? 1 : 0)
            || hour > 23 || minute > 59 || second > 59)
        refuse(field, describe(value) + QStringLiteral(" is not a date and time"));
    const qint64 days = daysFromCivil(year, month, day);
    const qint64 secondOfDay = hour * 3600 + minute * 60 + second;
    // Core refuses a negative time (validatePredicate).
    if (days < 0)
        refuse(field, describe(value) + QStringLiteral(" is before 1970, a negative time Core refuses"));
    if (days > (kMaxSeconds - secondOfDay) / 86400)
        refuse(field, describe(value) + QStringLiteral(" is past the last time a claim predicate holds, ")
                          + horizonTime(kMaxSeconds));
    const qint64 seconds = days * 86400 + secondOfDay;
    // One spelling per time: the '+' exactly past the year 9999, and no more
    // digits in the year than it takes.
    if (horizonTime(seconds) != text)
        refuse(field, QStringLiteral("%1 is not how Horizon writes that time: %2")
                          .arg(describe(value), horizonTime(seconds)));
    return seconds;
}

Predicate* readPredicate(const QJsonValue& value, const QString& field, int depth)
{
    if (!value.isObject())
        refuse(field, QStringLiteral("expected a predicate object, found ") + describe(value));
    // Such a predicate was read and signed, and Core refused it.
    if (depth > kMaxDepth)
        refuse(field, QStringLiteral("nested deeper than the %1 levels Core allows").arg(kMaxDepth));
    const QJsonObject predicate = value.toObject();
    const QStringList keys = predicate.keys();
    auto only = [&](std::initializer_list<const char*> allowed) {
        for (const QString& key : keys) {
            bool known = false;
            for (const char* a : allowed)
                known = known || key == QLatin1String(a);
            if (!known)
                refuse(member(field, key), QStringLiteral("not part of this predicate"));
        }
    };
    if (predicate.contains(QStringLiteral("unconditional"))) {
        only({ "unconditional" });
        const QJsonValue flag = predicate.value(QStringLiteral("unconditional"));
        if (!flag.isBool() || !flag.toBool())
            refuse(member(field, QStringLiteral("unconditional")),
                   QStringLiteral("expected true, found ") + describe(flag));
        return new Predicate::Unconditional();
    }
    for (const char* both : { "and", "or" }) {
        const QString key = QLatin1String(both);
        if (!predicate.contains(key))
            continue;
        only({ both });
        const QJsonValue inner = predicate.value(key);
        // Exactly two, as in the XDR: a list of any other length used to
        // make a node that toXdr() then clipped or refused.
        if (!inner.isArray() || inner.toArray().size() != 2)
            refuse(member(field, key), QStringLiteral("expected an array of two predicates, found ")
                                           + (inner.isArray() ? QStringLiteral("%1 of them").arg(inner.toArray().size())
                                                              : describe(inner)));
        QList<Predicate*> parts;
        try {
            for (int i = 0; i < 2; ++i)
                parts.append(readPredicate(inner.toArray().at(i), element(member(field, key), i), depth + 1));
        } catch (...) {
            qDeleteAll(parts);
            throw;
        }
        if (key == QLatin1String("and"))
            return new Predicate::And(parts);
        return new Predicate::Or(parts);
    }
    if (predicate.contains(QStringLiteral("not"))) {
        only({ "not" });
        return new Predicate::Not(readPredicate(predicate.value(QStringLiteral("not")),
                                                member(field, QStringLiteral("not")), depth + 1));
    }
    if (predicate.contains(QStringLiteral("abs_before")) || predicate.contains(QStringLiteral("abs_before_epoch"))) {
        only({ "abs_before", "abs_before_epoch" });
        const QString textField = member(field, QStringLiteral("abs_before"));
        if (!predicate.contains(QStringLiteral("abs_before")))
            refuse(textField, QStringLiteral("missing: Horizon writes the time as abs_before, "
                                             "abs_before_epoch beside it"));
        const qint64 seconds = horizonSeconds(predicate.value(QStringLiteral("abs_before")), textField);
        const QJsonValue epoch = predicate.value(QStringLiteral("abs_before_epoch"));
        if (!epoch.isUndefined()) {
            const qint64 given = ResponseConverters::exactInteger(
                epoch, member(field, QStringLiteral("abs_before_epoch")), 0, kMaxSeconds);
            if (given != seconds)
                refuse(textField, QStringLiteral("%1 is not abs_before_epoch %2 (%3)")
                                      .arg(horizonTime(seconds)).arg(given).arg(horizonTime(given)));
        }
        return new Predicate::AbsBefore(seconds);
    }
    if (predicate.contains(QStringLiteral("rel_before"))) {
        only({ "rel_before" });
        // Core refuses a negative one; text that is no number used to read 0,
        // "claimable only within 0 seconds".
        return new Predicate::RelBefore(ResponseConverters::exactInteger(
            predicate.value(QStringLiteral("rel_before")), member(field, QStringLiteral("rel_before")), 0,
            kMaxSeconds));
    }
    refuse(field, keys.isEmpty() ? QStringLiteral("an empty predicate")
                                 : QStringLiteral("unknown predicate \"%1\"").arg(keys.first()));
}

} // namespace

Predicate *Predicate::fromXdr(stellar::ClaimPredicate &xdr) {
    switch (xdr.type) {
    case stellar::ClaimPredicateType::CLAIM_PREDICATE_UNCONDITIONAL:
        return new Unconditional();
    case stellar::ClaimPredicateType::CLAIM_PREDICATE_AND:
        // This used to read xdr.notPredicate: the three members share a
        // union, so today they are the same bytes, but that is union aliasing
        // (UB) and any future layout divergence breaks it silently.
        if(xdr.andPredicates.value.size()>1)
            return new And(Predicate::convertXDRPredicates(xdr.andPredicates.value));
        else
            throw std::runtime_error("Corrupted predicate");
    case stellar::ClaimPredicateType::CLAIM_PREDICATE_OR:
        if(xdr.orPredicates.value.size()>1)
            return new Or(Predicate::convertXDRPredicates(xdr.orPredicates.value));
        else
            throw std::runtime_error("Corrupted predicate");
    case stellar::ClaimPredicateType::CLAIM_PREDICATE_NOT:
        if(xdr.notPredicate.value.size()>0)
            return new Not(Predicate::fromXdr(xdr.notPredicate.value[0]));
        else
            throw std::runtime_error("Corrupted predicate");
    case stellar::ClaimPredicateType::CLAIM_PREDICATE_BEFORE_RELATIVE_TIME:
        return new RelBefore(xdr.relBefore);
    case stellar::ClaimPredicateType::CLAIM_PREDICATE_BEFORE_ABSOLUTE_TIME:
        return new AbsBefore(xdr.absBefore);
    default:
        throw std::runtime_error("Unknown predicate");
    }
}

Predicate *Predicate::create(Predicate &predicate)
{
    try {
        Predicate::AbsBefore &rm = dynamic_cast<Predicate::AbsBefore&>(predicate);
        return new Predicate::AbsBefore(rm);
    }
    catch (const std::bad_cast&) {
    }
    try {
        Predicate::And &rm = dynamic_cast<Predicate::And&>(predicate);
        return new Predicate::And(rm);
    }
    catch (const std::bad_cast&) {
    }
    try {
        Predicate::Not &rm = dynamic_cast<Predicate::Not&>(predicate);
        return new Predicate::Not(rm);
    }
    catch (const std::bad_cast&) {
    }
    try {
        Predicate::Or &rm = dynamic_cast<Predicate::Or&>(predicate);
        return new Predicate::Or(rm);
    }
    catch (const std::bad_cast&) {
    }
    try {
        Predicate::RelBefore &rm = dynamic_cast<Predicate::RelBefore&>(predicate);
        return new Predicate::RelBefore(rm);
    }
    catch (const std::bad_cast&) {
    }
    try {
        Predicate::Unconditional &rm = dynamic_cast<Predicate::Unconditional&>(predicate);
        return new Predicate::Unconditional(rm);
    }
    catch (const std::bad_cast&) {
    }
    throw std::runtime_error("Unsupported predicate");
}

Predicate *Predicate::create(const QVariant &predicate)
{
    // This read leniently: a time Qt could not read became an invalid one
    // (and so another time), a rel_before that was no number 0 ("claimable
    // within 0 seconds"), an and/or of any length a node toXdr() then
    // clipped or refused, unknown keys were ignored ({"unconditional": false}
    // was unconditional) and nothing bounded the nesting.
    return fromJson(QJsonValue::fromVariant(predicate));
}

Predicate *Predicate::fromJson(const QJsonValue &json, const QString &field)
{
    return readPredicate(json, field, 1);
}

Predicate::~Predicate(){}

QSTELLAR_END_NS

static void registerTypes()
{
    using namespace qstellar;
    regType<Predicate::AbsBefore>();
    regType<Predicate::And>();
    regType<Predicate::Not>();
    regType<Predicate::Or>();
    regType<Predicate::RelBefore>();
    regType<Predicate::Unconditional>();
}
Q_COREAPP_STARTUP_FUNCTION(registerTypes)
