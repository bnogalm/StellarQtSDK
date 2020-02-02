#ifndef FEESTATSRESPONSE_H
#define FEESTATSRESPONSE_H

#include "response.h"

class FeeDistribution
{
    Q_GADGET
    Q_PROPERTY(qint64 min MEMBER m_min)
    Q_PROPERTY(qint64 max MEMBER m_max)
    Q_PROPERTY(qint64 mode MEMBER m_mode)

    Q_PROPERTY(qint64 p10 MEMBER m_p10)
    Q_PROPERTY(qint64 p20 MEMBER m_p20)
    Q_PROPERTY(qint64 p30 MEMBER m_p30)
    Q_PROPERTY(qint64 p40 MEMBER m_p40)
    Q_PROPERTY(qint64 p50 MEMBER m_p50)
    Q_PROPERTY(qint64 p60 MEMBER m_p60)
    Q_PROPERTY(qint64 p70 MEMBER m_p70)
    Q_PROPERTY(qint64 p80 MEMBER m_p80)
    Q_PROPERTY(qint64 p90 MEMBER m_p90)
    Q_PROPERTY(qint64 p95 MEMBER m_p95)
    Q_PROPERTY(qint64 p99 MEMBER m_p99)
    qint64 m_min;
    qint64 m_max;
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
public:
    FeeDistribution();
    qint64 getMin() const;

    qint64 getMax() const;

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

    bool operator !=(FeeDistribution& feeDistribution);
    bool operator ==(FeeDistribution& feeDistribution);
};

class FeeStatsResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(float ledger_capacity_usage MEMBER m_ledgerCapacityUsage)
    Q_PROPERTY(qint64 last_ledger_base_fee MEMBER m_lastLedgerBaseFee)
    Q_PROPERTY(qint64 last_ledger MEMBER m_lastLedger)
    Q_PROPERTY(FeeDistribution fee_charged MEMBER m_feeCharged)
    Q_PROPERTY(FeeDistribution max_fee MEMBER m_maxFee)

    float m_ledgerCapacityUsage;


    qint64 m_lastLedgerBaseFee;
    qint64 m_lastLedger;
    FeeDistribution m_feeCharged;
    FeeDistribution m_maxFee;

public:
    Q_INVOKABLE explicit FeeStatsResponse(QNetworkReply* reply=nullptr);
    virtual ~FeeStatsResponse();


    float getLedgerCapacityUsage() const;

    qint64 getLastLedgerBaseFee() const;

    qint64 getLastLedger() const;

    FeeDistribution getFeeCharged() const;

    FeeDistribution getMaxFee() const;

};

Q_DECLARE_METATYPE(FeeDistribution)

#endif // FEESTATSRESPONSE_H




