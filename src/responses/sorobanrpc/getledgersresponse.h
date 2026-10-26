#ifndef GETLEDGERSRESPONSE_H
#define GETLEDGERSRESPONSE_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QtGlobal>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/** Soroban RPC `getLedgers` result. */
struct LedgerInfo
{
    QString hash;
    quint32 sequence = 0;
    QString ledgerCloseTime;        // Unix seconds, as decimal text
    QString headerXdr;
    QString metadataXdr;
};

class GetLedgersResponse
{
    QList<LedgerInfo> m_ledgers;
    quint32 m_latestLedger = 0;
    QString m_latestLedgerCloseTime;
    quint32 m_oldestLedger = 0;
    QString m_oldestLedgerCloseTime;
    QString m_cursor;
public:
    /** Throws std::runtime_error when a field is missing or not what the RPC
     *  sends: a number that is not a JSON integer in range (a decimal string
     *  for a ledger's close time), a hash that is not 64 hex digits, or XDR
     *  that is not base64. */
    static GetLedgersResponse fromJson(const QJsonObject& result);
    const QList<LedgerInfo>& getLedgers() const { return m_ledgers; }
    quint32 getLatestLedger() const { return m_latestLedger; }
    /** Unix seconds, as decimal text. */
    QString getLatestLedgerCloseTime() const { return m_latestLedgerCloseTime; }
    quint32 getOldestLedger() const { return m_oldestLedger; }
    /** Unix seconds, as decimal text. */
    QString getOldestLedgerCloseTime() const { return m_oldestLedgerCloseTime; }
    QString getCursor() const { return m_cursor; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(GetLedgersResponse)




#endif // GETLEDGERSRESPONSE_H
