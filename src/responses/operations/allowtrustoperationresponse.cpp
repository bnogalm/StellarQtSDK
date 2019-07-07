#include "allowtrustoperationresponse.h"
#include "../../keypair.h"
#include "../../asset.h"
#include "../../assettypenative.h"
AllowTrustOperationResponse::AllowTrustOperationResponse(QNetworkReply *reply)
    :OperationResponse(reply),m_trustorKeypair(nullptr),m_trusteeKeypair(nullptr),m_asset(nullptr),m_authorize(false)
{

}

AllowTrustOperationResponse::~AllowTrustOperationResponse(){}

KeyPair& AllowTrustOperationResponse::getTrustor() {
    if(!m_trustorKeypair)
        m_trustorKeypair = KeyPair::fromAccountId(m_trustor);
    return *m_trustorKeypair;
}

KeyPair& AllowTrustOperationResponse::getTrustee() {
    if(!m_trusteeKeypair)
        m_trusteeKeypair = KeyPair::fromAccountId(m_trustee);
    return *m_trusteeKeypair;
}

Asset* AllowTrustOperationResponse::getAsset() {
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

QString AllowTrustOperationResponse::trustor() const
{
    return m_trustor;
}

QString AllowTrustOperationResponse::trustee() const
{
    return m_trustee;
}

QString AllowTrustOperationResponse::assetType() const
{
    return m_assetType;
}

QString AllowTrustOperationResponse::assetCode() const
{
    return m_assetCode;
}

QString AllowTrustOperationResponse::assetIssuer() const
{
    return m_assetIssuer;
}

bool AllowTrustOperationResponse::isAuthorize() const
{
    return m_authorize;
}

void AllowTrustOperationResponse::setTrustor(QString trustor)
{
    if(m_trustorKeypair){
        delete m_trustorKeypair;
        m_trustorKeypair=nullptr;
    }
    m_trustor = trustor;
}

void AllowTrustOperationResponse::setTrustee(QString trustee)
{
    if(m_trusteeKeypair){
        delete m_trusteeKeypair;
        m_trusteeKeypair=nullptr;
    }
    m_trustee = trustee;
}

void AllowTrustOperationResponse::setAssetType(QString assetType)
{
    if(m_asset){
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetType = assetType;
}

void AllowTrustOperationResponse::setAssetCode(QString assetCode)
{
    if(m_asset){
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetCode = assetCode;
}

void AllowTrustOperationResponse::setAssetIssuer(QString assetIssuer)
{
    if(m_asset){
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetIssuer = assetIssuer;
}
