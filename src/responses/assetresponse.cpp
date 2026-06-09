#include "assetresponse.h"

QSTELLAR_BEGIN_NS


AssetResponse::AssetResponse(QNetworkReply *reply):Response(reply),m_asset(nullptr),m_numAccounts(0),m_numClaimableBalances(0)
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

AssetResponseAttach::Accounts AssetResponse::getAccounts() const{
    return m_accounts;
}

AssetResponseAttach::Balances AssetResponse::getBalances() const{
    return m_balances;
}

QString AssetResponse::getClaimableBalancesAmount() const{
    return m_claimableBalancesAmount;
}

int AssetResponse::getNumClaimableBalances() {
    return m_numClaimableBalances;
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

bool AssetResponseAttach::Flags::operator ==(const AssetResponseAttach::Flags &t) const
{
    return m_authRequired == t.m_authRequired
            && m_authRevocable == t.m_authRevocable;
}

bool AssetResponseAttach::Flags::operator !=(const AssetResponseAttach::Flags &t) const
{
    return !(*this == t);
}

AssetResponseAttach::Links::Links() {
}

Link AssetResponseAttach::Links::getToml() {
    return m_toml;
}
bool AssetResponseAttach::Links::operator ==(const AssetResponseAttach::Links& t) const
{
    return m_toml == t.m_toml;
}

bool AssetResponseAttach::Links::operator !=(const AssetResponseAttach::Links& t) const
{
    return !(*this == t);
}

AssetResponseAttach::Accounts::Accounts():m_authorized(0),m_authorizedToMaintainLiabilities(0),m_unauthorized(0) {

}

int AssetResponseAttach::Accounts::authorized() {
    return m_authorized;
}

int AssetResponseAttach::Accounts::authorizedToMaintainLiabilities() {
    return m_authorizedToMaintainLiabilities;
}

int AssetResponseAttach::Accounts::unauthorized() {
    return m_unauthorized;
}

bool AssetResponseAttach::Accounts::operator ==(const Accounts &t) const
{
    return m_authorized == t.m_authorized
            && m_authorizedToMaintainLiabilities == t.m_authorizedToMaintainLiabilities
            && m_unauthorized == t.m_unauthorized;
}

bool AssetResponseAttach::Accounts::operator !=(const Accounts &t) const
{
    return !(*this == t);
}

AssetResponseAttach::Balances::Balances() {

}

QString AssetResponseAttach::Balances::authorized() {
    return m_authorized;
}

QString AssetResponseAttach::Balances::authorizedToMaintainLiabilities() {
    return m_authorizedToMaintainLiabilities;
}

QString AssetResponseAttach::Balances::unauthorized() {
    return m_unauthorized;
}

bool AssetResponseAttach::Balances::operator ==(const Balances &t) const
{
    return m_authorized == t.m_authorized
            && m_authorizedToMaintainLiabilities == t.m_authorizedToMaintainLiabilities
            && m_unauthorized == t.m_unauthorized;
}

bool AssetResponseAttach::Balances::operator !=(const Balances &t) const
{
    return !(*this == t);
}
QSTELLAR_END_NS
