#ifndef GETTRANSACTIONRESPONSE_H
#define GETTRANSACTIONRESPONSE_H

#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include "qstellar_namespace.h"
#include "xdr/stellarledgermeta.h"

QSTELLAR_BEGIN_NS

namespace sorobanrpcdetail { class RpcFields; }
class GetTransactionsResponse;

/** Soroban RPC `getTransaction` result. */
class GetTransactionResponse
{
public:
    enum class Status { NOT_FOUND, SUCCESS, FAILED };

    /**
     * Protocol 23+ event streams attached to a transaction result.
     * Each entry is base64-encoded XDR. `contractEventsXdr` is grouped per
     * operation, hence a list of lists. stellar-rpc's `events` object carries
     * transactionEventsXdr and contractEventsXdr only; diagnosticEventsXdr is
     * read here should a server send it, and is otherwise empty (the
     * diagnostics are the top-level getDiagnosticEventsXdr()).
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

    /** Reads a getTransaction answer (`single`) or one entry of a
     *  getTransactions page, which has no ledger range, a JSON integer for
     *  createdAt, and no NOT_FOUND. */
    static GetTransactionResponse read(const sorobanrpcdetail::RpcFields& fields, bool single);
    friend class GetTransactionsResponse;

public:
    /**
     * Throws std::runtime_error when a field is missing or not what the RPC
     * sends: a status other than SUCCESS, NOT_FOUND or FAILED (it used to
     * read as NOT_FOUND), a number that is not a JSON integer in range (or,
     * for the close times and createdAt, a decimal string), a txHash that is
     * not 64 hex digits, or XDR that is not base64 (envelopeXdr, resultXdr and
     * resultMetaXdr are required once the transaction is found).
     */
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
    /** The TransactionMeta, base64 (decoded by getTransactionMeta()). */
    QString getResultMetaXdr() const { return m_resultMetaXdr; }

    /**
     * The TransactionMeta, decoded exactly (stellar::decodeTransactionMeta):
     * every change the transaction made, its events, and for a Soroban call
     * its return value. Throws std::runtime_error when there is no meta (a
     * transaction NOT_FOUND) or it cannot be read exactly.
     */
    stellar::TransactionMeta getTransactionMeta() const;

    /**
     * Whether the transaction's meta carries the value its host function
     * returned on chain: js-stellar-sdk's getTransaction().returnValue
     * !== undefined. False for a classic transaction, for extend/restore
     * footprint and for a failed call. Throws as getTransactionMeta().
     */
    bool hasReturnValue() const;

    /**
     * The value the Soroban call returned ON CHAIN (js returnValue: meta v3/v4
     * sorobanMeta.returnValue), which can differ from the simulation's when
     * state changed in between (a counter, a minted id). Throws
     * std::runtime_error when there is none (see hasReturnValue()) or the
     * meta cannot be read exactly.
     */
    stellar::SCVal getReturnValue() const;
    quint32 getLedger() const { return m_ledger; }
    /** Unix seconds, as decimal text ("0" for a transaction not found). */
    QString getCreatedAt() const { return m_createdAt; }

    /** Whether this transaction was submitted as a fee-bump transaction. */
    bool getFeeBump() const { return m_feeBump; }
    /**
     * Diagnostic events (base64 XDR), from the top-level
     * `diagnosticEventsXdr` field, which is where stellar-rpc puts them (an
     * `events.diagnosticEventsXdr`, which stellar-rpc does not send, is used
     * only when that is empty).
     */
    QStringList getDiagnosticEventsXdr() const { return m_diagnosticEventsXdr; }
    /** Protocol 23+ event streams (diagnostic / transaction / contract). */
    Events getEvents() const { return m_events; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(GetTransactionResponse)




#endif // GETTRANSACTIONRESPONSE_H
