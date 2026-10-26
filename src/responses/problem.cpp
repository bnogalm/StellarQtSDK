#include "problem.h"
#include "../exception/sdkexception.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <cmath>
#include <limits>

QSTELLAR_BEGIN_NS


namespace {

// Each member is what it says or the document is refused (tryParse returns
// nullptr): a member of another kind was read as empty, and an operation's
// result code that was not text was dropped, so the codes after it moved to
// the operation before and an op_underfunded was reported for another.
[[noreturn]] void refuse(const char* key, const QJsonValue& value)
{
    throw qstellar::exception::SdkException(
        QStringLiteral("Problem: %1 is not what Horizon sends there (%2)")
            .arg(QLatin1String(key), QString::fromUtf8(QJsonDocument(QJsonArray{value}).toJson(QJsonDocument::Compact))));
}

QString readString(const QJsonObject& obj, const char* key)
{
    auto v = obj.value(QLatin1String(key));
    if (v.isUndefined() || v.isNull())
        return QString();
    if (!v.isString())
        refuse(key, v);
    return v.toString();
}

int readIntOrMinusOne(const QJsonObject& obj, const char* key)
{
    auto v = obj.value(QLatin1String(key));
    if (v.isUndefined() || v.isNull())
        return -1;
    const double d = v.toDouble();
    if (!v.isDouble() || !(d >= std::numeric_limits<int>::min() && d <= std::numeric_limits<int>::max())
            || d != std::floor(d))
        refuse(key, v);
    return static_cast<int>(d);
}

QStringList readStringArray(const QJsonObject& obj, const char* key)
{
    QStringList out;
    auto v = obj.value(QLatin1String(key));
    if (v.isUndefined() || v.isNull())
        return out;
    if (!v.isArray())
        refuse(key, v);
    for (const QJsonValue& item : v.toArray()) {
        if (!item.isString())
            refuse(key, item);
        out.append(item.toString());
    }
    return out;
}

} // anon

void Problem::parseFromJson(const QByteArray& json)
{
    QJsonParseError err{};
    QJsonDocument doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        throw qstellar::exception::SdkException(
            QStringLiteral("Problem: invalid JSON (%1)").arg(err.errorString()));
    }
    const QJsonObject obj = doc.object();
    m_type   = readString(obj, "type");
    m_title  = readString(obj, "title");
    m_status = readIntOrMinusOne(obj, "status");
    m_detail = readString(obj, "detail");

    const auto extrasVal = obj.value(QLatin1String("extras"));
    if (!extrasVal.isUndefined() && !extrasVal.isNull() && !extrasVal.isObject())
        refuse("extras", extrasVal);
    if (extrasVal.isObject()) {
        const QJsonObject extras = extrasVal.toObject();
        Extras e;
        e.setHash(readString(extras, "hash"));
        e.setEnvelopeXdr(readString(extras, "envelope_xdr"));
        e.setResultXdr(readString(extras, "result_xdr"));

        const auto rcVal = extras.value(QLatin1String("result_codes"));
        if (!rcVal.isUndefined() && !rcVal.isNull() && !rcVal.isObject())
            refuse("result_codes", rcVal);
        if (rcVal.isObject()) {
            const QJsonObject rc = rcVal.toObject();
            Extras::ResultCodes codes;
            codes.setTransaction(readString(rc, "transaction"));
            codes.setInnerTransaction(readString(rc, "inner_transaction"));
            codes.setOperations(readStringArray(rc, "operations"));
            e.setResultCodes(codes);
        }
        m_extras = e;
    }
}

Problem* Problem::tryParse(const QByteArray& json)
{
    Problem* p = new Problem();
    try {
        p->parseFromJson(json);
        return p;
    } catch (...) {
        delete p;
        return nullptr;
    }
}
QSTELLAR_END_NS
