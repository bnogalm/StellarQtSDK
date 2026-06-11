#include "tradeaggregationresponse.h"

QSTELLAR_BEGIN_NS


TradeAggregationResponse::TradeAggregationResponse(QNetworkReply* reply):Response(reply),m_timestamp(0),m_tradeCount(0)
{

}

TradeAggregationResponse::~TradeAggregationResponse()
{

}

qint64 TradeAggregationResponse::getTimestamp() const{
    return m_timestamp;
}

QDateTime TradeAggregationResponse::getDate() const{
    return QDateTime::fromMSecsSinceEpoch(m_timestamp);
}

qint32 TradeAggregationResponse::getTradeCount() const{
    return m_tradeCount;
}

QString TradeAggregationResponse::getBaseVolume() const{
    return m_baseVolume;
}

QString TradeAggregationResponse::getCounterVolume() const{
    return m_counterVolume;
}

QString TradeAggregationResponse::getAvg() const{
    return m_avg;
}

QString TradeAggregationResponse::getHigh() const{
    return m_high;
}

QString TradeAggregationResponse::getLow() const{
    return m_low;
}

QString TradeAggregationResponse::getOpen() const{
    return m_open;
}

QString TradeAggregationResponse::getClose() const{
    return m_close;
}

Price TradeAggregationResponse::getHighR() const{
    return Price(m_highR.value("n").toUInt(), m_highR.value("d").toUInt());
}

Price TradeAggregationResponse::getLowR() const{
    return Price(m_lowR.value("n").toUInt(), m_lowR.value("d").toUInt());
}

Price TradeAggregationResponse::getOpenR() const{
    return Price(m_openR.value("n").toUInt(), m_openR.value("d").toUInt());
}

Price TradeAggregationResponse::getCloseR() const{
    return Price(m_closeR.value("n").toUInt(), m_closeR.value("d").toUInt());
}
QSTELLAR_END_NS
