#ifndef CLAIMABLEBALANCERESPONSE_H
#define CLAIMABLEBALANCERESPONSE_H

#include "response.h"
#include <QObject>
#include "link.h"
#include "claimant.h"
#include "asset.h"
namespace ClaimableBalanceResponseAttach
{
/**
 * Links connected to ClaimableBalanceResponse.
 */
class Links {
    Q_GADGET
    Q_PROPERTY(Link self MEMBER m_self)

    Link m_self;
public:

    Link getSelf();

    bool operator !=(Links& links);
    bool operator ==(Links& links);
};
}

/**
 * Represents a claimable balance response.
 * @see org.stellar.sdk.requests.ClaimableBalancesRequestBuilder
 * @see org.stellar.sdk.Server#claimableBalances()
 */

class ClaimableBalanceResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(QString id READ getID WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString asset READ getAssetString WRITE setAsset NOTIFY assetChanged)
    Q_PROPERTY(QString amount READ getAmount WRITE setAmount NOTIFY amountChanged)
    Q_PROPERTY(QString sponsor READ getSponsor WRITE setSponsor NOTIFY sponsorChanged)
    Q_PROPERTY(qint64 last_modified_ledger READ getLastModifiedLedger WRITE setLastModifiedLedger NOTIFY lastModifiedLedgerChanged)
    Q_PROPERTY(qint64 last_modified_time READ getLastModifiedTime  WRITE setLastModifiedTime  NOTIFY lastModifiedTimeChanged)
    Q_PROPERTY(QString paging_token READ getPagingToken WRITE setPagingToken NOTIFY pagingTokenChanged)
    Q_PROPERTY(ClaimableBalanceResponseAttach::Links _links READ getLinks WRITE setLinks NOTIFY linksChanged)
    Q_PROPERTY(QList<Claimant> claimants READ getClaimants WRITE setClaimants NOTIFY claimantsChanged)


public:
    Q_INVOKABLE explicit ClaimableBalanceResponse(QNetworkReply* reply=nullptr);
    virtual ~ClaimableBalanceResponse();
    QString getID() const;
    QString getAssetString() const;
    Asset *getAsset();

    QString getAmount() const;

    QString getSponsor() const;

    qint64 getLastModifiedLedger() const;

    qint64 getLastModifiedTime() const;

    QString getPagingToken() const;

    ClaimableBalanceResponseAttach::Links getLinks() const;

    QList<Claimant> getClaimants() const;

public slots:
    void setId(QString id);
    void setAsset(QString asset);

    void setAmount(QString amount);

    void setSponsor(QString sponsor);

    void setLastModifiedLedger(qint64 lastModifiedLedger);

    void setLastModifiedTime(qint64 lastModifiedTime);

    void setPagingToken(QString pagingToken);

    void setLinks(ClaimableBalanceResponseAttach::Links links);

    void setClaimants(QList<Claimant> claimants);

signals:
    void idChanged();
    void assetChanged();
    void amountChanged();
    void sponsorChanged();
    void lastModifiedLedgerChanged();
    void lastModifiedTimeChanged();
    void pagingTokenChanged();
    void linksChanged();
    void claimantsChanged();

private:
    QString m_id;
    QString m_assetString;
    Asset* m_asset;
    QString m_amount;
    QString m_sponsor;
    qint64 m_lastModifiedLedger;
    qint64 m_lastModifiedTime;
    QString m_pagingToken;
    ClaimableBalanceResponseAttach::Links m_links;
    QList<Claimant> m_claimants;
};
Q_DECLARE_METATYPE(ClaimableBalanceResponseAttach::Links)
#endif // CLAIMABLEBALANCERESPONSE_H
