#ifndef GETTRANSACTIONRESPONSE_H
#define GETTRANSACTIONRESPONSE_H

#include <QJsonObject>
#include <QString>
#include <QtGlobal>

/** Soroban RPC `getTransaction` result. */
class GetTransactionResponse
{
public:
    enum class Status { NOT_FOUND, SUCCESS, FAILED };

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
};

#endif // GETTRANSACTIONRESPONSE_H
