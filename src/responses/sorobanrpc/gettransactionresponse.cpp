#include "gettransactionresponse.h"
#include "qtcompat.h"

#include <QJsonArray>

QSTELLAR_BEGIN_NS


namespace {
quint32 readU32(const QJsonObject& o, const QString& k) {
    QJsonValue v = o.value(k);
    return v.isString() ? v.toString().toUInt()
                        : static_cast<quint32>(jsonToInt64(v));
}
}

GetTransactionResponse GetTransactionResponse::fromJson(const QJsonObject& r)
{
    GetTransactionResponse out;
    const QString status = r.value("status").toString();
    if (status == "SUCCESS")        out.m_status = Status::SUCCESS;
    else if (status == "FAILED")    out.m_status = Status::FAILED;
    else                            out.m_status = Status::NOT_FOUND;

    out.m_txHash                 = r.value("txHash").toString();
    out.m_latestLedger           = readU32(r, "latestLedger");
    out.m_latestLedgerCloseTime  = r.value("latestLedgerCloseTime").toString();
    out.m_oldestLedger           = readU32(r, "oldestLedger");
    out.m_oldestLedgerCloseTime  = r.value("oldestLedgerCloseTime").toString();
    out.m_applicationOrder       = static_cast<qint32>(jsonToInt64(r.value("applicationOrder")));
    out.m_envelopeXdr            = r.value("envelopeXdr").toString();
    out.m_resultXdr              = r.value("resultXdr").toString();
    out.m_resultMetaXdr          = r.value("resultMetaXdr").toString();
    out.m_ledger                 = readU32(r, "ledger");
    out.m_createdAt              = r.value("createdAt").toString();
    out.m_feeBump                = r.value("feeBump").toBool(false);

    for (const QJsonValue& v : r.value("diagnosticEventsXdr").toArray())
        out.m_diagnosticEventsXdr.append(v.toString());

    // Protocol 23+ nests the event streams under an `events` object.
    const QJsonObject events = r.value("events").toObject();
    for (const QJsonValue& v : events.value("diagnosticEventsXdr").toArray())
        out.m_events.diagnosticEventsXdr.append(v.toString());
    for (const QJsonValue& v : events.value("transactionEventsXdr").toArray())
        out.m_events.transactionEventsXdr.append(v.toString());
    for (const QJsonValue& group : events.value("contractEventsXdr").toArray()) {
        QStringList inner;
        for (const QJsonValue& v : group.toArray())
            inner.append(v.toString());
        out.m_events.contractEventsXdr.append(inner);
    }

    // Expose diagnostics uniformly regardless of which shape the server used:
    // if the flat field was empty, fall back to the nested one.
    if (out.m_diagnosticEventsXdr.isEmpty())
        out.m_diagnosticEventsXdr = out.m_events.diagnosticEventsXdr;

    return out;
}
QSTELLAR_END_NS
