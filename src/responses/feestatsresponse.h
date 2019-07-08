#ifndef FEESTATSRESPONSE_H
#define FEESTATSRESPONSE_H

#include "response.h"

class FeeStatsResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(qint64 min_accepted_fee MEMBER m_min)
    Q_PROPERTY(qint64 mode_accepted_fee MEMBER m_mode)

    Q_PROPERTY(qint64 p10_accepted_fee MEMBER m_p10)
    Q_PROPERTY(qint64 p20_accepted_fee MEMBER m_p20)
    Q_PROPERTY(qint64 p30_accepted_fee MEMBER m_p30)
    Q_PROPERTY(qint64 p40_accepted_fee MEMBER m_p40)
    Q_PROPERTY(qint64 p50_accepted_fee MEMBER m_p50)
    Q_PROPERTY(qint64 p60_accepted_fee MEMBER m_p60)
    Q_PROPERTY(qint64 p70_accepted_fee MEMBER m_p70)
    Q_PROPERTY(qint64 p80_accepted_fee MEMBER m_p80)
    Q_PROPERTY(qint64 p90_accepted_fee MEMBER m_p90)
    Q_PROPERTY(qint64 p95_accepted_fee MEMBER m_p95)
    Q_PROPERTY(qint64 p99_accepted_fee MEMBER m_p99)
    Q_PROPERTY(float ledger_capacity_usage MEMBER m_ledgerCapacityUsage)
    Q_PROPERTY(qint64 last_ledger_base_fee MEMBER m_lastLedgerBaseFee)
    Q_PROPERTY(qint64 last_ledger MEMBER m_lastLedger)
    qint64 m_min;
    qint64 m_mode;

    qint64 m_p10;
    qint64 m_p20;
    qint64 m_p30;
    qint64 m_p40;
    qint64 m_p50;
    qint64 m_p60;
    qint64 m_p70;
    qint64 m_p80;
    qint64 m_p90;
    qint64 m_p95;
    qint64 m_p99;
    float m_ledgerCapacityUsage;


    qint64 m_lastLedgerBaseFee;
    qint64 m_lastLedger;

public:
    Q_INVOKABLE explicit FeeStatsResponse(QNetworkReply* reply=nullptr);
    virtual ~FeeStatsResponse();

    qint64 getMin() const;

    qint64 getMode() const;

    qint64 getP10() const;

    qint64 getP20() const;

    qint64 getP30() const;

    qint64 getP40() const;

    qint64 getP50() const;

    qint64 getP60() const;

    qint64 getP70() const;

    qint64 getP80() const;

    qint64 getP90() const;

    qint64 getP95() const;

    qint64 getP99() const;

    float getLedgerCapacityUsage() const;

    qint64 getLastLedgerBaseFee() const;

    qint64 getLastLedger() const;

};

#endif // FEESTATSRESPONSE_H




