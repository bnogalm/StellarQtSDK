#ifndef GETEVENTSRESPONSE_H
#define GETEVENTSRESPONSE_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QtGlobal>

/** A single Soroban contract event. */
struct EventInfo
{
    QString type;                // "system" | "contract" | "diagnostic"
    quint32 ledger = 0;
    QString ledgerClosedAt;
    QString contractId;          // C-strkey
    QString id;                  // event id (paging cursor)
    qint32 pagingToken = 0;      // deprecated, kept for compat
    bool inSuccessfulContractCall = true;
    QString txHash;
    QStringList topic;           // base64 SCVal XDR strings
    QString value;               // base64 SCVal XDR
};

/** Soroban RPC `getEvents` result. */
class GetEventsResponse
{
    QList<EventInfo> m_events;
    quint32 m_latestLedger = 0;
    QString m_cursor;
public:
    static GetEventsResponse fromJson(const QJsonObject& result);
    const QList<EventInfo>& getEvents() const { return m_events; }
    quint32 getLatestLedger() const { return m_latestLedger; }
    QString getCursor() const { return m_cursor; }
};

#endif // GETEVENTSRESPONSE_H
