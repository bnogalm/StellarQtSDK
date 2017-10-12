#include "pathpaymentoperationresponse.h"
#include "../../keypair.h"
#include "../../asset.h"
#include "../../assettypenative.h"
PathPaymentOperationResponse::PathPaymentOperationResponse(QNetworkReply *reply)
    :OperationResponse(reply)
    ,m_fromKeypair(0),m_toKeypair(0),m_asset(0),m_sendAsset(0)
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
    if(m_sendAsset)
        delete m_sendAsset;
}

QString PathPaymentOperationResponse::getAmount() {
    return m_amount;
}

QString PathPaymentOperationResponse::getSourceAmount() {
    return m_sourceAmount;
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

Asset* PathPaymentOperationResponse::getSendAsset() {
    if(m_sendAsset)
        return m_sendAsset;
    if (m_sendAssetType =="native") {
        m_sendAsset= new AssetTypeNative();
    } else {
        KeyPair* issuer = KeyPair::fromAccountId(m_sendAssetIssuer);
        m_sendAsset =Asset::createNonNativeAsset(m_sendAssetCode, issuer);
    }
    return m_sendAsset;
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

QString PathPaymentOperationResponse::sendAssetType() const
{
    return m_sendAssetType;
}

QString PathPaymentOperationResponse::sendAssetCode() const
{
    return m_sendAssetCode;
}

QString PathPaymentOperationResponse::sendAssetIssuer() const
{
    return m_sendAssetIssuer;
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
        m_asset=0;
    }
    m_assetType = assetType;
}

void PathPaymentOperationResponse::setAssetCode(QString assetCode)
{
    if(m_asset){
        delete m_asset;
        m_asset=0;
    }
    m_assetCode = assetCode;
}

void PathPaymentOperationResponse::setAssetIssuer(QString assetIssuer)
{
    if(m_asset){
        delete m_asset;
        m_asset=0;
    }
    m_assetIssuer = assetIssuer;
}

void PathPaymentOperationResponse::setSendAssetType(QString sendAssetType)
{
    if(m_sendAsset){
        delete m_sendAsset;
        m_sendAsset=0;
    }
    m_sendAssetType = sendAssetType;
}

void PathPaymentOperationResponse::setSendAssetCode(QString sendAssetCode)
{
    if(m_sendAsset){
        delete m_sendAsset;
        m_sendAsset=0;
    }
    m_sendAssetCode = sendAssetCode;
}

void PathPaymentOperationResponse::setSendAssetIssuer(QString sendAssetIssuer)
{
    if(m_sendAsset){
        delete m_sendAsset;
        m_sendAsset=0;
    }
    m_sendAssetIssuer = sendAssetIssuer;
}
