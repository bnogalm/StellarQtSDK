#ifndef GETHEALTHRESPONSE_H
#define GETHEALTHRESPONSE_H

#include <QJsonObject>
#include <QString>
#include <QtGlobal>

/**
 * Result of `getHealth` (Soroban RPC).
 *
 *   { "status": "healthy",
 *     "latestLedger": ...,
 *     "oldestLedger": ...,
 *     "ledgerRetentionWindow": ... }
 */
class GetHealthResponse
{
    QString m_status;
    quint32 m_latestLedger = 0;
    quint32 m_oldestLedger = 0;
    quint32 m_ledgerRetentionWindow = 0;
public:
    GetHealthResponse() = default;

    /** Builds the response from the JSON-RPC `result` object. */
    static GetHealthResponse fromJson(const QJsonObject& result);

    QString getStatus() const { return m_status; }
    quint32 getLatestLedger() const { return m_latestLedger; }
    quint32 getOldestLedger() const { return m_oldestLedger; }
    quint32 getLedgerRetentionWindow() const { return m_ledgerRetentionWindow; }
};

#endif // GETHEALTHRESPONSE_H
