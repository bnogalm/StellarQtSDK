#include "orderbookresponse.h"
#include "../asset.h"
#include "../assettypecreditalphanum.h"
#include "../assettypenative.h"
#include "../keypair.h"
#include "common.h"

static void registerTypes()
{
    qRegisterMetaType<OrderBookResponseAttach::Row>();
    QMetaType::registerConverter<QVariantList ,QList<OrderBookResponseAttach::Row> >(&ResponseConverters::convert<OrderBookResponseAttach::Row>);
}

Q_COREAPP_STARTUP_FUNCTION(registerTypes)

QString OrderBookResponseAttach::Row::getAmount() {
    return m_amount;
}

QString OrderBookResponseAttach::Row::getPrice() {
    return m_price;
}

Price OrderBookResponseAttach::Row::getPriceR() {
    return Price(m_priceR.value("n").toUInt(), m_priceR.value("d").toUInt());
}

bool OrderBookResponseAttach::Row::operator !=(const Row &row) const
{
    Q_UNUSED(row)
    return true;
}

bool OrderBookResponseAttach::Row::operator ==(const Row &row) const
{
    return (this->m_amount==row.m_amount)
            && (this->m_price==row.m_price)
            && (this->m_priceR==row.m_priceR);
}

OrderBookResponse::OrderBookResponse(QNetworkReply *reply)
    :Response(reply),m_baseAsset(nullptr),m_counterAsset(nullptr)
{

}

OrderBookResponse::~OrderBookResponse()
{
    if(m_baseAsset)
        delete m_baseAsset;
    if(m_counterAsset)
        delete m_counterAsset;
}


Asset *OrderBookResponse::getBase() {
    if(!m_baseAsset)
        m_baseAsset = assetFromVariantMap(m_base);
    return m_baseAsset;
}

Asset *OrderBookResponse::getCounter() {
    if(!m_counterAsset)
        m_counterAsset = assetFromVariantMap(m_counter);
    return m_counterAsset;
}

QList<OrderBookResponseAttach::Row> OrderBookResponse::getAsks() {
    return m_asks;
}

QList<OrderBookResponseAttach::Row> OrderBookResponse::getBids() {
    return m_bids;
}

QVariantMap OrderBookResponse::base() const
{
    return m_base;
}

QVariantMap OrderBookResponse::counter() const
{
    return m_counter;
}

void OrderBookResponse::setBase(QVariantMap base)
{
    if(m_baseAsset){
        delete m_baseAsset;
        m_baseAsset=nullptr;
    }
    m_base = base;
}

void OrderBookResponse::setCounter(QVariantMap counter)
{
    if(m_counterAsset){
        delete m_counterAsset;
        m_counterAsset=nullptr;
    }
    m_counter = counter;
}
