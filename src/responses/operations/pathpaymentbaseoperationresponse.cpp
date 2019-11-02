#include "pathpaymentbaseoperationresponse.h"


#include "../../keypair.h"
#include "../../asset.h"
#include "../../assettypenative.h"

#include "../common.h"


PathPaymentBaseOperationResponse::PathPaymentBaseOperationResponse(QNetworkReply *reply)
    :OperationResponse(reply)
    ,m_fromKeypair(nullptr),m_toKeypair(nullptr),m_asset(nullptr),m_sourceAsset(nullptr)
{

}

PathPaymentBaseOperationResponse::~PathPaymentBaseOperationResponse()
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

QString PathPaymentBaseOperationResponse::getAmount() const {
    return m_amount;
}

QString PathPaymentBaseOperationResponse::getSourceAmount() const
{
    return m_sourceAmount;
}

KeyPair &PathPaymentBaseOperationResponse::getFrom() {
    if(!m_fromKeypair)
        m_fromKeypair = KeyPair::fromAccountId(m_from);
    return *m_fromKeypair;
}

KeyPair &PathPaymentBaseOperationResponse::getTo() {
    if(!m_toKeypair)
        m_toKeypair = KeyPair::fromAccountId(m_to);
    return *m_toKeypair;
}

Asset* PathPaymentBaseOperationResponse::getAsset() {
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

Asset* PathPaymentBaseOperationResponse::getSourceAsset() {
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

QString PathPaymentBaseOperationResponse::from() const
{
    return m_from;
}

QString PathPaymentBaseOperationResponse::to() const
{
    return m_to;
}

QString PathPaymentBaseOperationResponse::assetType() const
{
    return m_assetType;
}

QString PathPaymentBaseOperationResponse::assetCode() const
{
    return m_assetCode;
}

QString PathPaymentBaseOperationResponse::assetIssuer() const
{
    return m_assetIssuer;
}

QString PathPaymentBaseOperationResponse::sourceAssetType() const
{
    return m_sourceAssetType;
}

QString PathPaymentBaseOperationResponse::sourceAssetCode() const
{
    return m_sourceAssetCode;
}

QString PathPaymentBaseOperationResponse::sourceAssetIssuer() const
{
    return m_sourceAssetIssuer;
}

QVariantList PathPaymentBaseOperationResponse::path() const
{
    return m_path;
}

const QList<Asset *>& PathPaymentBaseOperationResponse::getPath()
{
    if(m_pathConverted.empty()){
        for(const auto & a : m_path)
        {
            const QVariantMap m = a.toMap();
            m_pathConverted.append(assetFromVariantMap(m));
        }
    }
    return m_pathConverted;
}

void PathPaymentBaseOperationResponse::setFrom(QString from)
{
    if(m_fromKeypair){
        delete m_fromKeypair;
        m_fromKeypair=nullptr;
    }
    m_from = from;
}

void PathPaymentBaseOperationResponse::setTo(QString to)
{
    if(m_toKeypair){
        delete m_toKeypair;
        m_toKeypair=nullptr;
    }
    m_to = to;
}

void PathPaymentBaseOperationResponse::setAssetType(QString assetType)
{
    if(m_asset){
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetType = assetType;
}

void PathPaymentBaseOperationResponse::setAssetCode(QString assetCode)
{
    if(m_asset){
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetCode = assetCode;
}

void PathPaymentBaseOperationResponse::setAssetIssuer(QString assetIssuer)
{
    if(m_asset){
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetIssuer = assetIssuer;
}

void PathPaymentBaseOperationResponse::setSourceAssetType(QString sourceAssetType)
{
    if(m_sourceAsset){
        delete m_sourceAsset;
        m_sourceAsset=nullptr;
    }
    m_sourceAssetType = sourceAssetType;
}

void PathPaymentBaseOperationResponse::setSourceAssetCode(QString sourceAssetCode)
{
    if(m_sourceAsset){
        delete m_sourceAsset;
        m_sourceAsset=nullptr;
    }
    m_sourceAssetCode = sourceAssetCode;
}

void PathPaymentBaseOperationResponse::setSourceAssetIssuer(QString sourceAssetIssuer)
{
    if(m_sourceAsset){
        delete m_sourceAsset;
        m_sourceAsset=nullptr;
    }
    m_sourceAssetIssuer = sourceAssetIssuer;
}

void PathPaymentBaseOperationResponse::setPath(QVariantList path)
{
    for(Asset * a :m_pathConverted)
    {
        delete a;
    }
    m_pathConverted.clear();
    m_path = path;
}
