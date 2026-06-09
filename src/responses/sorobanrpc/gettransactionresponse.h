#ifndef GETTRANSACTIONRESPONSE_H
#define GETTRANSACTIONRESPONSE_H

#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/** Soroban RPC `getTransaction` result. */
class GetTransactionResponse
{
public:
    enum class Status { NOT_FOUND, SUCCESS, FAILED };

    /**
     * Protocol 23+ event streams attached to a transaction result.
     * Each entry is base64-encoded XDR. `contractEventsXdr` is grouped per
     * operation, hence a list of lists.
     */
    struct Events {
        QStringList diagnosticEventsXdr;
        QStringList transactionEventsXdr;
        QList<QStringList> contractEventsXdr;
    };

private:
    Status m_status = Status::NOT_FOUND;
    QString m_txHash;
    quint32 m_latestLedger = 0;
    QString m_latestLedgerCloseTime;
    quint32 m_oldestLedger = 0;
    QString m_oldestLedgerCloseTime;
    qint32 m_applicationOrder = 0;
    QString m_envelopeXdr;
    QString m_resultXdr;
    QString m_resultMetaXdr;
    quint32 m_ledger = 0;
    QString m_createdAt;
    bool m_feeBump = false;
    QStringList m_diagnosticEventsXdr;
    Events m_events;

public:
    static GetTransactionResponse fromJson(const QJsonObject& result);

    Status getStatus() const { return m_status; }
    QString getTxHash() const { return m_txHash; }
    quint32 getLatestLedger() const { return m_latestLedger; }
    QString getLatestLedgerCloseTime() const { return m_latestLedgerCloseTime; }
    quint32 getOldestLedger() const { return m_oldestLedger; }
    QString getOldestLedgerCloseTime() const { return m_oldestLedgerCloseTime; }
    qint32 getApplicationOrder() const { return m_applicationOrder; }
    QString getEnvelopeXdr() const { return m_envelopeXdr; }
    QString getResultXdr() const { return m_resultXdr; }
    QString getResultMetaXdr() const { return m_resultMetaXdr; }
    quint32 getLedger() const { return m_ledger; }
    QString getCreatedAt() const { return m_createdAt; }

    /** Whether this transaction was submitted as a fee-bump transaction. */
    bool getFeeBump() const { return m_feeBump; }
    /**
     * Diagnostic events (base64 XDR). Sourced from the top-level
     * `diagnosticEventsXdr` field; if the server only returned the Protocol 23
     * `events` object, this mirrors `events.diagnosticEventsXdr`.
     */
    QStringList getDiagnosticEventsXdr() const { return m_diagnosticEventsXdr; }
    /** Protocol 23+ event streams (diagnostic / transaction / contract). */
    Events getEvents() const { return m_events; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(GetTransactionResponse)




#endif // GETTRANSACTIONRESPONSE_H
