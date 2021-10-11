#include "managebuyofferoperationresponse.h"
#include "../../asset.h"
#include "../../assettypenative.h"
#include "../../keypair.h"
ManageBuyOfferOperationResponse::ManageBuyOfferOperationResponse(QNetworkReply * reply)
    :OperationResponse(reply)
  ,m_buyingAsset(nullptr),m_sellingAsset(nullptr)
{

}


ManageBuyOfferOperationResponse::~ManageBuyOfferOperationResponse()
{
    if(m_buyingAsset)
        delete m_buyingAsset;
    if(m_sellingAsset)
        delete m_sellingAsset;
}

qint64 ManageBuyOfferOperationResponse::getOfferId() const {
    return m_offerID;
}

QString ManageBuyOfferOperationResponse::getAmount() const {
    return m_amount;
}

QString ManageBuyOfferOperationResponse::getPrice() const{
    return m_price;
}

Asset* ManageBuyOfferOperationResponse::getBuyingAsset() {
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

Asset* ManageBuyOfferOperationResponse::getSellingAsset() {
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

QString ManageBuyOfferOperationResponse::buyingAssetType() const
{
    return m_buyingAssetType;
}

QString ManageBuyOfferOperationResponse::buyingAssetCode() const
{
    return m_buyingAssetCode;
}

QString ManageBuyOfferOperationResponse::buyingAssetIssuer() const
{
    return m_buyingAssetIssuer;
}

QString ManageBuyOfferOperationResponse::sellingAssetType() const
{
    return m_sellingAssetType;
}

QString ManageBuyOfferOperationResponse::sellingAssetCode() const
{
    return m_sellingAssetCode;
}

QString ManageBuyOfferOperationResponse::sellingAssetIssuer() const
{
    return m_sellingAssetIssuer;
}

void ManageBuyOfferOperationResponse::setBuyingAssetType(QString buyingAssetType)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=nullptr;
    }
    m_buyingAssetType = buyingAssetType;
}

void ManageBuyOfferOperationResponse::setBuyingAssetCode(QString buyingAssetCode)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=nullptr;
    }
    m_buyingAssetCode = buyingAssetCode;
}

void ManageBuyOfferOperationResponse::setBuyingAssetIssuer(QString buyingAssetIssuer)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=nullptr;
    }
    m_buyingAssetIssuer = buyingAssetIssuer;
}

void ManageBuyOfferOperationResponse::setSellingAssetType(QString sellingAssetType)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=nullptr;
    }
    m_sellingAssetType = sellingAssetType;
}

void ManageBuyOfferOperationResponse::setSellingAssetCode(QString sellingAssetCode)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=nullptr;
    }
    m_sellingAssetCode = sellingAssetCode;
}

void ManageBuyOfferOperationResponse::setSellingAssetIssuer(QString sellingAssetIssuer)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=nullptr;
    }
    m_sellingAssetIssuer = sellingAssetIssuer;
}
