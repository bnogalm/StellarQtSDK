#ifndef SIMULATETRANSACTIONRESPONSE_H
#define SIMULATETRANSACTIONRESPONSE_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/** A single result from `simulateTransaction` (one per operation). */
struct SimulateHostFunctionResult
{
    QString xdr;                   // base64 SCVal — the function's return value
    QStringList auth;              // base64 SorobanAuthorizationEntry list
};

/** A state-change entry returned by `simulateTransaction`. */
struct SimulateStateChange
{
    QString type;        // "created" | "updated" | "deleted"
    QString key;         // base64 LedgerKey
    QString before;      // base64 LedgerEntry (absent for "created")
    QString after;       // base64 LedgerEntry (absent for "deleted")
};

/**
 * Present when the simulation requires a RestoreFootprint operation to be
 * applied first (because some of the contract's storage has been archived).
 * `transactionData` is the SorobanTransactionData for the restore tx;
 * `minResourceFee` is its resource fee.
 */
struct SimulateRestorePreamble
{
    QString transactionData;   // base64 SorobanTransactionData
    QString minResourceFee;    // string to preserve uint64
    bool present = false;
};

/** Soroban RPC `simulateTransaction` result. */
class SimulateTransactionResponse
{
    quint32 m_latestLedger = 0;
    QStringList m_events;          // base64 ContractEvent XDR
    QString m_transactionData;     // base64 SorobanTransactionData
    QString m_minResourceFee;      // string to preserve uint64 precision
    QList<SimulateHostFunctionResult> m_results;
    QList<SimulateStateChange> m_stateChanges;
    QString m_error;               // present when simulation failed
    SimulateRestorePreamble m_restorePreamble;
public:
    static SimulateTransactionResponse fromJson(const QJsonObject& result);

    quint32 getLatestLedger() const { return m_latestLedger; }
    QStringList getEvents() const { return m_events; }
    QString getTransactionData() const { return m_transactionData; }
    QString getMinResourceFee() const { return m_minResourceFee; }
    const QList<SimulateHostFunctionResult>& getResults() const { return m_results; }
    const QList<SimulateStateChange>& getStateChanges() const { return m_stateChanges; }
    QString getError() const { return m_error; }
    const SimulateRestorePreamble& getRestorePreamble() const { return m_restorePreamble; }

    /** True iff the simulation failed (carries an `error` payload). */
    bool isError() const { return !m_error.isEmpty(); }
    /** True iff the simulation needs a RestoreFootprint applied first. */
    bool needsRestore() const { return m_restorePreamble.present; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(SimulateTransactionResponse)




#endif // SIMULATETRANSACTIONRESPONSE_H
