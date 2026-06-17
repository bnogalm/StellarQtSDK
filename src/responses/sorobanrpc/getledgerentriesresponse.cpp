#include "getledgerentriesresponse.h"
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

GetLedgerEntriesResponse GetLedgerEntriesResponse::fromJson(const QJsonObject& r)
{
    GetLedgerEntriesResponse out;
    for (const QJsonValue& v : r.value("entries").toArray()) {
        QJsonObject e = v.toObject();
        LedgerEntryResult er;
        er.key                  = e.value("key").toString();
        er.xdr                  = e.value("xdr").toString();
        er.lastModifiedLedgerSeq = readU32(e, "lastModifiedLedgerSeq");
        er.liveUntilLedgerSeq    = readU32(e, "liveUntilLedgerSeq");
        out.m_entries.append(er);
    }
    out.m_latestLedger = readU32(r, "latestLedger");
    return out;
}
QSTELLAR_END_NS
