#include "manageofferoperationresponse.h"
#include "../../asset.h"
#include "../../assettypenative.h"
#include "../../keypair.h"
ManageOfferOperationResponse::ManageOfferOperationResponse(QNetworkReply * reply)
    :OperationResponse(reply)
  ,m_buyingAsset(0),m_sellingAsset(0)
{

}


ManageOfferOperationResponse::~ManageOfferOperationResponse()
{
    if(m_buyingAsset)
        delete m_buyingAsset;
    if(m_sellingAsset)
        delete m_sellingAsset;
}

qint32 ManageOfferOperationResponse::getOfferId() const {
    return m_offerID;
}

QString ManageOfferOperationResponse::getAmount() const {
    return m_amount;
}

QString ManageOfferOperationResponse::getPrice() const{
    return m_price;
}

Asset* ManageOfferOperationResponse::getBuyingAsset() {
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

Asset* ManageOfferOperationResponse::getSellingAsset() {
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

QString ManageOfferOperationResponse::buyingAssetType() const
{
    return m_buyingAssetType;
}

QString ManageOfferOperationResponse::buyingAssetCode() const
{
    return m_buyingAssetCode;
}

QString ManageOfferOperationResponse::buyingAssetIssuer() const
{
    return m_buyingAssetIssuer;
}

QString ManageOfferOperationResponse::sellingAssetType() const
{
    return m_sellingAssetType;
}

QString ManageOfferOperationResponse::sellingAssetCode() const
{
    return m_sellingAssetCode;
}

QString ManageOfferOperationResponse::sellingAssetIssuer() const
{
    return m_sellingAssetIssuer;
}

void ManageOfferOperationResponse::setBuyingAssetType(QString buyingAssetType)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=0;
    }
    m_buyingAssetType = buyingAssetType;
}

void ManageOfferOperationResponse::setBuyingAssetCode(QString buyingAssetCode)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=0;
    }
    m_buyingAssetCode = buyingAssetCode;
}

void ManageOfferOperationResponse::setBuyingAssetIssuer(QString buyingAssetIssuer)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=0;
    }
    m_buyingAssetIssuer = buyingAssetIssuer;
}

void ManageOfferOperationResponse::setSellingAssetType(QString sellingAssetType)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=0;
    }
    m_sellingAssetType = sellingAssetType;
}

void ManageOfferOperationResponse::setSellingAssetCode(QString sellingAssetCode)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=0;
    }
    m_sellingAssetCode = sellingAssetCode;
}

void ManageOfferOperationResponse::setSellingAssetIssuer(QString sellingAssetIssuer)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=0;
    }
    m_sellingAssetIssuer = sellingAssetIssuer;
}
