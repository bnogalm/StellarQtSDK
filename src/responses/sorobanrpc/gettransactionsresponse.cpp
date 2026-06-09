#include "gettransactionsresponse.h"

#include <QJsonArray>

QSTELLAR_BEGIN_NS


namespace {
quint32 readU32(const QJsonObject& o, const QString& k) {
    QJsonValue v = o.value(k);
    return v.isString() ? v.toString().toUInt()
                        : static_cast<quint32>(v.toInteger(0));
}
}

GetTransactionsResponse GetTransactionsResponse::fromJson(const QJsonObject& r)
{
    GetTransactionsResponse out;
    for (const QJsonValue& v : r.value("transactions").toArray()) {
        out.m_transactions.append(GetTransactionResponse::fromJson(v.toObject()));
    }
    out.m_latestLedger          = readU32(r, "latestLedger");
    out.m_latestLedgerCloseTime = r.value("latestLedgerCloseTime").toString();
    out.m_oldestLedger          = readU32(r, "oldestLedger");
    out.m_oldestLedgerCloseTime = r.value("oldestLedgerCloseTime").toString();
    out.m_cursor                = r.value("cursor").toString();
    return out;
}
QSTELLAR_END_NS
