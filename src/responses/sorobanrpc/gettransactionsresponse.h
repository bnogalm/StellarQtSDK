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
 *  single getTransaction result. */
class GetTransactionsResponse
{
    QList<GetTransactionResponse> m_transactions;
    quint32 m_latestLedger = 0;
    QString m_latestLedgerCloseTime;
    quint32 m_oldestLedger = 0;
    QString m_oldestLedgerCloseTime;
    QString m_cursor;
public:
    static GetTransactionsResponse fromJson(const QJsonObject& result);
    const QList<GetTransactionResponse>& getTransactions() const { return m_transactions; }
    quint32 getLatestLedger() const { return m_latestLedger; }
    QString getLatestLedgerCloseTime() const { return m_latestLedgerCloseTime; }
    quint32 getOldestLedger() const { return m_oldestLedger; }
    QString getOldestLedgerCloseTime() const { return m_oldestLedgerCloseTime; }
    QString getCursor() const { return m_cursor; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(GetTransactionsResponse)




#endif // GETTRANSACTIONSRESPONSE_H
