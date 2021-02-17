#include "claimablebalanceresponse.h"


ClaimableBalanceResponse::ClaimableBalanceResponse(QNetworkReply *reply):Response(reply),m_asset(nullptr)
{

}

ClaimableBalanceResponse::~ClaimableBalanceResponse()
{
    if(m_asset)
        delete m_asset;
}

QString ClaimableBalanceResponse::getID() const
{
    return m_id;
}

Asset* ClaimableBalanceResponse::getAsset()
{
    if(!m_asset)
        m_asset = Asset::create(m_assetString);
    return m_asset;
}

QString ClaimableBalanceResponse::getAssetString() const
{
    return m_assetString;
}

QString ClaimableBalanceResponse::getAmount() const
{
    return m_amount;
}

QString ClaimableBalanceResponse::getSponsor() const
{
    return m_sponsor;
}

qint64 ClaimableBalanceResponse::getLastModifiedLedger() const
{
    return m_lastModifiedLedger;
}

qint64 ClaimableBalanceResponse::getLastModifiedTime() const
{
    return m_lastModifiedTime;
}

QString ClaimableBalanceResponse::getPagingToken() const
{
    return m_pagingToken;
}

ClaimableBalanceResponseAttach::Links ClaimableBalanceResponse::getLinks() const
{
    return m_links;
}

QList<Claimant> ClaimableBalanceResponse::getClaimants() const
{
    return m_claimants;
}

void ClaimableBalanceResponse::setId(QString id)
{
    if (m_id == id)
        return;

    m_id = id;
    emit idChanged();
}

void ClaimableBalanceResponse::setAsset(QString asset)
{
    if (m_assetString == asset)
        return;
    if(m_asset)
    {
        delete m_asset;
        m_asset=nullptr;
    }
    m_assetString = asset;
    emit assetChanged();
}

void ClaimableBalanceResponse::setAmount(QString amount)
{
    if (m_amount == amount)
        return;

    m_amount = amount;
    emit amountChanged();
}

void ClaimableBalanceResponse::setSponsor(QString sponsor)
{
    if (m_sponsor == sponsor)
        return;

    m_sponsor = sponsor;
    emit sponsorChanged();
}

void ClaimableBalanceResponse::setLastModifiedLedger(qint64 lastModifiedLedger)
{
    if (m_lastModifiedLedger == lastModifiedLedger)
        return;

    m_lastModifiedLedger = lastModifiedLedger;
    emit lastModifiedLedgerChanged();
}

void ClaimableBalanceResponse::setLastModifiedTime(qint64 lastModifiedTime)
{
    if (m_lastModifiedTime == lastModifiedTime)
        return;

    m_lastModifiedTime = lastModifiedTime;
    emit lastModifiedTimeChanged();
}

void ClaimableBalanceResponse::setPagingToken(QString pagingToken)
{
    if (m_pagingToken == pagingToken)
        return;

    m_pagingToken = pagingToken;
    emit pagingTokenChanged();
}

void ClaimableBalanceResponse::setLinks(ClaimableBalanceResponseAttach::Links links)
{
    if (m_links == links)
        return;

    m_links = links;
    emit linksChanged();
}

void ClaimableBalanceResponse::setClaimants(QList<Claimant> claimants)
{
    if (m_claimants == claimants)
        return;

    m_claimants = claimants;
    emit claimantsChanged();
}

Link ClaimableBalanceResponseAttach::Links::getSelf()
{
    return m_self;
}

bool ClaimableBalanceResponseAttach::Links::operator !=(ClaimableBalanceResponseAttach::Links &links)
{
    return m_self!=links.m_self;
}

bool ClaimableBalanceResponseAttach::Links::operator ==(ClaimableBalanceResponseAttach::Links &links)
{
    return m_self==links.m_self;
}
