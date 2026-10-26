#include "tradeaggregationresponse.h"
#include "traderesponse.h"

#include <stdexcept>

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

namespace {

Price exactPrice(const QVariantMap& r, const char* field, const char* getters)
{
    const qint64 n = TradeResponse::priceTerm(r, "n");
    const qint64 d = TradeResponse::priceTerm(r, "d");
    Price price;
    if (!Price::fromInt64(n, d, price))
        throw std::runtime_error(QStringLiteral("%1 %2/%3 does not fit a Price (int32 terms, "
                                                "positive denominator); use %4")
                                     .arg(QLatin1String(field)).arg(n).arg(d)
                                     .arg(QLatin1String(getters)).toStdString());
    return price;
}

} // namespace

Price TradeAggregationResponse::getHighR() const{
    return exactPrice(m_highR, "high_r", "getHighRN()/getHighRD()");
}

Price TradeAggregationResponse::getLowR() const{
    return exactPrice(m_lowR, "low_r", "getLowRN()/getLowRD()");
}

Price TradeAggregationResponse::getOpenR() const{
    return exactPrice(m_openR, "open_r", "getOpenRN()/getOpenRD()");
}

Price TradeAggregationResponse::getCloseR() const{
    return exactPrice(m_closeR, "close_r", "getCloseRN()/getCloseRD()");
}

qint64 TradeAggregationResponse::getHighRN() const{
    return TradeResponse::priceTerm(m_highR, "n");
}

qint64 TradeAggregationResponse::getHighRD() const{
    return TradeResponse::priceTerm(m_highR, "d");
}

qint64 TradeAggregationResponse::getLowRN() const{
    return TradeResponse::priceTerm(m_lowR, "n");
}

qint64 TradeAggregationResponse::getLowRD() const{
    return TradeResponse::priceTerm(m_lowR, "d");
}

qint64 TradeAggregationResponse::getOpenRN() const{
    return TradeResponse::priceTerm(m_openR, "n");
}

qint64 TradeAggregationResponse::getOpenRD() const{
    return TradeResponse::priceTerm(m_openR, "d");
}

qint64 TradeAggregationResponse::getCloseRN() const{
    return TradeResponse::priceTerm(m_closeR, "n");
}

qint64 TradeAggregationResponse::getCloseRD() const{
    return TradeResponse::priceTerm(m_closeR, "d");
}
QSTELLAR_END_NS
