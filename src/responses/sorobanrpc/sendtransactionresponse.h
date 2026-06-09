#ifndef SENDTRANSACTIONRESPONSE_H
#define SENDTRANSACTIONRESPONSE_H

#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/** Soroban RPC `sendTransaction` result. */
class SendTransactionResponse
{
public:
    enum class Status { PENDING, DUPLICATE, TRY_AGAIN_LATER, ERROR_ };

private:
    Status m_status = Status::PENDING;
    QString m_hash;
    quint32 m_latestLedger = 0;
    QString m_latestLedgerCloseTime;
    QString m_errorResultXdr;          // present when status == ERROR_
    QStringList m_diagnosticEventsXdr;

public:
    static SendTransactionResponse fromJson(const QJsonObject& result);

    Status getStatus() const { return m_status; }
    QString getHash() const { return m_hash; }
    quint32 getLatestLedger() const { return m_latestLedger; }
    QString getLatestLedgerCloseTime() const { return m_latestLedgerCloseTime; }
    QString getErrorResultXdr() const { return m_errorResultXdr; }
    QStringList getDiagnosticEventsXdr() const { return m_diagnosticEventsXdr; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(SendTransactionResponse)




#endif // SENDTRANSACTIONRESPONSE_H
