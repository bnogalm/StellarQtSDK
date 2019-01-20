#ifndef TRADEAGGREGATIONRESPONSE_H
#define TRADEAGGREGATIONRESPONSE_H

#include "response.h"

class TradeAggregationResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(qint64 timestamp MEMBER m_timestamp READ getTimestamp)
    Q_PROPERTY(qint32 trade_count MEMBER m_tradeCount READ getTradeCount)
    Q_PROPERTY(QString base_volume MEMBER m_baseVolume READ getBaseVolume)
    Q_PROPERTY(QString counter_volume MEMBER m_counterVolume READ getCounterVolume)
    Q_PROPERTY(QString avg MEMBER m_avg READ getAvg)
    Q_PROPERTY(QString high MEMBER m_high READ getHigh)
    Q_PROPERTY(QString low MEMBER m_low READ getLow)
    Q_PROPERTY(QString open MEMBER m_open READ getOpen)
    Q_PROPERTY(QString close MEMBER m_close READ getClose)

    qint64 m_timestamp;
    qint32 m_tradeCount;
    QString m_baseVolume;
    QString m_counterVolume;
    QString m_avg;
    QString m_high;
    QString m_low;
    QString m_open;
    QString m_close;

public:
    Q_INVOKABLE explicit TradeAggregationResponse(QNetworkReply* reply=nullptr);
    virtual ~TradeAggregationResponse();
    qint64 getTimestamp() const;
    QDateTime getDate() const;
    qint32 getTradeCount() const;
    QString getBaseVolume() const;
    QString getCounterVolume() const;
    QString getAvg() const;
    QString getHigh() const;
    QString getLow() const;
    QString getOpen() const;
    QString getClose() const;
};

#endif // TRADEAGGREGATIONRESPONSE_H
