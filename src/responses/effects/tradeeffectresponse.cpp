#include "tradeeffectresponse.h"
#include "../../keypair.h"
#include "../../assettypenative.h"
#include "../../assettypecreditalphanum.h"

TradeEffectResponse::TradeEffectResponse(QNetworkReply *reply)
    :EffectResponse(reply)
    ,m_sellerKeypair(nullptr)
    ,m_offerId(0)
    ,m_soldAsset(nullptr)
    ,m_boughtAsset(nullptr)
{

}

TradeEffectResponse::~TradeEffectResponse()
{
    if(m_sellerKeypair)
        delete m_sellerKeypair;
    if(m_boughtAsset)
        delete m_boughtAsset;
    if(m_soldAsset)
        delete m_soldAsset;

}

KeyPair &TradeEffectResponse::getSeller() {
    if(!m_sellerKeypair){
        m_sellerKeypair = KeyPair::fromAccountId(m_seller);
    }
    return *m_sellerKeypair;
}

qint64 TradeEffectResponse::getOfferId() const {
    return m_offerId;
}

QString TradeEffectResponse::getSoldAmount() const{
    return m_soldAmount;
}

QString TradeEffectResponse::getBoughtAmount() const{
    return m_boughtAmount;
}

Asset *TradeEffectResponse::getSoldAsset() {
    if(!m_soldAsset){
        if (m_soldAssetType=="native") {
            m_soldAsset= new AssetTypeNative();
        } else {
            KeyPair* issuer = KeyPair::fromAccountId(m_soldAssetIssuer);
            m_soldAsset= Asset::createNonNativeAsset(m_soldAssetCode, issuer);
        }
    }
    return m_soldAsset;
}

Asset *TradeEffectResponse::getBoughtAsset() {
    if(!m_boughtAsset){
        if (m_boughtAssetType=="native") {
            m_boughtAsset= new AssetTypeNative();
        } else {
            KeyPair* issuer = KeyPair::fromAccountId(m_boughtAssetIssuer);
            m_boughtAsset= Asset::createNonNativeAsset(m_boughtAssetCode, issuer);
        }
    }
    return m_boughtAsset;
}

QString TradeEffectResponse::seller() const
{
    return m_seller;
}

QString TradeEffectResponse::soldAssetCode() const
{
    return m_soldAssetCode;
}

QString TradeEffectResponse::soldAssetIssuer() const
{
    return m_soldAssetIssuer;
}

QString TradeEffectResponse::boughtAssetType() const
{
    return m_boughtAssetType;
}

QString TradeEffectResponse::boughtAssetCode() const
{
    return m_boughtAssetCode;
}

QString TradeEffectResponse::boughtAssetIssuer() const
{
    return m_boughtAssetIssuer;
}

QString TradeEffectResponse::soldAssetType() const
{
    return m_soldAssetType;
}

void TradeEffectResponse::setSeller(QString seller)
{
    m_seller = seller;
}

void TradeEffectResponse::setSoldAssetCode(QString soldAssetCode)
{
    if(m_soldAsset){
        delete m_soldAsset;
        m_soldAsset=nullptr;
    }
    m_soldAssetCode = soldAssetCode;
}

void TradeEffectResponse::setSoldAssetIssuer(QString soldAssetIssuer)
{
    if(m_soldAsset){
        delete m_soldAsset;
        m_soldAsset=nullptr;
    }
    m_soldAssetIssuer = soldAssetIssuer;
}
void TradeEffectResponse::setSoldAssetType(QString soldAssetType)
{
    if(m_soldAsset){
        delete m_soldAsset;
        m_soldAsset=nullptr;
    }
    m_soldAssetType = soldAssetType;
}

void TradeEffectResponse::setBoughtAssetType(QString boughtAssetType)
{
    if(m_boughtAsset){
        delete m_boughtAsset;
        m_boughtAsset=nullptr;
    }
    m_boughtAssetType = boughtAssetType;
}

void TradeEffectResponse::setBoughtAssetCode(QString boughtAssetCode)
{
    if(m_boughtAsset){
        delete m_boughtAsset;
        m_boughtAsset=nullptr;
    }
    m_boughtAssetCode = boughtAssetCode;
}

void TradeEffectResponse::setBoughtAssetIssuer(QString boughtAssetIssuer)
{
    if(m_boughtAsset){
        delete m_boughtAsset;
        m_boughtAsset=nullptr;
    }
    m_boughtAssetIssuer = boughtAssetIssuer;
}

