#include "gethealthresponse.h"

QSTELLAR_BEGIN_NS


GetHealthResponse GetHealthResponse::fromJson(const QJsonObject& result)
{
    GetHealthResponse r;
    r.m_status = result.value(QStringLiteral("status")).toString();
    // Numeric fields may arrive as integer (small ledger numbers) or as
    // string (when the JSON producer keeps uint64 precision). Accept both.
    auto readUint32 = [&](const QString& key) -> quint32 {
        QJsonValue v = result.value(key);
        if (v.isString()) return v.toString().toUInt();
        return static_cast<quint32>(v.toInteger(0));
    };
    r.m_latestLedger = readUint32(QStringLiteral("latestLedger"));
    r.m_oldestLedger = readUint32(QStringLiteral("oldestLedger"));
    r.m_ledgerRetentionWindow = readUint32(QStringLiteral("ledgerRetentionWindow"));
    return r;
}
QSTELLAR_END_NS
