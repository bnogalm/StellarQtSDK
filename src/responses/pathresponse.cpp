#include "pathresponse.h"
#include "../asset.h"
#include "../keypair.h"
#include "../assettypenative.h"
#include "../assettypecreditalphanum.h"
#include "common.h"

PathResponse::PathResponse(QNetworkReply *reply )
    : Response(reply),m_destinationAsset(nullptr),m_sourceAsset(nullptr)
{

}

PathResponse::~PathResponse()
{
    if(m_destinationAsset)
        delete m_destinationAsset;
    if(m_sourceAsset)
        delete m_sourceAsset;

    for(Asset * a :m_pathConverted)
    {
        delete a;
    }
}

QString PathResponse::getDestinationAmount() {
    return m_destinationAmount;
}

QString PathResponse::getSourceAmount() {
    return m_sourceAmount;
}

const QList<Asset *>& PathResponse::getPath() {
    if(m_pathConverted.empty()){
        for(auto & a : m_path)
        {
            const QVariantMap m = a.toMap();
            m_pathConverted.append(assetFromVariantMap(m));
        }
    }
    return m_pathConverted;
}

Asset &PathResponse::getDestinationAsset() {
    if(!m_destinationAsset){
        if (m_destinationAssetType == "native") {
            m_destinationAsset= new AssetTypeNative();
        } else {
            KeyPair* issuer = KeyPair::fromAccountId(m_destinationAssetIssuer);
            m_destinationAsset= Asset::createNonNativeAsset(m_destinationAssetCode, issuer);
        }
    }
    return *m_destinationAsset;
}

Asset& PathResponse::getSourceAsset() {
    if(!m_sourceAsset){
        if (m_sourceAssetType == "native") {
            m_sourceAsset= new AssetTypeNative();
        } else {
            KeyPair* issuer = KeyPair::fromAccountId(m_sourceAssetIssuer);
            m_sourceAsset= Asset::createNonNativeAsset(m_sourceAssetCode, issuer);
        }
    }
    return *m_sourceAsset;
}

PathResponseAttach::Links &PathResponse::getLinks() {
    return m_links;
}

QString PathResponse::destinationAssetType() const
{
    return m_destinationAssetType;
}

QString PathResponse::destinationAssetCode() const
{
    return m_destinationAssetCode;
}

QString PathResponse::destinationAssetIssuer() const
{
    return m_destinationAssetIssuer;
}

QString PathResponse::sourceAssetType() const
{
    return m_sourceAssetType;
}

QString PathResponse::sourceAssetCode() const
{
    return m_sourceAssetCode;
}

QString PathResponse::sourceAssetIssuer() const
{
    return m_sourceAssetIssuer;
}

void PathResponse::setDestinationAssetType(QString destinationAssetType)
{
    if(m_destinationAsset)
    {
        delete m_destinationAsset;
        m_destinationAsset=nullptr;
    }
    m_destinationAssetType = destinationAssetType;
}

void PathResponse::setDestinationAssetCode(QString destinationAssetCode)
{
    if(m_destinationAsset)
    {
        delete m_destinationAsset;
        m_destinationAsset=nullptr;
    }
    m_destinationAssetCode = destinationAssetCode;
}

void PathResponse::setDestinationAssetIssuer(QString destinationAssetIssuer)
{
    if(m_destinationAsset)
    {
        delete m_destinationAsset;
        m_destinationAsset=nullptr;
    }
    m_destinationAssetIssuer = destinationAssetIssuer;
}

void PathResponse::setSourceAssetType(QString sourceAssetType)
{
    if(m_sourceAsset)
    {
        delete m_sourceAsset;
        m_sourceAsset=nullptr;
    }
    m_sourceAssetType = sourceAssetType;
}

void PathResponse::setSourceAssetCode(QString sourceAssetCode)
{
    if(m_sourceAsset)
    {
        delete m_sourceAsset;
        m_sourceAsset=nullptr;
    }
    m_sourceAssetCode = sourceAssetCode;
}

void PathResponse::setSourceAssetIssuer(QString sourceAssetIssuer)
{
    if(m_sourceAsset)
    {
        delete m_sourceAsset;
        m_sourceAsset=nullptr;
    }
    m_sourceAssetIssuer = sourceAssetIssuer;
}

void PathResponse::setPath(QVariantList path)
{
    for(Asset * a :m_pathConverted)
    {
        delete a;
    }
    m_pathConverted.clear();
    m_path = path;
}
