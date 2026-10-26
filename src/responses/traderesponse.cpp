#include "traderesponse.h"
#include "../keypair.h"
#include "../asset.h"
#include "price.h"
#include <QCoreApplication>
#include "qtcompat.h"

#include <cmath>
#include <stdexcept>

QSTELLAR_BEGIN_NS

static void registerTypes()
{
    regType<TradeResponseAttach::Links>();
    regType<Price>();
}
Q_COREAPP_STARTUP_FUNCTION(registerTypes)

TradeResponse::TradeResponse(QNetworkReply *reply)
    :Response(reply)
    ,m_baseIsSeller(false)
    ,m_baseAccountKeypair(nullptr),m_counterAccountKeypair(nullptr)
    ,m_liquidityPoolFeeBp(0)
    ,m_baseAsset(nullptr),m_counterAsset(nullptr)
{

}

TradeResponse::~TradeResponse()
{
    if(m_baseAccountKeypair)
        delete m_baseAccountKeypair;
    if(m_counterAccountKeypair)
        delete m_counterAccountKeypair;
    if(m_baseAsset)
        delete m_baseAsset;
    if(m_counterAsset)
        delete m_counterAsset;
}

QString TradeResponse::getId() const{
    return m_id;
}

QString TradeResponse::getPagingToken() const{
    return m_pagingToken;
}

QString TradeResponse::getLedgerCloseTime() const
{
    return m_ledgerCloseTime;
}

QString TradeResponse::getOfferId() const
{
    return m_offerId;
}

bool TradeResponse::isBaseSeller() const
{
    return m_baseIsSeller;
}

QString TradeResponse::getBaseOfferId() const
{
    return m_baseOfferId;
}

KeyPair &TradeResponse::getBaseAccount() {
    if(!m_baseAccountKeypair)
    {
        m_baseAccountKeypair = KeyPair::fromAccountId(m_baseAccount);
    }
    return *m_baseAccountKeypair;
}

QString TradeResponse::getBaseAmount() const{
    return m_baseAmount;
}

QString TradeResponse::getBaseAssetType() const{
    return m_baseAssetType;
}

QString TradeResponse::getBaseAssetCode() const{
    return m_baseAssetCode;
}

QString TradeResponse::getBaseAssetIssuer() const{
    return m_baseAssetIssuer;
}

QString TradeResponse::getCounterOfferId() const
{
    return m_counterOfferId;
}

KeyPair &TradeResponse::getCounterAccount() {
    if(!m_counterAccountKeypair)
    {
        m_counterAccountKeypair = KeyPair::fromAccountId(m_counterAccount);
    }
    return *m_counterAccountKeypair;
}

QString TradeResponse::getCounterAmount() const{
    return m_counterAmount;
}

QString TradeResponse::getCounterAssetType() const{
    return m_counterAssetType;
}

QString TradeResponse::getCounterAssetCode() const{
    return m_counterAssetCode;
}

QString TradeResponse::getCounterAssetIssuer() const{
    return m_counterAssetIssuer;
}

Price TradeResponse::getPrice() const{
    const qint64 n = getPriceN(), d = getPriceD();
    Price price;
    if (!Price::fromInt64(n, d, price))
        throw std::runtime_error(QStringLiteral("trade price %1/%2 does not fit a Price (int32 terms, "
                                                "positive denominator); use getPriceN()/getPriceD()")
                                     .arg(n).arg(d).toStdString());
    return price;
}

qint64 TradeResponse::getPriceN() const{
    return priceTerm(m_priceR, "n");
}

qint64 TradeResponse::getPriceD() const{
    return priceTerm(m_priceR, "d");
}

