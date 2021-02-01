#include "offerresponse.h"
#include "common.h"

OfferResponse::OfferResponse(QNetworkReply *reply)
    :Response(reply)
    ,m_id(0)
    ,m_sellerKeypair(nullptr),m_sellerAsset(nullptr),m_buyingAsset(nullptr),m_lastModifiedLedger(0)
{

}

OfferResponse::~OfferResponse()
{
    if(m_sellerKeypair)
    {
        delete m_sellerKeypair;
    }
    if(m_buyingAsset)
    {
        delete m_buyingAsset;
    }
    if(m_sellerAsset)
    {
        delete m_sellerAsset;
    }
}

qint64 OfferResponse::getId() const{
    return m_id;
}

QString OfferResponse::getPagingToken() const{
    return m_pagingToken;
}

KeyPair *OfferResponse::getSeller(){
    if(!m_sellerKeypair)
        m_sellerKeypair = KeyPair::fromAccountId(m_seller);
    return m_sellerKeypair;
}

Asset *OfferResponse::getSelling(){
    if(!m_sellerAsset)
        m_sellerAsset = assetFromVariantMap(m_selling);
    return m_sellerAsset;
}

Asset *OfferResponse::getBuying(){
    if(!m_buyingAsset)
        m_buyingAsset = assetFromVariantMap(m_buying);
    return m_buyingAsset;
}

QString OfferResponse::getAmount(){
    return m_amount;
}

QString OfferResponse::getPrice() const{
    return m_price;
}

qint32 OfferResponse::getLastModifiedLedger() const
{
    return m_lastModifiedLedger;
}

QString OfferResponse::getLastModifiedTime() const
{
    return m_lastModifiedTime;
}

OfferResponseAttach::Links OfferResponse::getLinks() const{
    return m_links;
}

QString OfferResponse::seller() const
{
    return m_seller;
}

QVariantMap OfferResponse::buying() const
{
    return m_buying;
}

QVariantMap OfferResponse::selling() const
{
    return m_selling;
}

QString OfferResponse::getSponsor() const
{
    return m_sponsor;
}

void OfferResponse::setSeller(QString seller)
{
    if(m_sellerKeypair)
    {
        delete m_sellerKeypair;
        m_sellerKeypair=nullptr;
    }
    m_seller = seller;
}

void OfferResponse::setBuying(QVariantMap buying)
{
    if(m_buyingAsset)
    {
        delete m_buyingAsset;
        m_buyingAsset=nullptr;
    }
    m_buying = buying;
}

void OfferResponse::setSelling(QVariantMap selling)
{
    if(m_sellerAsset)
    {
        delete m_sellerAsset;
        m_sellerAsset=nullptr;
    }
    m_selling = selling;
}

bool OfferResponseAttach::Links::operator !=(OfferResponseAttach::Links &links)
{
    Q_UNUSED(links)
    return true;
}
