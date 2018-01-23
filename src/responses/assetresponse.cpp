#include "assetresponse.h"

AssetResponse::AssetResponse(QNetworkReply *reply):Response(reply),m_asset(nullptr),m_numAccounts(0)
{

}

AssetResponse::~AssetResponse()
{
    if(m_asset)
        delete m_asset;
}

QString AssetResponse::getAssetType() const{
    return m_assetType;
}

QString AssetResponse::getAssetCode() const{
    return m_assetCode;
}

QString AssetResponse::getAssetIssuer() const{
    return m_assetIssuer;
}

Asset *AssetResponse::getAsset() {
    if(!m_asset)
    {
        m_asset = Asset::create(m_assetType,m_assetCode,m_assetIssuer);
    }
    return m_asset;
}

QString AssetResponse::getPagingToken() const{
    return m_pagingToken;
}

QString AssetResponse::getAmount() const{
    return m_amount;
}

int AssetResponse::getNumAccounts() const{
    return m_numAccounts;
}

AssetResponseAttach::Flags AssetResponse::getFlags() const{
    return m_flags;
}

AssetResponseAttach::Links AssetResponse::getLinks() const{
    return m_links;
}

void AssetResponse::setAssetType(QString asset_type)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetType = asset_type;
}

void AssetResponse::setAssetCode(QString asset_code)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetCode = asset_code;
}

void AssetResponse::setAssetIssuer(QString asset_issuer)
{
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetIssuer = asset_issuer;
}

AssetResponseAttach::Flags::Flags():m_authRequired(false),m_authRevocable(false){
}

bool AssetResponseAttach::Flags::isAuthRequired(){
    return m_authRequired;
}

bool AssetResponseAttach::Flags::isAuthRevocable(){
    return m_authRevocable;
}

bool AssetResponseAttach::Flags::operator !=(AssetResponseAttach::Flags &t)
{
    Q_UNUSED(t)
    return true;
}

AssetResponseAttach::Links::Links() {
}

Link AssetResponseAttach::Links::getToml() {
    return m_toml;
}
bool AssetResponseAttach::Links::operator !=(AssetResponseAttach::Links& t)
{
    Q_UNUSED(t)
    return true;
}
