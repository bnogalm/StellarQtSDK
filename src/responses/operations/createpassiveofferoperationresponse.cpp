#include "createpassiveofferoperationresponse.h"
#include "../../asset.h"
#include "../../keypair.h"
#include "../../asset.h"
#include "../../assettypenative.h"

CreatePassiveOfferOperationResponse::CreatePassiveOfferOperationResponse(QNetworkReply *reply)
    :OperationResponse(reply),m_offerID(0),m_buyingAsset(0),m_sellingAsset(0)
{

}

CreatePassiveOfferOperationResponse::~CreatePassiveOfferOperationResponse()
{
    if(m_buyingAsset)
        delete m_buyingAsset;
    if(m_sellingAsset)
        delete m_sellingAsset;
}

qint32 CreatePassiveOfferOperationResponse::getOfferId() const {
    return m_offerID;
}

QString CreatePassiveOfferOperationResponse::getAmount() const {
    return m_amount;
}

QString CreatePassiveOfferOperationResponse::getPrice() const{
    return m_price;
}

Asset* CreatePassiveOfferOperationResponse::getBuyingAsset() {
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

Asset* CreatePassiveOfferOperationResponse::getSellingAsset() {
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

QString CreatePassiveOfferOperationResponse::buyingAssetType() const
{
    return m_buyingAssetType;
}

QString CreatePassiveOfferOperationResponse::buyingAssetCode() const
{
    return m_buyingAssetCode;
}

QString CreatePassiveOfferOperationResponse::buyingAssetIssuer() const
{
    return m_buyingAssetIssuer;
}

QString CreatePassiveOfferOperationResponse::sellingAssetType() const
{
    return m_sellingAssetType;
}

QString CreatePassiveOfferOperationResponse::sellingAssetCode() const
{
    return m_sellingAssetCode;
}

QString CreatePassiveOfferOperationResponse::sellingAssetIssuer() const
{
    return m_sellingAssetIssuer;
}

void CreatePassiveOfferOperationResponse::setBuyingAssetType(QString buyingAssetType)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=0;
    }
    m_buyingAssetType = buyingAssetType;
}

void CreatePassiveOfferOperationResponse::setBuyingAssetCode(QString buyingAssetCode)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=0;
    }
    m_buyingAssetCode = buyingAssetCode;
}

void CreatePassiveOfferOperationResponse::setBuyingAssetIssuer(QString buyingAssetIssuer)
{
    if(m_buyingAsset){
        delete m_buyingAsset;
        m_buyingAsset=0;
    }
    m_buyingAssetIssuer = buyingAssetIssuer;
}

void CreatePassiveOfferOperationResponse::setSellingAssetType(QString sellingAssetType)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=0;
    }
    m_sellingAssetType = sellingAssetType;
}

void CreatePassiveOfferOperationResponse::setSellingAssetCode(QString sellingAssetCode)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=0;
    }
    m_sellingAssetCode = sellingAssetCode;
}

void CreatePassiveOfferOperationResponse::setSellingAssetIssuer(QString sellingAssetIssuer)
{
    if(m_sellingAsset){
        delete m_sellingAsset;
        m_sellingAsset=0;
    }
    m_sellingAssetIssuer = sellingAssetIssuer;
}
