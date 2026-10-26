#include "claimablebalanceresponse.h"
#include <QCoreApplication>
#include <QDateTime>
#include <stdexcept>
#include "qtcompat.h"

QSTELLAR_BEGIN_NS

static void registerTypes()
{
    regType<ClaimableBalanceResponseAttach::Links>();
    regType<ClaimableBalanceResponseAttach::Flags>();
}
Q_COREAPP_STARTUP_FUNCTION(registerTypes)



// The two times start at 0: left out, they read whatever the memory held.
ClaimableBalanceResponse::ClaimableBalanceResponse(QNetworkReply *reply):Response(reply),m_asset(nullptr)
  ,m_lastModifiedLedger(0),m_lastModifiedTime(0)
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

bool ClaimableBalanceResponse::readMember(const QString &key, const QJsonValue &value, const QString &field)
{
    // Horizon writes the time as ISO 8601 text ("2021-08-04T20:01:24Z"), or
    // null; into this integer property it never converted, and read as 0.
    if (key != QLatin1String("last_modified_time") || value.isNull())
        return false;
    const QDateTime time = value.isString() ? QDateTime::fromString(value.toString(), Qt::ISODate) : QDateTime();
    if (!time.isValid())
        throw std::runtime_error(QStringLiteral("%1: expected an ISO 8601 time").arg(field).toStdString());
    setLastModifiedTime(time.toSecsSinceEpoch());
    return true;
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
QSTELLAR_END_NS
