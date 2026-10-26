#include "gethealthresponse.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QMetaType>
#include <QVariant>
#include <limits>
#include <stdexcept>

#include "util.h"

QSTELLAR_BEGIN_NS


GetHealthResponse GetHealthResponse::fromJson(const QJsonObject& result)
{
    // Only JSON integers, as the RPC sends these uint32s: a string used to be
    // read with toUInt(), so text read as 0 and a number past 32 bits as 0,
    // and a missing field read as 0.
    const sorobanrpcdetail::RpcFields fields(result, QStringLiteral("getHealth"));
    GetHealthResponse r;
    // An unhealthy RPC answers with an error: "healthy" is the one status a
    // result carries.
    r.m_status = fields.oneOf(QStringLiteral("status"), { QStringLiteral("healthy") });
    r.m_latestLedger = fields.uint32(QStringLiteral("latestLedger"));
    r.m_oldestLedger = fields.uint32(QStringLiteral("oldestLedger"));
    r.m_ledgerRetentionWindow = fields.uint32(QStringLiteral("ledgerRetentionWindow"));
    return r;
}

namespace sorobanrpcdetail {

namespace {
/**
 * Appends `value` to `out` as compact JSON text, and stops once `out` is
 * longer than `limit`: an error shows no more. Every integer Qt holds exactly
 * (a qlonglong) is written exactly. QJsonDocument writes one through a double
 * on Qt 5.15, so a refusal of 9007199254740993 used to show 9007199254740992
 * there, another number than the one sent.
 */
void appendJson(QString& out, const QJsonValue& value, int limit)
{
    if (out.size() > limit)
        return;
    if (value.isArray()) {
        const QJsonArray items = value.toArray();
        out += QLatin1Char('[');
        for (int i = 0; i < items.size() && out.size() <= limit; ++i) {
            if (i > 0)
                out += QLatin1Char(',');
            appendJson(out, items.at(i), limit);
        }
        out += QLatin1Char(']');
    } else if (value.isObject()) {
        const QJsonObject members = value.toObject();
        out += QLatin1Char('{');
        for (auto it = members.constBegin(); it != members.constEnd() && out.size() <= limit; ++it) {
            if (it != members.constBegin())
                out += QLatin1Char(',');
            appendJson(out, QJsonValue(it.key()), limit);
            out += QLatin1Char(':');
            appendJson(out, it.value(), limit);
        }
        out += QLatin1Char('}');
    } else if (value.isDouble() && value.toVariant().userType() == QMetaType::LongLong) {
        out += QString::number(value.toVariant().toLongLong());
    } else {
        // A string, true, false, null, or a number Qt holds as a double (a
        // fraction, or one past 64 bits, whose digits the JSON parser already
        // rounded). QJsonDocument writes arrays and objects only: wrap, then
        // unwrap.
        const QString text = QString::fromUtf8(QJsonDocument(QJsonArray{ value }).toJson(QJsonDocument::Compact));
        out += text.mid(1, text.size() - 2);
    }
}

/** The value as JSON text, cut short: what an error shows of it. */
QString shown(const QJsonValue& value)
{
    QString text;
    appendJson(text, value, 64);
    if (text.size() > 64)
        text = text.left(61) + QStringLiteral("...");
    return text;
}

[[noreturn]] void refuseValue(const QString& method, const QString& field, const QString& problem,
                              const QJsonValue& value)
{
    throw std::runtime_error(QStringLiteral("Soroban RPC: %1 answered %2, which %3: %4")
                                 .arg(method, field, problem, shown(value)).toStdString());
}
}

RpcFields::RpcFields(const QJsonObject& object, const QString& method, const QString& path)
    : m_object(object), m_method(method), m_path(path)
{
}

void RpcFields::refuse(const QString& key, const QString& problem) const
{
    refuseValue(m_method, name(key), problem, m_object.value(key));
}

QJsonValue RpcFields::value(const QString& key) const
{
    const QJsonValue v = m_object.value(key);
    if (v.isUndefined()) {
        throw std::runtime_error(QStringLiteral("Soroban RPC: %1 answered without %2")
                                     .arg(m_method, name(key)).toStdString());
    }
    if (v.isNull()) {
        throw std::runtime_error(QStringLiteral("Soroban RPC: %1 answered null for %2")
                                     .arg(m_method, name(key)).toStdString());
    }
    return v;
}

qint64 RpcFields::integer(const QString& key, qint64 min, qint64 max) const
{
    const QJsonValue v = value(key);
    // Qt keeps a JSON integer that fits 64 bits exact, as a qlonglong, and
    // anything else (a fraction, or a number past 64 bits) as a double.
    const QVariant exact = v.isDouble() ? v.toVariant() : QVariant();
    if (exact.userType() != QMetaType::LongLong || exact.toLongLong() < min || exact.toLongLong() > max)
        refuse(key, QStringLiteral("is not a whole number from %1 to %2").arg(min).arg(max));
    return exact.toLongLong();
}

quint32 RpcFields::uint32(const QString& key) const
{
    return static_cast<quint32>(integer(key, 0, std::numeric_limits<quint32>::max()));
}

bool RpcFields::optionalUint32(const QString& key, quint32* out) const
{
    if (!has(key))
        return false;
    *out = uint32(key);
    return true;
}

qint32 RpcFields::int32(const QString& key) const
{
    return static_cast<qint32>(integer(key, 0, std::numeric_limits<qint32>::max()));
}

QString RpcFields::int64Text(const QString& key) const
{
    return QString::number(integer(key, 0, std::numeric_limits<qint64>::max()));
}

QString RpcFields::decimal(const QString& key, quint64 max) const
{
    const QJsonValue v = value(key);
    const QString text = v.toString();
    bool digits = v.isString() && !text.isEmpty() && text.size() <= 20;
    for (int i = 0; digits && i < text.size(); ++i)
        digits = text.at(i) >= QLatin1Char('0') && text.at(i) <= QLatin1Char('9');
    // Digits only: toULongLong alone takes a sign, spaces and "0x" as well.
    bool ok = false;
    const quint64 number = digits ? text.toULongLong(&ok) : 0;
    if (!ok || number > max)
        refuse(key, QStringLiteral("is not a decimal string from 0 to %1").arg(max));
    return QString::number(number);
}

QString RpcFields::optionalDecimal(const QString& key, quint64 max) const
{
    return has(key) ? decimal(key, max) : QString();
}

QString RpcFields::string(const QString& key) const
{
    const QJsonValue v = value(key);
    if (!v.isString())
        refuse(key, QStringLiteral("is not a string"));
    return v.toString();
}

QString RpcFields::optionalString(const QString& key) const
{
    return has(key) ? string(key) : QString();
}

QString RpcFields::oneOf(const QString& key, const QStringList& values) const
{
    const QJsonValue v = value(key);
    if (!v.isString() || !values.contains(v.toString()))
        refuse(key, QStringLiteral("is not one of %1").arg(values.join(QStringLiteral(", "))));
    return v.toString();
}

bool RpcFields::boolean(const QString& key) const
{
    const QJsonValue v = value(key);
    if (!v.isBool())
        refuse(key, QStringLiteral("is not true or false"));
    return v.toBool();
}

bool RpcFields::optionalBoolean(const QString& key, bool* out) const
{
    if (!has(key))
        return false;
    *out = boolean(key);
    return true;
}

QString RpcFields::hash(const QString& key) const
{
    const QJsonValue v = value(key);
    if (!v.isString() || !isHexHash(v.toString()))
        refuse(key, QStringLiteral("is not a hash of 64 hex digits"));
    return v.toString();
}

QString RpcFields::xdr(const QString& key) const
{
    const QJsonValue v = value(key);
    if (!v.isString() || !isBase64Xdr(v.toString()))
        refuse(key, QStringLiteral("is not base64 XDR"));
    return v.toString();
}

QString RpcFields::optionalXdr(const QString& key) const
{
    return has(key) ? xdr(key) : QString();
}

QStringList RpcFields::xdrList(const QString& key) const
{
    return has(key) ? xdrListValue(name(key), value(key)) : QStringList();
}

QStringList RpcFields::xdrListValue(const QString& listName, const QJsonValue& list) const
{
    if (!list.isArray())
        refuseValue(m_method, listName, QStringLiteral("is not an array of base64 XDR"), list);
    QStringList out;
    const QJsonArray items = list.toArray();
    for (int i = 0; i < items.size(); ++i) {
        const QJsonValue item = items.at(i);
        if (!item.isString() || !isBase64Xdr(item.toString())) {
            refuseValue(m_method, QStringLiteral("%1[%2]").arg(listName).arg(i),
                        QStringLiteral("is not base64 XDR"), item);
        }
        out.append(item.toString());
    }
    return out;
}

QJsonArray RpcFields::array(const QString& key) const
{
    const QJsonValue v = value(key);
    if (!v.isArray())
        refuse(key, QStringLiteral("is not an array"));
    return v.toArray();
}

RpcFields RpcFields::object(const QString& key) const
{
    const QJsonValue v = value(key);
    if (!v.isObject())
        refuse(key, QStringLiteral("is not an object"));
    return RpcFields(v.toObject(), m_method, name(key) + QLatin1Char('.'));
}

RpcFields RpcFields::element(const QString& key, const QJsonArray& array, int index) const
{
    const QString at = QStringLiteral("%1[%2]").arg(name(key)).arg(index);
    const QJsonValue v = array.at(index);
    if (!v.isObject())
        refuseValue(m_method, at, QStringLiteral("is not an object"), v);
    return RpcFields(v.toObject(), m_method, at + QLatin1Char('.'));
}

bool isBase64Xdr(const QString& text)
{
    // Strict: a character that is not base64 is an error, where
    // QByteArray::fromBase64 skips it and decodes the rest. Canonical too, as
    // the RPC (Go's padded base64) writes it: the decoder also takes base64
    // without its padding ("AAAAAA") and with the unused bits of the last
    // character set ("AAAAAB==", the same bytes as "AAAAAA=="), which were
    // accepted as other spellings of the same XDR. The decoded bytes must
    // encode back to the very text.
    if (text.isEmpty() || text.size() % 4 != 0)
        return false;
    const QByteArray latin1 = text.toLatin1();
    const auto decoded = QByteArray::fromBase64Encoding(
        latin1, XDR_BASE64ENCODING | QByteArray::AbortOnBase64DecodingErrors);
    return decoded && !(*decoded).isEmpty() && (*decoded).size() % 4 == 0
           && (*decoded).toBase64(XDR_BASE64ENCODING) == latin1;
}

bool isHexHash(const QString& text)
{
    if (text.size() != 64)
        return false;
    for (const QChar c : text) {
        const bool hex = (c >= QLatin1Char('0') && c <= QLatin1Char('9'))
                         || (c >= QLatin1Char('a') && c <= QLatin1Char('f'))
                         || (c >= QLatin1Char('A') && c <= QLatin1Char('F'));
        if (!hex)
            return false;
    }
    return true;
}

} // namespace sorobanrpcdetail

QSTELLAR_END_NS
