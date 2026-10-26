#ifndef GETEVENTSRESPONSE_H
#define GETEVENTSRESPONSE_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/** A single Soroban contract event. */
struct EventInfo
{
    QString type;                // "system" | "contract" | "diagnostic"
    quint32 ledger = 0;
    QString ledgerClosedAt;
    QString contractId;          // C-strkey
    QString id;                  // event id (paging cursor)
    qint32 operationIndex = 0;   // position of the op within its transaction
    qint32 transactionIndex = 0; // position of the tx within its ledger
    qint32 pagingToken = 0;      // deprecated, kept for compat: always 0 (the RPC's was the id, as text); use id
    /** Whether the event came from a successful call, as the RPC sent it
     *  (stellar-rpc 23 and later send whether its transaction succeeded: false
     *  only on the fee events of a failed transaction). When the RPC sent
     *  nothing it is true, as 1.0.3 read a missing one, whatever happened to
     *  the transaction: check hasInSuccessfulContractCall before relying on
     *  it. The RPC deprecated the field in 23.0 and its protocol package has
     *  dropped it, so an answer without it is read, not refused. */
    bool inSuccessfulContractCall = true;
    QString txHash;
    QStringList topic;           // base64 SCVal XDR strings
    QString value;               // base64 SCVal XDR
    /** Whether the RPC sent inSuccessfulContractCall. When it is false,
     *  inSuccessfulContractCall is true and tells nothing. */
    bool hasInSuccessfulContractCall = false;
};

/** Soroban RPC `getEvents` result. */
class GetEventsResponse
{
    QList<EventInfo> m_events;
    quint32 m_latestLedger = 0;
    quint32 m_oldestLedger = 0;
    QString m_latestLedgerCloseTime;
    QString m_oldestLedgerCloseTime;
    QString m_cursor;
public:
    /** Throws std::runtime_error when a field is missing or not what the RPC
     *  sends: a number that is not a JSON integer in range (or, for the close
     *  times, a decimal string), an unknown event type, a txHash that is not 64
     *  hex digits, a contractId that is neither empty nor a C... strkey, a
     *  topic or value that is not base64 XDR, or an inSuccessfulContractCall
     *  that is there but not true or false (it may be left out). */
    static GetEventsResponse fromJson(const QJsonObject& result);
    const QList<EventInfo>& getEvents() const { return m_events; }
    quint32 getLatestLedger() const { return m_latestLedger; }
    quint32 getOldestLedger() const { return m_oldestLedger; }
    /** Unix-seconds (string) close time of the latest ledger. */
    QString getLatestLedgerCloseTime() const { return m_latestLedgerCloseTime; }
    /** Unix-seconds (string) close time of the oldest retained ledger. */
    QString getOldestLedgerCloseTime() const { return m_oldestLedgerCloseTime; }
    QString getCursor() const { return m_cursor; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(GetEventsResponse)




#endif // GETEVENTSRESPONSE_H
