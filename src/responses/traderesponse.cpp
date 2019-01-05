#include "traderesponse.h"
#include "../keypair.h"
#include "../asset.h"
TradeResponse::TradeResponse(QNetworkReply *reply)
    :Response(reply)
    ,m_baseIsSeller(false)
    ,m_baseAccountKeypair(nullptr),m_counterAccountKeypair(nullptr)
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
    return m_price;
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
