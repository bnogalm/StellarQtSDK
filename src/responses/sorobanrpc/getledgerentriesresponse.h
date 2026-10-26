#ifndef GETLEDGERENTRIESRESPONSE_H
#define GETLEDGERENTRIESRESPONSE_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QtGlobal>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/** Soroban RPC `getLedgerEntries` result. */
struct LedgerEntryResult
{
    QString key;                  // base64 XDR LedgerKey
    QString xdr;                  // base64 XDR LedgerEntryData (type + body; no lastModifiedLedgerSeq or ext)
    quint32 lastModifiedLedgerSeq = 0;
    /** The ledger the entry lives until; 0 for an entry that is archived (the
     *  RPC sends 0), and 0 too when the RPC sends none (an entry with no TTL,
     *  such as an account): hasLiveUntilLedgerSeq tells the two apart. */
    quint32 liveUntilLedgerSeq = 0;
    /** Whether the RPC sent liveUntilLedgerSeq. */
    bool hasLiveUntilLedgerSeq = false;
};

class GetLedgerEntriesResponse
{
    QList<LedgerEntryResult> m_entries;
    quint32 m_latestLedger = 0;
public:
    /** Throws std::runtime_error when a field is missing or not what the RPC
     *  sends (a JSON integer for a ledger number, base64 XDR for key and xdr). */
    static GetLedgerEntriesResponse fromJson(const QJsonObject& result);
    const QList<LedgerEntryResult>& getEntries() const { return m_entries; }
    quint32 getLatestLedger() const { return m_latestLedger; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(GetLedgerEntriesResponse)




#endif // GETLEDGERENTRIESRESPONSE_H
