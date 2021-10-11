#include "createpassiveofferoperationresponse.h"
#include "../../asset.h"
#include "../../keypair.h"
#include "../../asset.h"
#include "../../assettypenative.h"

CreatePassiveSellOfferOperationResponse::CreatePassiveSellOfferOperationResponse(QNetworkReply *reply)
    :OperationResponse(reply),m_offerID(0),m_buyingAsset(nullptr),m_sellingAsset(nullptr)
{

}

CreatePassiveSellOfferOperationResponse::~CreatePassiveSellOfferOperationResponse()
{
    if(m_buyingAsset)
        delete m_buyingAsset;
    if(m_sellingAsset)
        delete m_sellingAsset;
}

qint64 CreatePassiveSellOfferOperationResponse::getOfferId() const {
    return m_offerID;
}

QString CreatePassiveSellOfferOperationResponse::getAmount() const {
    return m_amount;
}

QString CreatePassiveSellOfferOperationResponse::getPrice() const{
    return m_price;
}

Asset* CreatePassiveSellOfferOperationResponse::getBuyingAsset() {
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

Asset* CreatePassiveSellOfferOperationResponse::getSellingAsset() {
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

QString CreatePassiveSellOfferOperationResponse::buyingAssetType() const
{
    return m_buyingAssetType;
}

QString CreatePassiveSellOfferOperationResponse::buyingAssetCode() const
{
    return m_buyingAssetCode;
}

QString CreatePassiveSellOfferOperationResponse::buyingAssetIssuer() const
{
    return m_buyingAssetIssuer;
}

QString CreatePassiveSellOfferOperationResponse::sellingAssetType() const
{
    return m_sellingAssetType;
}

QString CreatePassiveSellOfferOperationResponse::sellingAssetCode() const
{
    return m_sellingAssetCode;
}

QString CreatePassiveSellOfferOperationResponse::sellingAssetIssuer() const
{
    return m_sellingAssetIssuer;
}

void CreatePassiveSellOfferOperationResponse::setBuyingAssetType(QString buyingAssetType)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=nullptr;
    }
    m_buyingAssetType = buyingAssetType;
}

void CreatePassiveSellOfferOperationResponse::setBuyingAssetCode(QString buyingAssetCode)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=nullptr;
    }
    m_buyingAssetCode = buyingAssetCode;
}

void CreatePassiveSellOfferOperationResponse::setBuyingAssetIssuer(QString buyingAssetIssuer)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=nullptr;
    }
    m_buyingAssetIssuer = buyingAssetIssuer;
}

void CreatePassiveSellOfferOperationResponse::setSellingAssetType(QString sellingAssetType)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=nullptr;
    }
    m_sellingAssetType = sellingAssetType;
}

void CreatePassiveSellOfferOperationResponse::setSellingAssetCode(QString sellingAssetCode)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=nullptr;
    }
    m_sellingAssetCode = sellingAssetCode;
}

void CreatePassiveSellOfferOperationResponse::setSellingAssetIssuer(QString sellingAssetIssuer)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=nullptr;
    }
    m_sellingAssetIssuer = sellingAssetIssuer;
}
