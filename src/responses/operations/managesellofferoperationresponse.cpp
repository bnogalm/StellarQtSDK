#include "managesellofferoperationresponse.h"
#include "../../asset.h"
#include "../../assettypenative.h"
#include "../../keypair.h"
ManageSellOfferOperationResponse::ManageSellOfferOperationResponse(QNetworkReply * reply)
    :OperationResponse(reply)
  ,m_buyingAsset(nullptr),m_sellingAsset(nullptr)
{

}


ManageSellOfferOperationResponse::~ManageSellOfferOperationResponse()
{
    if(m_buyingAsset)
        delete m_buyingAsset;
    if(m_sellingAsset)
        delete m_sellingAsset;
}

qint64 ManageSellOfferOperationResponse::getOfferId() const {
    return m_offerID;
}

QString ManageSellOfferOperationResponse::getAmount() const {
    return m_amount;
}

QString ManageSellOfferOperationResponse::getPrice() const{
    return m_price;
}

Asset* ManageSellOfferOperationResponse::getBuyingAsset() {
    if(m_buyingAsset)
        return m_buyingAsset;
    if (m_buyingAssetType =="native") {
        m_buyingAsset= new AssetTypeNative();
    } else {
        KeyPair* issuer = KeyPair::fromAccountId(m_buyingAssetIssuer);
        m_buyingAsset =Asset::createNonNativeAsset(m_buyingAssetCode, issuer);
    }
    return m_buyingAsset;
}

Asset* ManageSellOfferOperationResponse::getSellingAsset() {
    if(m_sellingAsset)
        return m_sellingAsset;
    if (m_sellingAssetType =="native") {
        m_sellingAsset= new AssetTypeNative();
    } else {
        KeyPair* issuer = KeyPair::fromAccountId(m_sellingAssetIssuer);
        m_sellingAsset =Asset::createNonNativeAsset(m_sellingAssetCode, issuer);
    }
    return m_sellingAsset;
}

QString ManageSellOfferOperationResponse::buyingAssetType() const
{
    return m_buyingAssetType;
}

QString ManageSellOfferOperationResponse::buyingAssetCode() const
{
    return m_buyingAssetCode;
}

QString ManageSellOfferOperationResponse::buyingAssetIssuer() const
{
    return m_buyingAssetIssuer;
}

QString ManageSellOfferOperationResponse::sellingAssetType() const
{
    return m_sellingAssetType;
}

QString ManageSellOfferOperationResponse::sellingAssetCode() const
{
    return m_sellingAssetCode;
}

QString ManageSellOfferOperationResponse::sellingAssetIssuer() const
{
    return m_sellingAssetIssuer;
}

void ManageSellOfferOperationResponse::setBuyingAssetType(QString buyingAssetType)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=nullptr;
    }
    m_buyingAssetType = buyingAssetType;
}

void ManageSellOfferOperationResponse::setBuyingAssetCode(QString buyingAssetCode)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=nullptr;
    }
    m_buyingAssetCode = buyingAssetCode;
}

void ManageSellOfferOperationResponse::setBuyingAssetIssuer(QString buyingAssetIssuer)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=nullptr;
    }
    m_buyingAssetIssuer = buyingAssetIssuer;
}

void ManageSellOfferOperationResponse::setSellingAssetType(QString sellingAssetType)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=nullptr;
    }
    m_sellingAssetType = sellingAssetType;
}

void ManageSellOfferOperationResponse::setSellingAssetCode(QString sellingAssetCode)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=nullptr;
    }
    m_sellingAssetCode = sellingAssetCode;
}

void ManageSellOfferOperationResponse::setSellingAssetIssuer(QString sellingAssetIssuer)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=nullptr;
    }
    m_sellingAssetIssuer = sellingAssetIssuer;
}
