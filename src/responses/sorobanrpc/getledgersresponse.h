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
    QString ledgerCloseTime;
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
    static GetLedgersResponse fromJson(const QJsonObject& result);
    const QList<LedgerInfo>& getLedgers() const { return m_ledgers; }
    quint32 getLatestLedger() const { return m_latestLedger; }
    QString getLatestLedgerCloseTime() const { return m_latestLedgerCloseTime; }
    quint32 getOldestLedger() const { return m_oldestLedger; }
    QString getOldestLedgerCloseTime() const { return m_oldestLedgerCloseTime; }
    QString getCursor() const { return m_cursor; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(GetLedgersResponse)




#endif // GETLEDGERSRESPONSE_H
