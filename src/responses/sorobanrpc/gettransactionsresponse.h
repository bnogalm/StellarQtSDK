#ifndef GETTRANSACTIONSRESPONSE_H
#define GETTRANSACTIONSRESPONSE_H

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QtGlobal>

#include "gettransactionresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/** Soroban RPC `getTransactions` result. Each entry is the same shape as a
 *  single getTransaction result, without the ledger range (its latestLedger,
 *  oldestLedger and their close times are 0 and empty): the page carries it. */
class GetTransactionsResponse
{
    QList<GetTransactionResponse> m_transactions;
    quint32 m_latestLedger = 0;
    QString m_latestLedgerCloseTime;
    quint32 m_oldestLedger = 0;
    QString m_oldestLedgerCloseTime;
    QString m_cursor;
public:
    /** Throws std::runtime_error when a field is missing or not what the RPC
     *  sends (see GetTransactionResponse::fromJson; a listed transaction is
     *  SUCCESS or FAILED). */
    static GetTransactionsResponse fromJson(const QJsonObject& result);
    const QList<GetTransactionResponse>& getTransactions() const { return m_transactions; }
    quint32 getLatestLedger() const { return m_latestLedger; }
    /** Unix seconds, as decimal text (the RPC's latestLedgerCloseTimestamp). */
    QString getLatestLedgerCloseTime() const { return m_latestLedgerCloseTime; }
    quint32 getOldestLedger() const { return m_oldestLedger; }
    /** Unix seconds, as decimal text (the RPC's oldestLedgerCloseTimestamp). */
    QString getOldestLedgerCloseTime() const { return m_oldestLedgerCloseTime; }
    QString getCursor() const { return m_cursor; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(GetTransactionsResponse)




#endif // GETTRANSACTIONSRESPONSE_H
