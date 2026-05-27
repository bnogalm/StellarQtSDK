#include "geteventsresponse.h"

#include <QJsonArray>

namespace {
quint32 readU32(const QJsonObject& o, const QString& k) {
    QJsonValue v = o.value(k);
    return v.isString() ? v.toString().toUInt()
                        : static_cast<quint32>(v.toInteger(0));
}
}

GetEventsResponse GetEventsResponse::fromJson(const QJsonObject& r)
{
    GetEventsResponse out;
    for (const QJsonValue& v : r.value("events").toArray()) {
        QJsonObject e = v.toObject();
        EventInfo ev;
        ev.type            = e.value("type").toString();
        ev.ledger          = readU32(e, "ledger");
        ev.ledgerClosedAt  = e.value("ledgerClosedAt").toString();
        ev.contractId      = e.value("contractId").toString();
        ev.id              = e.value("id").toString();
        ev.pagingToken     = static_cast<qint32>(e.value("pagingToken").toInteger(0));
        ev.inSuccessfulContractCall =
            e.value("inSuccessfulContractCall").toBool(true);
        ev.txHash          = e.value("txHash").toString();
        for (const QJsonValue& t : e.value("topic").toArray()) {
            ev.topic.append(t.toString());
        }
        ev.value           = e.value("value").toString();
        out.m_events.append(ev);
    }
    out.m_latestLedger = readU32(r, "latestLedger");
    out.m_cursor       = r.value("cursor").toString();
    return out;
}
