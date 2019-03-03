#include "pathpaymentoperationresponse.h"
#include "../../keypair.h"
#include "../../asset.h"
#include "../../assettypenative.h"
PathPaymentOperationResponse::PathPaymentOperationResponse(QNetworkReply *reply)
    :OperationResponse(reply)
    ,m_fromKeypair(0),m_toKeypair(0),m_asset(0),m_sourceAsset(0)
{

}

PathPaymentOperationResponse::~PathPaymentOperationResponse()
{
    if(m_fromKeypair)
        delete m_fromKeypair;
    if(m_toKeypair)
        delete m_toKeypair;
    if(m_asset)
        delete m_asset;
    if(m_sourceAsset)
        delete m_sourceAsset;
}

QString PathPaymentOperationResponse::getAmount() const {
    return m_amount;
}

QString PathPaymentOperationResponse::getSourceAmount() const
{
    return m_sourceAmount;
}

QString PathPaymentOperationResponse::getSourceMax() const{
    return m_sourceMax;
}

KeyPair &PathPaymentOperationResponse::getFrom() {
    if(!m_fromKeypair)
        m_fromKeypair = KeyPair::fromAccountId(m_from);
    return *m_fromKeypair;
}

KeyPair &PathPaymentOperationResponse::getTo() {
    if(!m_toKeypair)
        m_toKeypair = KeyPair::fromAccountId(m_to);
    return *m_toKeypair;
}

Asset* PathPaymentOperationResponse::getAsset() {
    if(m_asset)
        return m_asset;
    if (m_assetType =="native") {
        m_asset= new AssetTypeNative();
    } else {
        KeyPair* issuer = KeyPair::fromAccountId(m_assetIssuer);
        m_asset =Asset::createNonNativeAsset(m_assetCode, issuer);
    }
    return m_asset;
}

Asset* PathPaymentOperationResponse::getSourceAsset() {
    if(m_sourceAsset)
        return m_sourceAsset;
    if (m_sourceAssetType =="native") {
        m_sourceAsset= new AssetTypeNative();
    } else {
        KeyPair* issuer = KeyPair::fromAccountId(m_sourceAssetIssuer);
        m_sourceAsset =Asset::createNonNativeAsset(m_sourceAssetCode, issuer);
    }
    return m_sourceAsset;
}

QString PathPaymentOperationResponse::from() const
{
    return m_from;
}

QString PathPaymentOperationResponse::to() const
{
    return m_to;
}

QString PathPaymentOperationResponse::assetType() const
{
    return m_assetType;
}

QString PathPaymentOperationResponse::assetCode() const
{
    return m_assetCode;
}

QString PathPaymentOperationResponse::assetIssuer() const
{
    return m_assetIssuer;
}

QString PathPaymentOperationResponse::sourceAssetType() const
{
    return m_sourceAssetType;
}

QString PathPaymentOperationResponse::sourceAssetCode() const
{
    return m_sourceAssetCode;
}

QString PathPaymentOperationResponse::sourceAssetIssuer() const
{
    return m_sourceAssetIssuer;
}

void PathPaymentOperationResponse::setFrom(QString from)
{
    if(m_fromKeypair){
        delete m_fromKeypair;
        m_fromKeypair=0;
    }
    m_from = from;
}

void PathPaymentOperationResponse::setTo(QString to)
{
    if(m_toKeypair){
        delete m_toKeypair;
        m_toKeypair=0;
    }
    m_to = to;
}

void PathPaymentOperationResponse::setAssetType(QString assetType)
{
    if(m_asset){
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetType = assetType;
}

void PathPaymentOperationResponse::setAssetCode(QString assetCode)
{
    if(m_asset){
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetCode = assetCode;
}

void PathPaymentOperationResponse::setAssetIssuer(QString assetIssuer)
{
    if(m_asset){
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetIssuer = assetIssuer;
}

void PathPaymentOperationResponse::setSourceAssetType(QString sourceAssetType)
{
    if(m_sourceAsset){
        delete m_sourceAsset;
        m_sourceAsset=nullptr;
    }
    m_sourceAssetType = sourceAssetType;
}

void PathPaymentOperationResponse::setSourceAssetCode(QString sourceAssetCode)
{
    if(m_sourceAsset){
        delete m_sourceAsset;
        m_sourceAsset=nullptr;
    }
    m_sourceAssetCode = sourceAssetCode;
}

void PathPaymentOperationResponse::setSourceAssetIssuer(QString sourceAssetIssuer)
{
    if(m_sourceAsset){
        delete m_sourceAsset;
        m_sourceAsset=nullptr;
    }
    m_sourceAssetIssuer = sourceAssetIssuer;
}
