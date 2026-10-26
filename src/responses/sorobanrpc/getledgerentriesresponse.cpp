#include "getledgerentriesresponse.h"
#include "gethealthresponse.h"

#include <QJsonArray>

QSTELLAR_BEGIN_NS


GetLedgerEntriesResponse GetLedgerEntriesResponse::fromJson(const QJsonObject& r)
{
    // Each field as the RPC sends it, or the answer is refused: a missing or
    // non-numeric ledger number used to read as 0, and `entries` that was not
    // a list read as no entries at all, which getSACBalance takes for a
    // balance that does not exist.
    const sorobanrpcdetail::RpcFields fields(r, QStringLiteral("getLedgerEntries"));
    GetLedgerEntriesResponse out;
    const QString entriesKey = QStringLiteral("entries");
    const QJsonArray entries = fields.array(entriesKey);
    for (int i = 0; i < entries.size(); ++i) {
        const sorobanrpcdetail::RpcFields e = fields.element(entriesKey, entries, i);
        LedgerEntryResult er;
        er.key                   = e.xdr(QStringLiteral("key"));
        er.xdr                   = e.xdr(QStringLiteral("xdr"));
        er.lastModifiedLedgerSeq = e.uint32(QStringLiteral("lastModifiedLedgerSeq"));
        // Only entries with a TTL have one; an archived entry's is 0.
        er.hasLiveUntilLedgerSeq = e.optionalUint32(QStringLiteral("liveUntilLedgerSeq"),
                                                    &er.liveUntilLedgerSeq);
        out.m_entries.append(er);
    }
    out.m_latestLedger = fields.uint32(QStringLiteral("latestLedger"));
    return out;
}
QSTELLAR_END_NS
