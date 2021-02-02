#ifndef CLAIMABLEBALANCERESPONSE_H
#define CLAIMABLEBALANCERESPONSE_H

#include "response.h"
#include <QObject>
#include "link.h"
#include "claimant.h"

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
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString asset READ getAsset WRITE setAsset NOTIFY assetChanged)
    Q_PROPERTY(QString amount READ getAmount WRITE setAmount NOTIFY amountChanged)
    Q_PROPERTY(QString sponsor READ getSponsor WRITE setSponsor NOTIFY sponsorChanged)
    Q_PROPERTY(qint64 last_modified_ledget READ getLastModified_ledget WRITE setLastModifiedLedget NOTIFY lastModifiedLedgetChanged)
    Q_PROPERTY(qint64 last_modified_time READ getLastModifiedTime  WRITE setLastModifiedTime  NOTIFY lastModifiedTimeChanged)
    Q_PROPERTY(QString paging_token READ getPagingToken WRITE setPagingToken NOTIFY pagingTokenChanged)
    Q_PROPERTY(ClaimableBalanceResponseAttach::Links _links READ getLinks WRITE setLinks NOTIFY linksChanged)
    Q_PROPERTY(QList<Claimant> claimants READ getClaimants WRITE setClaimants NOTIFY claimantsChanged)


public:
    Q_INVOKABLE explicit ClaimableBalanceResponse(QNetworkReply* reply=nullptr);
    QString id() const;
    QString getAsset() const;

    QString getAmount() const;

    QString getSponsor() const;

    qint64 getLastModified_ledget() const;

    qint64 getLastModifiedTime() const;

    QString getPagingToken() const;

    ClaimableBalanceResponseAttach::Links getLinks() const;

    QList<Claimant> getClaimants() const;

public slots:
    void setId(QString id);
    void setAsset(QString asset);

    void setAmount(QString amount);

    void setSponsor(QString sponsor);

    void setLastModifiedLedget(qint64 last_modified_ledget);

    void setLastModifiedTime(qint64 last_modified_time);

    void setPagingToken(QString paging_token);

    void setLinks(ClaimableBalanceResponseAttach::Links links);

    void setClaimants(QList<Claimant> claimants);

signals:
    void idChanged();
    void assetChanged();
    void amountChanged();
    void sponsorChanged();
    void lastModifiedLedgetChanged();
    void lastModifiedTimeChanged();
    void pagingTokenChanged();
    void linksChanged();
    void claimantsChanged();

private:
    QString m_id;
    QString m_asset;
    QString m_amount;
    QString m_sponsor;
    qint64 m_last_modified_ledget;
    qint64 m_last_modified_time;
    QString m_paging_token;
    ClaimableBalanceResponseAttach::Links m_links;
    QList<Claimant> m_claimants;
};

#endif // CLAIMABLEBALANCERESPONSE_H
