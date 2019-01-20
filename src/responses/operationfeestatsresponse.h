#ifndef OPERATIONFEESTATSRESPONSE_H
#define OPERATIONFEESTATSRESPONSE_H

#include "response.h"

class OperationFeeStatsResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(qint64 min_accepted_fee MEMBER m_min)
    Q_PROPERTY(qint64 mode_accepted_fee MEMBER m_mode)
    Q_PROPERTY(qint64 last_ledger_base_fee MEMBER m_lastLedgerBaseFee)
    Q_PROPERTY(qint64 last_ledger MEMBER m_lastLedger)
    qint64 m_min;
    qint64 m_mode;
    qint64 m_lastLedgerBaseFee;
    qint64 m_lastLedger;

public:
    Q_INVOKABLE explicit OperationFeeStatsResponse(QNetworkReply* reply=nullptr);
    virtual ~OperationFeeStatsResponse();

    qint64 getMin() const;

    qint64 getMode() const;

    qint64 getLastLedgerBaseFee() const;

    qint64 getLastLedger() const;

};

#endif // OPERATIONFEESTATSRESPONSE_H




