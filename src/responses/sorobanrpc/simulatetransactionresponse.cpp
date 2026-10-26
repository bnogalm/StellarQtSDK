#include "simulatetransactionresponse.h"
#include "gethealthresponse.h"

#include <QJsonArray>
#include <limits>

QSTELLAR_BEGIN_NS


SimulateTransactionResponse SimulateTransactionResponse::fromJson(const QJsonObject& r)
{
    const sorobanrpcdetail::RpcFields fields(r, QStringLiteral("simulateTransaction"));
    const quint64 feeMax = std::numeric_limits<qint64>::max();
    SimulateTransactionResponse out;
    // Always sent, as 0 when the RPC could not read the transaction at all.
    out.m_latestLedger    = fields.uint32(QStringLiteral("latestLedger"));
    out.m_transactionData = fields.optionalXdr(QStringLiteral("transactionData"));
    // minResourceFee is a decimal string (an int64), left out when 0. Absent
    // stays empty. A value that is not one used to be kept as text for
    // applyTransactionSimulation to refuse, and a JSON number was taken too;
    // now the answer is refused here.
    out.m_minResourceFee  = fields.optionalDecimal(QStringLiteral("minResourceFee"), feeMax);
    // Present only when the simulation failed, and then with what it said:
    // an empty or non-string error used to read as a simulation that worked.
    const QString errorKey = QStringLiteral("error");
    if (fields.has(errorKey)) {
        out.m_error = fields.string(errorKey);
        if (out.m_error.isEmpty())
            fields.refuse(errorKey, QStringLiteral("is empty"));
    }

    out.m_events = fields.xdrList(QStringLiteral("events"));

    const QString resultsKey = QStringLiteral("results");
    if (fields.has(resultsKey)) {
        const QJsonArray results = fields.array(resultsKey);
        for (int i = 0; i < results.size(); ++i) {
            const sorobanrpcdetail::RpcFields o = fields.element(resultsKey, results, i);
            SimulateHostFunctionResult res;
            // The return value is always there (an SCV_VOID for none).
            res.xdr  = o.xdr(QStringLiteral("xdr"));
            res.auth = o.xdrList(QStringLiteral("auth"));
            out.m_results.append(res);
        }
    }

    const QString changesKey = QStringLiteral("stateChanges");
    if (fields.has(changesKey)) {
        const QJsonArray changes = fields.array(changesKey);
        for (int i = 0; i < changes.size(); ++i) {
            const sorobanrpcdetail::RpcFields o = fields.element(changesKey, changes, i);
            SimulateStateChange sc;
            sc.type = o.oneOf(QStringLiteral("type"), { QStringLiteral("created"),
                                                        QStringLiteral("updated"),
                                                        QStringLiteral("deleted") });
            sc.key  = o.xdr(QStringLiteral("key"));
            // null (or left out) for the side the change has no entry on: a
            // created entry has no before, a deleted one no after. An entry on
            // the wrong side contradicts the type.
            auto side = [&o](const QString& key, bool expected) {
                const QJsonValue v = o.json().value(key);
                const bool present = !v.isUndefined() && !v.isNull();
                if (present != expected) {
                    o.refuse(key, expected ? QStringLiteral("is missing for this type of change")
                                           : QStringLiteral("is not null for this type of change"));
                }
                return present ? o.xdr(key) : QString();
            };
            sc.before = side(QStringLiteral("before"), sc.type != QLatin1String("created"));
            sc.after  = side(QStringLiteral("after"), sc.type != QLatin1String("deleted"));
            out.m_stateChanges.append(sc);
        }
    }

    const QString preambleKey = QStringLiteral("restorePreamble");
    if (fields.has(preambleKey)) {
        const sorobanrpcdetail::RpcFields o = fields.object(preambleKey);
        out.m_restorePreamble.transactionData = o.xdr(QStringLiteral("transactionData"));
        // Always sent, even when 0; a JSON number used to be read too.
        out.m_restorePreamble.minResourceFee = o.decimal(QStringLiteral("minResourceFee"), feeMax);
        out.m_restorePreamble.present = true;
    }
    return out;
}
QSTELLAR_END_NS
