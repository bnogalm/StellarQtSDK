#ifndef GETLEDGERENTRIESRESPONSE_H
#define GETLEDGERENTRIESRESPONSE_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QtGlobal>

/** Soroban RPC `getLedgerEntries` result. */
struct LedgerEntryResult
{
    QString key;                  // base64 XDR LedgerKey
    QString xdr;                  // base64 XDR LedgerEntry
    quint32 lastModifiedLedgerSeq = 0;
    quint32 liveUntilLedgerSeq = 0; // optional; 0 means "absent"
};

class GetLedgerEntriesResponse
{
    QList<LedgerEntryResult> m_entries;
    quint32 m_latestLedger = 0;
public:
    static GetLedgerEntriesResponse fromJson(const QJsonObject& result);
    const QList<LedgerEntryResult>& getEntries() const { return m_entries; }
    quint32 getLatestLedger() const { return m_latestLedger; }
};

#endif // GETLEDGERENTRIESRESPONSE_H
