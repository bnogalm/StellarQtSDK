#include "simulatetransactionresponse.h"

#include <QJsonArray>

QSTELLAR_BEGIN_NS


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

    QJsonValue rp = r.value("restorePreamble");
    if (rp.isObject()) {
        QJsonObject o = rp.toObject();
        out.m_restorePreamble.transactionData = o.value("transactionData").toString();
        out.m_restorePreamble.minResourceFee = o.value("minResourceFee").isString()
            ? o.value("minResourceFee").toString()
            : QString::number(o.value("minResourceFee").toInteger(0));
        out.m_restorePreamble.present = true;
    }
    return out;
}
QSTELLAR_END_NS
