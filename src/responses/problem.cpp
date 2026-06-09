#include "problem.h"
#include "../exception/sdkexception.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

QSTELLAR_BEGIN_NS


namespace {

QString readString(const QJsonObject& obj, const char* key)
{
    auto v = obj.value(QLatin1String(key));
    return v.isString() ? v.toString() : QString();
}

int readIntOrMinusOne(const QJsonObject& obj, const char* key)
{
    auto v = obj.value(QLatin1String(key));
    if (v.isDouble())
        return v.toInt();
    return -1;
}

QStringList readStringArray(const QJsonObject& obj, const char* key)
{
    QStringList out;
    auto v = obj.value(QLatin1String(key));
    if (v.isArray()) {
        for (const QJsonValue& item : v.toArray()) {
            if (item.isString())
                out.append(item.toString());
        }
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
    if (extrasVal.isObject()) {
        const QJsonObject extras = extrasVal.toObject();
        Extras e;
        e.setHash(readString(extras, "hash"));
        e.setEnvelopeXdr(readString(extras, "envelope_xdr"));
        e.setResultXdr(readString(extras, "result_xdr"));

        const auto rcVal = extras.value(QLatin1String("result_codes"));
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
