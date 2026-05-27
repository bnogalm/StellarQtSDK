#include "simulatetransactionresponse.h"

#include <QJsonArray>

namespace {
quint32 readU32(const QJsonObject& o, const QString& k) {
    QJsonValue v = o.value(k);
    return v.isString() ? v.toString().toUInt()
                        : static_cast<quint32>(v.toInteger(0));
}
}

SimulateTransactionResponse SimulateTransactionResponse::fromJson(const QJsonObject& r)
{
    SimulateTransactionResponse out;
    out.m_latestLedger    = readU32(r, "latestLedger");
    out.m_transactionData = r.value("transactionData").toString();
    out.m_minResourceFee  = r.value("minResourceFee").isString()
        ? r.value("minResourceFee").toString()
        : QString::number(r.value("minResourceFee").toInteger(0));
    out.m_error           = r.value("error").toString();

    for (const QJsonValue& v : r.value("events").toArray()) {
        out.m_events.append(v.toString());
    }
    for (const QJsonValue& v : r.value("results").toArray()) {
        QJsonObject o = v.toObject();
        SimulateHostFunctionResult res;
        res.xdr = o.value("xdr").toString();
        for (const QJsonValue& a : o.value("auth").toArray()) {
            res.auth.append(a.toString());
        }
        out.m_results.append(res);
    }
    for (const QJsonValue& v : r.value("stateChanges").toArray()) {
        QJsonObject o = v.toObject();
        SimulateStateChange sc;
        sc.type   = o.value("type").toString();
        sc.key    = o.value("key").toString();
        sc.before = o.value("before").toString();
        sc.after  = o.value("after").toString();
        out.m_stateChanges.append(sc);
    }
    return out;
}
