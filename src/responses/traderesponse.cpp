#include "traderesponse.h"
#include "../keypair.h"
TradeResponse::TradeResponse(QNetworkReply *reply)
    :Response(reply),m_sellerKeypair(nullptr),m_buyerKeypair(nullptr)
{

}

TradeResponse::~TradeResponse()
{
    if(m_sellerKeypair)
        delete m_sellerKeypair;
    if(m_buyerKeypair)
        delete m_buyerKeypair;
}

QString TradeResponse::getId() const{
    return m_id;
}

QString TradeResponse::getPagingToken() const{
    return m_pagingToken;
}

QString TradeResponse::getCreatedAt() const{
    return m_createdAt;
}

KeyPair &TradeResponse::getSeller() {
    if(!m_sellerKeypair)
    {
        m_sellerKeypair = KeyPair::fromAccountId(m_seller);
    }
    return *m_sellerKeypair;
}

QString TradeResponse::getSoldAmount() const{
    return m_soldAmount;
}

QString TradeResponse::getSoldAssetType() const{
    return m_soldAssetType;
}

QString TradeResponse::getSoldAssetCode() const{
    return m_soldAssetCode;
}

QString TradeResponse::getSoldAssetIssuer() const{
    return m_soldAssetIssuer;
}

KeyPair &TradeResponse::getBuyer() {
    if(!m_buyerKeypair)
    {
        m_buyerKeypair = KeyPair::fromAccountId(m_buyer);
    }
    return *m_buyerKeypair;
}

QString TradeResponse::getBoughtAmount() const{
    return m_boughtAmount;
}

QString TradeResponse::getBoughtAssetType() const{
    return m_boughtAssetType;
}

QString TradeResponse::getBoughtAssetCode() const{
    return m_boughtAssetCode;
}

QString TradeResponse::getBoughtAssetIssuer() const{
    return m_boughtAssetIssuer;
}

TradeResponseAttach::Links &TradeResponse::getLinks(){
    return m_links;
}

QString TradeResponse::seller() const
{
    return m_seller;
}

QString TradeResponse::buyer() const
{
    return m_buyer;
}

void TradeResponse::setSeller(QString seller)
{
    if(m_sellerKeypair && m_seller!=seller)
    {
        delete m_sellerKeypair;
        m_sellerKeypair=nullptr;
    }
    m_seller = seller;
}

void TradeResponse::setBuyer(QString buyer)
{
    if(m_buyerKeypair && m_buyer!=buyer)
    {
        delete m_buyerKeypair;
        m_buyerKeypair=nullptr;
    }
    m_buyer = buyer;
}
