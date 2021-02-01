#include "claimablebalanceresponse.h"

ClaimableBalanceResponse::ClaimableBalanceResponse(QNetworkReply *reply):Response(reply)
{

}

QString ClaimableBalanceResponse::id() const
{
    return m_id;
}

QString ClaimableBalanceResponse::getAsset() const
{
    return m_asset;
}

QString ClaimableBalanceResponse::getAmount() const
{
    return m_amount;
}

QString ClaimableBalanceResponse::getSponsor() const
{
    return m_sponsor;
}

qint64 ClaimableBalanceResponse::getLastModified_ledget() const
{
    return m_last_modified_ledget;
}

qint64 ClaimableBalanceResponse::getLastModifiedTime() const
{
    return m_last_modified_time;
}

QString ClaimableBalanceResponse::getPagingToken() const
{
    return m_paging_token;
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
    if (m_asset == asset)
        return;

    m_asset = asset;
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

void ClaimableBalanceResponse::setLastModifiedLedget(qint64 last_modified_ledget)
{
    if (m_last_modified_ledget == last_modified_ledget)
        return;

    m_last_modified_ledget = last_modified_ledget;
    emit lastModifiedLedgetChanged();
}

void ClaimableBalanceResponse::setLastModifiedTime(qint64 last_modified_time)
{
    if (m_last_modified_time == last_modified_time)
        return;

    m_last_modified_time = last_modified_time;
    emit lastModifiedTimeChanged();
}

void ClaimableBalanceResponse::setPagingToken(QString paging_token)
{
    if (m_paging_token == paging_token)
        return;

    m_paging_token = paging_token;
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