qint64 TradeResponse::priceTerm(const QVariantMap &price, const char *key)
{
    const QVariant v = price.value(QLatin1String(key));
    bool ok = false;
    qint64 term = 0;
    switch (v.userType()) {
    case QMetaType::QString:
        // How Horizon sends it: int64 terms as text.
        term = v.toString().toLongLong(&ok, 10);
        break;
    case QMetaType::Int:
    case QMetaType::LongLong:
        // A JSON number: at 2^53 and above the parser may already have
        // rounded it (2^53 + 1 reads as 2^53), so only text is exact there.
        term = v.toLongLong();
        ok = term > -9007199254740992LL && term < 9007199254740992LL;
        break;
    case QMetaType::Double: {
        // A JSON number (Qt 5 reads every number as a double): exact only
        // below 2^53, and only when it is whole.
        const double x = v.toDouble();
        ok = std::isfinite(x) && x == std::trunc(x) && std::fabs(x) < 9007199254740992.0;
        if (ok)
            term = static_cast<qint64>(x);
        break;
    }
    default:
        break;
    }
    if (!ok)
        throw std::runtime_error(QStringLiteral("price.%1 is missing or not an int64: %2")
                                     .arg(QLatin1String(key), v.toString()).toStdString());
    return term;
}

QString TradeResponse::getTradeType() const{
    return m_tradeType;
}

qint32 TradeResponse::getLiquidityPoolFeeBp() const{
    return m_liquidityPoolFeeBp;
}

QString TradeResponse::getBaseLiquidityPoolId() const{
    return m_baseLiquidityPoolId;
}

QString TradeResponse::getCounterLiquidityPoolId() const{
    return m_counterLiquidityPoolId;
}

TradeResponseAttach::Links &TradeResponse::getLinks(){
    return m_links;
}

QString TradeResponse::baseAccount() const
{
    return m_baseAccount;
}

QString TradeResponse::counterAccount() const
{
    return m_counterAccount;
}

Asset *TradeResponse::getBaseAsset()
{
    if(!m_baseAsset)
    {
        m_baseAsset = Asset::create(m_baseAssetType,m_baseAssetCode,m_baseAssetIssuer);
    }
    return m_baseAsset;
}

Asset *TradeResponse::getCounterAsset()
{
    if(!m_counterAsset)
    {
        m_counterAsset = Asset::create(m_counterAssetType,m_counterAssetCode, m_counterAssetIssuer);
    }
    return m_counterAsset;
}

void TradeResponse::setBaseAccount(QString base_account)
{
    if(this->m_baseAccountKeypair)
    {
        delete m_baseAccountKeypair;
        m_baseAccountKeypair=nullptr;
    }
    m_baseAccount = base_account;
}

void TradeResponse::setCounterAccount(QString counter_account)
{
    if(this->m_counterAccountKeypair)
    {
        delete m_counterAccountKeypair;
        m_counterAccountKeypair=nullptr;
    }
    m_counterAccount = counter_account;
}

void TradeResponse::setBaseAssetType(QString base_asset_type)
{
    if(this->m_baseAsset)
    {
        delete m_baseAsset;
        m_baseAsset=nullptr;
    }
    m_baseAssetType = base_asset_type;
}

void TradeResponse::setBaseAssetCode(QString base_asset_code)
{
    if(this->m_baseAsset)
    {
        delete m_baseAsset;
        m_baseAsset=nullptr;
    }
    m_baseAssetCode = base_asset_code;
}

void TradeResponse::setBaseAssetIssuer(QString base_asset_issuer)
{
    if(this->m_baseAsset)
    {
        delete m_baseAsset;
        m_baseAsset=nullptr;
    }
    m_baseAssetIssuer = base_asset_issuer;
}

void TradeResponse::setCounterAssetType(QString counter_asset_type)
{
    if(m_counterAsset)
    {
        delete m_counterAsset;
        m_counterAsset = nullptr;
    }
    m_counterAssetType = counter_asset_type;
}

void TradeResponse::setCounterAssetCode(QString counter_asset_code)
{
    if(m_counterAsset)
    {
        delete m_counterAsset;
        m_counterAsset = nullptr;
    }
    m_counterAssetCode = counter_asset_code;
}

void TradeResponse::setCounterAssetIssuer(QString counter_asset_issuer)
{
    if(m_counterAsset)
    {
        delete m_counterAsset;
        m_counterAsset = nullptr;
    }
    m_counterAssetIssuer = counter_asset_issuer;
}

Link &TradeResponseAttach::Links::getBase() {
    return m_base;
}

Link &TradeResponseAttach::Links::getCounter() {
    return m_counter;
}

Link &TradeResponseAttach::Links::getOperation() {
    return m_operation;
}
QSTELLAR_END_NS
