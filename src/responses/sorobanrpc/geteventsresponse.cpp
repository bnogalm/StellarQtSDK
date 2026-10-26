#include "geteventsresponse.h"
#include "gethealthresponse.h"
#include "stellaraddress.h"

#include <QJsonArray>
#include <limits>

QSTELLAR_BEGIN_NS


namespace {
/** "" (an event no contract emitted) or a contract's C... strkey, as the RPC
 *  writes it. */
QString readContractId(const sorobanrpcdetail::RpcFields& e)
{
    const QString key = QStringLiteral("contractId");
    const QString id = e.string(key);
    if (id.isEmpty())
        return id;
    bool contract = false;
    try {
        const StellarAddress address = StellarAddress::fromString(id);
        contract = address.getType() == StellarAddress::Type::CONTRACT && address.toString() == id;
    } catch (const std::exception&) {
        contract = false;
    }
    if (!contract)
        e.refuse(key, QStringLiteral("is not a contract's strkey (C...)"));
    return id;
}
}

GetEventsResponse GetEventsResponse::fromJson(const QJsonObject& r)
{
    // Each field as the RPC sends it, or the answer is refused. A missing or
    // non-numeric ledger number used to read as 0, and an unknown event type
    // was kept as it came.
    const sorobanrpcdetail::RpcFields fields(r, QStringLiteral("getEvents"));
    const qint64 closeTimeMax = std::numeric_limits<qint64>::max();
    GetEventsResponse out;
    const QString eventsKey = QStringLiteral("events");
    const QJsonArray events = fields.array(eventsKey);
    for (int i = 0; i < events.size(); ++i) {
        const sorobanrpcdetail::RpcFields e = fields.element(eventsKey, events, i);
        EventInfo ev;
        ev.type            = e.oneOf(QStringLiteral("type"),
                                     { QStringLiteral("contract"), QStringLiteral("system"),
                                       QStringLiteral("diagnostic") });
        // An int32 in the RPC: it refuses a ledger past that.
        ev.ledger          = static_cast<quint32>(e.int32(QStringLiteral("ledger")));
        ev.ledgerClosedAt  = e.string(QStringLiteral("ledgerClosedAt"));
        ev.contractId      = readContractId(e);
        ev.id              = e.string(QStringLiteral("id"));
        if (ev.id.isEmpty())
            e.refuse(QStringLiteral("id"), QStringLiteral("is empty"));
        ev.operationIndex  = e.int32(QStringLiteral("operationIndex"));
        ev.transactionIndex= e.int32(QStringLiteral("transactionIndex"));
        // pagingToken is not read: the RPC sent it as a string (the event's
        // id), which this int never held; it is no longer sent. Use id.
        // inSuccessfulContractCall went the same way: deprecated in RPC 23
        // ("remove in v24"), and already dropped from stellar-rpc's protocol
        // package. It was required, so the first RPC without it would have
        // had every page with an event refused; now its absence is recorded,
        // while a wrong type or null is refused. A missing one stays true (the
        // EventInfo default), as 1.0.3 read it: the RPC sends false only on
        // the fee events of a failed transaction, so false was wrong for every
        // other event, and code filtering on it would have dropped them all.
        ev.hasInSuccessfulContractCall = e.optionalBoolean(QStringLiteral("inSuccessfulContractCall"),
                                                           &ev.inSuccessfulContractCall);
        ev.txHash          = e.hash(QStringLiteral("txHash"));
        // Left out when the event has no topics.
        ev.topic           = e.xdrList(QStringLiteral("topic"));
        ev.value           = e.xdr(QStringLiteral("value"));
        out.m_events.append(ev);
    }
    out.m_latestLedger = fields.uint32(QStringLiteral("latestLedger"));
    out.m_oldestLedger = fields.uint32(QStringLiteral("oldestLedger"));
    out.m_latestLedgerCloseTime = fields.decimal(QStringLiteral("latestLedgerCloseTime"), closeTimeMax);
    out.m_oldestLedgerCloseTime = fields.decimal(QStringLiteral("oldestLedgerCloseTime"), closeTimeMax);
    out.m_cursor       = fields.string(QStringLiteral("cursor"));
    return out;
}
QSTELLAR_END_NS
