#include "accountresponse.h"
#include "../keypair.h"

#include <QCoreApplication>
static void registerTypes()
{
    qRegisterMetaType<AccountResponseAttach::Balance>();
    qRegisterMetaType<AccountResponseAttach::Signer>();
    QMetaType::registerConverter<QVariantList ,QList<AccountResponseAttach::Balance> >(&convert<AccountResponseAttach::Balance>);
    QMetaType::registerConverter<QVariantList ,QList<AccountResponseAttach::Signer> >(&convert<AccountResponseAttach::Signer>);
}

Q_COREAPP_STARTUP_FUNCTION(registerTypes)

AccountResponse::AccountResponse(QNetworkReply *reply)
    :Response(reply), m_keypair(0), m_sequence(0) ,m_subentryCount(0)
{

}

AccountResponse::~AccountResponse()
{
    if(m_keypair){
        delete m_keypair;
    }
}

KeyPair *AccountResponse::getKeypair()
{
    if(!m_keypair)
    {
        m_keypair = KeyPair::fromAccountId(m_account_id);
    }
    return this->m_keypair;
}


QString AccountResponse::accountID() const
{
    return m_account_id;
}

AccountResponseAttach::Links AccountResponse::getLinks() const {
    return m_links;
}

qint64 AccountResponse::getSequenceNumber() const{
    return m_sequence;
}

qint64 AccountResponse::getIncrementedSequenceNumber() const{
    return m_sequence + 1;
}

void AccountResponse::incrementSequenceNumber(){
    m_sequence++;
}

QString AccountResponse::getPagingToken() const{
    return m_pagingToken;
}

qint64 AccountResponse::getSubentryCount() const{
    return m_subentryCount;
}

QString AccountResponse::getInflationDestination() const{
    return m_inflationDestination;
}

QString AccountResponse::getHomeDomain() const{
    return m_homeDomain;
}

AccountResponseAttach::Thresholds AccountResponse::getThresholds() const{
    return m_thresholds;
}

AccountResponseAttach::Flags AccountResponse::getFlags() const {
    return m_flags;
}

QList<AccountResponseAttach::Balance> AccountResponse::getBalances() const{
    return m_balances;
}

QList<AccountResponseAttach::Signer> AccountResponse::getSigners() const{
    return m_signers;
}

void AccountResponse::setAccountID(QString account_id)
{
    if(m_keypair)
    {
        delete m_keypair;
        m_keypair = nullptr;
    }
    m_account_id = account_id;
}

namespace AccountResponseAttach
{
Balance::Balance():m_assetIssuerKeypair(0),m_asset(0){
}

Balance::~Balance()
{
    if(m_assetIssuerKeypair)
        delete m_assetIssuerKeypair;
}

Asset *Balance::getAsset() {
    if(!m_asset){
        if (m_assetType =="native") {
            m_asset= new AssetTypeNative();
        } else {
            m_asset= Asset::createNonNativeAsset(m_assetCode, getAssetIssuer());
        }
    }
    return m_asset;
}

QString Balance::getAssetType() {
    return m_assetType;
}

QString Balance::getAssetCode() {
    return m_assetCode;
}

KeyPair &Balance::getAssetIssuer() {
    if(!m_assetIssuerKeypair)
        m_assetIssuerKeypair = KeyPair::fromAccountId(m_assetIssuer);
    return *m_assetIssuerKeypair;
}

QString Balance::getBalance() {
    return m_balance;
}

QString Balance::getLimit() {
    return m_limit;
}

bool Balance::operator !=(Balance &b)
{
    Q_UNUSED(b)
    return true;
}

bool Balance::operator ==(Balance &b)
{
    return (this->m_assetType==b.m_assetType)
            && (this->m_assetCode==b.m_assetCode)
            && (this->m_assetIssuer==b.m_assetIssuer)
            && (this->m_limit==b.m_limit)
            && (this->m_balance==b.m_balance);
}

Signer::Signer():m_weight(0)
{

}

QString Signer::getAccountId() {
    return m_accountId;
}

int Signer::getWeight() {
    return m_weight;
}

bool Signer::operator !=(Signer &s)
{
    Q_UNUSED(s)
    return true;
}

bool Signer::operator ==(Signer &s)
{
    return (this->m_accountId==s.m_accountId)
            && (this->m_weight==s.m_weight);
}

Link Links::getEffects() {
    return m_effects;
}

Link Links::getOffers() {
    return m_offers;
}

Link Links::getOperations() {
    return m_operations;
}

Link AccountResponseAttach::Links::getSelf() {
    return m_self;
}

Link AccountResponseAttach::Links::getTransactions() {
    return m_transactions;
}

bool Links::operator !=(Links &links)
{
    Q_UNUSED(links)
    return true;
}

Flags::Flags()
    :m_authRequired(false)
    ,m_authRevocable(false)
{

}

bool Flags::getAuthRequired() {
    return m_authRequired;
}

bool Flags::getAuthRevocable() {
    return m_authRevocable;
}

bool Flags::operator !=(Flags &f)
{
    Q_UNUSED(f)
    return true;
}

Thresholds::Thresholds()
    : m_lowThreshold(0)
    , m_medThreshold(0)
    , m_highThreshold(0)
{

}

int Thresholds::getLowThreshold() {
    return m_lowThreshold;
}

int Thresholds::getMedThreshold() {
    return m_medThreshold;
}

int Thresholds::getHighThreshold() {
    return m_highThreshold;
}

bool Thresholds::operator !=(Thresholds &t)
{
    Q_UNUSED(t)
    return true;
}

}

