#include "getledgersresponse.h"
#include "gethealthresponse.h"

#include <QJsonArray>
#include <limits>

QSTELLAR_BEGIN_NS


GetLedgersResponse GetLedgersResponse::fromJson(const QJsonObject& r)
{
    // Each field as the RPC sends it, or the answer is refused. A ledger's
    // close time comes as a decimal string, the latest and oldest ones as JSON
    // integers: those two used to be read as strings, so every real answer
    // gave them empty, and a missing or non-numeric sequence read as 0.
    const sorobanrpcdetail::RpcFields fields(r, QStringLiteral("getLedgers"));
    GetLedgersResponse out;
    const QString ledgersKey = QStringLiteral("ledgers");
    const QJsonArray ledgers = fields.array(ledgersKey);
    for (int i = 0; i < ledgers.size(); ++i) {
        const sorobanrpcdetail::RpcFields e = fields.element(ledgersKey, ledgers, i);
        LedgerInfo li;
        li.hash             = e.hash(QStringLiteral("hash"));
        li.sequence         = e.uint32(QStringLiteral("sequence"));
        li.ledgerCloseTime  = e.decimal(QStringLiteral("ledgerCloseTime"),
                                        std::numeric_limits<qint64>::max());
        li.headerXdr        = e.xdr(QStringLiteral("headerXdr"));
        li.metadataXdr      = e.xdr(QStringLiteral("metadataXdr"));
        out.m_ledgers.append(li);
    }
    out.m_latestLedger          = fields.uint32(QStringLiteral("latestLedger"));
    out.m_latestLedgerCloseTime = fields.int64Text(QStringLiteral("latestLedgerCloseTime"));
    out.m_oldestLedger          = fields.uint32(QStringLiteral("oldestLedger"));
    out.m_oldestLedgerCloseTime = fields.int64Text(QStringLiteral("oldestLedgerCloseTime"));
    out.m_cursor                = fields.string(QStringLiteral("cursor"));
    return out;
}
QSTELLAR_END_NS
