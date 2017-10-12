#include "paymentoperationresponse.h"
#include "../../asset.h"
#include "../../assettypenative.h"
#include "../../keypair.h"

PaymentOperationResponse::PaymentOperationResponse(QNetworkReply *reply)
    :OperationResponse(reply),m_fromKeypair(0),m_toKeypair(0),m_asset(0)
{

}

PaymentOperationResponse::~PaymentOperationResponse()
{
    if(m_fromKeypair)
        delete m_fromKeypair;
    if(m_toKeypair)
        delete m_toKeypair;
    if(m_asset)
        delete m_asset;
}

QString PaymentOperationResponse::getAmount() {
    return m_amount;
}

QString PaymentOperationResponse::assetType()
{
    return m_assetType;
}

QString PaymentOperationResponse::assetCode()
{
    return m_assetCode;
}

Asset* PaymentOperationResponse::getAsset() {
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

KeyPair &PaymentOperationResponse::getFrom() {
    if(!m_fromKeypair)
        m_fromKeypair = KeyPair::fromAccountId(m_from);
    return *m_fromKeypair;
}

KeyPair &PaymentOperationResponse::getTo() {
    if(!m_toKeypair)
        m_toKeypair = KeyPair::fromAccountId(m_to);
    return *m_toKeypair;
}

QString PaymentOperationResponse::from() const
{
    return m_from;
}

QString PaymentOperationResponse::to() const
{
    return m_to;
}

QString PaymentOperationResponse::assetIssuer() const
{
    return m_assetIssuer;
}

void PaymentOperationResponse::setFrom(QString from)
{
    if(m_fromKeypair){
        delete m_fromKeypair;
        m_fromKeypair=0;
    }
    m_from = from;
}

void PaymentOperationResponse::setTo(QString to)
{
    if(m_toKeypair){
        delete m_toKeypair;
        m_toKeypair=0;
    }
    m_to = to;
}

void PaymentOperationResponse::setAssetType(QString assetType)
{
    if(m_asset){
        delete m_asset;
        m_asset=0;
    }
    m_assetType = assetType;
}

void PaymentOperationResponse::setAssetCode(QString assetCode)
{
    if(m_asset){
        delete m_asset;
        m_asset=0;
    }
    m_assetCode = assetCode;
}

void PaymentOperationResponse::setAssetIssuer(QString assetIssuer)
{
    if(m_asset){
        delete m_asset;
        m_asset=0;
    }
    m_assetIssuer = assetIssuer;
}
