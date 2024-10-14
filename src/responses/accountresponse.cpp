#include "accountresponse.h"
#include "../keypair.h"
#include "../util.h"
#include <QCoreApplication>
static void registerTypes()
{
    qRegisterMetaType<AccountResponseAttach::Balance>();
    qRegisterMetaType<AccountResponseAttach::Signer>();
    QMetaType::registerConverter<QVariantList ,QList<AccountResponseAttach::Balance> >(&ResponseConverters::convert<AccountResponseAttach::Balance>);
    QMetaType::registerConverter<QVariantList ,QList<AccountResponseAttach::Signer> >(&ResponseConverters::convert<AccountResponseAttach::Signer>);
    qRegisterMetaType<AccountResponseAttach::Data>();
    QMetaType::registerConverter<QVariantMap ,AccountResponseAttach::Data>(&ResponseConverters::Account::convertData);
}

Q_COREAPP_STARTUP_FUNCTION(registerTypes)

AccountResponse::AccountResponse(QNetworkReply *reply)
    :Response(reply), m_keypair(nullptr), m_sequence(0) ,m_subentryCount(0),m_numSponsoring(0),m_numSponsored(0)
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

AccountResponseAttach::Data AccountResponse::getData() const{
    return m_data;
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

qint64 AccountResponse::getNumSponsoring() const
{
    return m_numSponsoring;
}

qint64 AccountResponse::getNumSponsored() const
{
    return m_numSponsored;
}

QString AccountResponse::getSponsor() const
{
    return m_sponsor;
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
Balance::Balance():m_assetIssuerKeypair(nullptr),m_asset(nullptr){
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

QString Balance::getAssetType() const{
    return m_assetType;
}

QString Balance::getAssetCode() const{
    return m_assetCode;
}

QString Balance::assetIssuer() const{
    return m_assetIssuer;
}

KeyPair &Balance::getAssetIssuer() {
    if(!m_assetIssuerKeypair)
        m_assetIssuerKeypair = KeyPair::fromAccountId(m_assetIssuer);
    return *m_assetIssuerKeypair;
}

QString Balance::getBalance() const{
    return m_balance;
}

QString Balance::getLimit() const{
    return m_limit;
}

QString Balance::getBuyingLiabilities() const
{
    return m_buyingLiabilities;
}

QString Balance::getSellingLiabilities() const
{
    return m_sellingLiabilities;
}

bool Balance::getAuthorized() const
{
    return m_isAuthorized;
}

bool Balance::getAuthorizedToMaintainLiabilities() const
{
    return m_isAuthorizedToMaintainLiabilities;
}

int Balance::getLastModifiedLedger() const
{
    return m_lastModifiedLedger;
}

bool Balance::operator !=(const Balance &b) const
{
    return (this->m_assetType!=b.m_assetType)
            || (this->m_assetCode!=b.m_assetCode)
            || (this->m_assetIssuer!=b.m_assetIssuer)
            || (this->m_limit!=b.m_limit)
            || (this->m_balance!=b.m_balance)
            || (this->m_buyingLiabilities!=b.m_buyingLiabilities)
            || (this->m_sellingLiabilities!=b.m_sellingLiabilities)
            || (this->m_isAuthorized!=b.m_isAuthorized)
            || (this->m_isAuthorizedToMaintainLiabilities!=b.m_isAuthorizedToMaintainLiabilities)
            || (this->m_lastModifiedLedger!=b.m_lastModifiedLedger)
            || (this->m_sponsor!=b.m_sponsor);
}

bool Balance::operator ==(const Balance &b) const
{
    return (this->m_assetType==b.m_assetType)
            && (this->m_assetCode==b.m_assetCode)
            && (this->m_assetIssuer==b.m_assetIssuer)
            && (this->m_limit==b.m_limit)
            && (this->m_balance==b.m_balance)
            && (this->m_buyingLiabilities==b.m_buyingLiabilities)
            && (this->m_sellingLiabilities==b.m_sellingLiabilities)
            && (this->m_isAuthorized==b.m_isAuthorized)
            && (this->m_isAuthorizedToMaintainLiabilities==b.m_isAuthorizedToMaintainLiabilities)
            && (this->m_lastModifiedLedger==b.m_lastModifiedLedger)
            && (this->m_sponsor==b.m_sponsor);
}

QString Balance::getSponsor() const
{
    return m_sponsor;
}



Signer::Signer():m_weight(0)
{

}

QString Signer::getAccountId() const{
    return m_accountId;
}

QString Signer::getKey() const
{
    return m_accountId;
}

QString Signer::getType() const
{
    return m_type;
}

int Signer::getWeight() const{
    return m_weight;
}

bool Signer::operator !=(const Signer &s) const
{
    return (this->m_accountId!=s.m_accountId)
                || (this->m_type!=s.m_type)
                || (this->m_weight!=s.m_weight)
                || (this->m_sponsor!=s.m_sponsor);
}

bool Signer::operator ==(const Signer &s) const
{
    return (this->m_accountId==s.m_accountId)
            && (this->m_type==s.m_type)
            && (this->m_weight==s.m_weight)
            && (this->m_sponsor==s.m_sponsor);
}

QString Signer::getSponsor() const
{
    return m_sponsor;
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

bool Links::operator !=(const Links &links) const
{
    return  (m_effects!= links.m_effects)
            || (m_offers!= links.m_offers)
            || (m_operations!= links.m_operations)
            || (m_self!= links.m_self)
            || (m_transactions!= links.m_transactions);
}

bool Links::operator ==(const Links &links) const
{
    return  (m_effects== links.m_effects)
            && (m_offers== links.m_offers)
            && (m_operations== links.m_operations)
            && (m_self== links.m_self)
            && (m_transactions== links.m_transactions);
}

Flags::Flags()
    :m_authRequired(false)
    ,m_authRevocable(false)
    ,m_authImmutable(false)
{

}

bool Flags::getAuthRequired() const{
    return m_authRequired;
}

bool Flags::getAuthRevocable() const{
    return m_authRevocable;
}

bool Flags::getAuthImmutable() const{
    return m_authImmutable;
}

bool Flags::operator !=(const Flags &f) const
{
    return (m_authRequired!=f.m_authRequired)
            || (m_authRevocable != f.m_authRevocable)
            || (m_authImmutable != f.m_authImmutable);
}

bool Flags::operator ==(const Flags &f) const
{
    return (m_authRequired==f.m_authRequired)
            && (m_authRevocable== f.m_authRevocable)
            && (m_authImmutable== f.m_authImmutable);
}

Thresholds::Thresholds()
    : m_lowThreshold(0)
    , m_medThreshold(0)
    , m_highThreshold(0)
{

}

int Thresholds::getLowThreshold() const{
    return m_lowThreshold;
}

int Thresholds::getMedThreshold() const {
    return m_medThreshold;
}

int Thresholds::getHighThreshold() const{
    return m_highThreshold;
}

bool Thresholds::operator !=(const Thresholds &t) const
{
    return (m_lowThreshold!=t.m_lowThreshold)
            || (m_medThreshold!=t.m_medThreshold)
            || (m_highThreshold!=t.m_highThreshold);
}

bool Thresholds::operator ==(const Thresholds &t) const
{
    return (m_lowThreshold==t.m_lowThreshold)
            && (m_medThreshold==t.m_medThreshold)
            && (m_highThreshold==t.m_highThreshold);
}

Data::Data(){}

Data::Data(const QVariantMap &data)
{
    for(QVariantMap::const_iterator it = data.begin(); it != data.end(); it++) {
        m_data.insert(it.key(),it.value().toByteArray());
    }

}

int Data::size() const
{
    return m_data.size();
}

QString Data::get(QString key) const{
    return QString::fromUtf8(m_data.value(key));
}

QByteArray Data::getDecoded(QString key) const{
    return QByteArray::fromBase64(m_data.value(key),XDR_BASE64ENCODING);
}

QByteArray Data::getRaw(QString key) const{
    return m_data.value(key);
}

bool Data::operator !=(const Data &data) const
{
    return (this->m_data!=data.m_data);
}

bool Data::operator ==(const Data &data) const
{
    return (this->m_data==data.m_data);
}

}

AccountResponseAttach::Data ResponseConverters::Account::convertData(const QVariantMap &source)
{
    return AccountResponseAttach::Data(source);
}
