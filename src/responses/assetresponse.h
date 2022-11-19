#ifndef ASSETRESPONSE_H
#define ASSETRESPONSE_H
#include "response.h"
#include "link.h"
#include "../asset.h"
#include "../assettypenative.h"

namespace AssetResponseAttach
{
/**
 * Accounts describe asset accounts.
 */
class Accounts {
    Q_GADGET

    Q_PROPERTY(int authorized MEMBER m_authorized)
    Q_PROPERTY(int authorized_to_maintain_liabilities MEMBER m_authorizedToMaintainLiabilities)
    Q_PROPERTY(int unauthorized MEMBER m_unauthorized)

    int m_authorized;
    int m_authorizedToMaintainLiabilities;
    int m_unauthorized;
public:
    Accounts();

    int authorized();
    int authorizedToMaintainLiabilities();
    int unauthorized();
    bool operator !=(Accounts& t);
};

/**
 * Balances describe asset balances.
 */
class Balances {
    Q_GADGET
    Q_PROPERTY(QString authorized MEMBER m_authorized)
    Q_PROPERTY(QString authorized_to_maintain_liabilities MEMBER m_authorizedToMaintainLiabilities)
    Q_PROPERTY(QString unauthorized MEMBER m_unauthorized)

    QString m_authorized;
    QString m_authorizedToMaintainLiabilities;
    QString m_unauthorized;
public:
    Balances();

    QString authorized();
    QString authorizedToMaintainLiabilities();
    QString unauthorized();
    bool operator !=(Balances& t);
};
/**
 * Flags describe asset flags.
 */
class Flags {
    Q_GADGET
    Q_PROPERTY(bool auth_required MEMBER m_authRequired)
    Q_PROPERTY(bool auth_revocable MEMBER m_authRevocable)
    bool m_authRequired;
    bool m_authRevocable;
public:
    Flags();

    bool isAuthRequired();

    bool isAuthRevocable();
    bool operator !=(Flags& t);
};

/**
 * Links connected to asset.
 */
class Links {
    Q_GADGET
    Q_PROPERTY(Link toml MEMBER m_toml)
    Link m_toml;
public:
    Links();

    Link getToml();
    bool operator !=(Links& t);
};

}


class AssetResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(QString asset_type READ getAssetType WRITE setAssetType)
    Q_PROPERTY(QString asset_code READ getAssetCode WRITE setAssetCode)
    Q_PROPERTY(QString asset_issuer READ getAssetIssuer WRITE setAssetIssuer)
    Q_PROPERTY(QString paging_token MEMBER m_pagingToken)
    Q_PROPERTY(AssetResponseAttach::Accounts accounts MEMBER m_accounts)
    Q_PROPERTY(AssetResponseAttach::Balances balances MEMBER m_balances)
    Q_PROPERTY(QString amount MEMBER m_amount)
    Q_PROPERTY(QString claimable_balances_amount MEMBER m_claimableBalancesAmount)
    Q_PROPERTY(int num_accounts MEMBER m_numAccounts)
    Q_PROPERTY(int num_claimable_balances MEMBER m_numClaimableBalances)
    Q_PROPERTY(AssetResponseAttach::Flags flags MEMBER m_flags)
    Q_PROPERTY(AssetResponseAttach::Links _links MEMBER m_links)

    QString m_assetType;
    QString m_assetCode;
    QString m_assetIssuer;
    Asset *m_asset;
    QString m_pagingToken;
    QString m_amount;
    int m_numAccounts;
    AssetResponseAttach::Flags m_flags;
    AssetResponseAttach::Links m_links;

    AssetResponseAttach::Accounts m_accounts;

    AssetResponseAttach::Balances m_balances;

    QString m_claimableBalancesAmount;

    int m_numClaimableBalances;

public:
    Q_INVOKABLE explicit AssetResponse(QNetworkReply *reply=nullptr);
    virtual ~AssetResponse();
public:

    QString getAssetType() const;

    QString getAssetCode() const;

    QString getAssetIssuer() const;

    Asset* getAsset();

    QString getPagingToken() const;

    QString getAmount() const;

    int getNumAccounts() const;

    AssetResponseAttach::Accounts getAccounts() const;

    AssetResponseAttach::Balances getBalances() const;

    QString getClaimableBalancesAmount() const;

    int getNumClaimableBalances();

    AssetResponseAttach::Flags getFlags() const;

    AssetResponseAttach::Links getLinks() const;

public slots:
    void setAssetType(QString asset_type);
    void setAssetCode(QString asset_code);
    void setAssetIssuer(QString asset_issuer);
};

Q_DECLARE_METATYPE(AssetResponseAttach::Accounts)
Q_DECLARE_METATYPE(AssetResponseAttach::Balances)
Q_DECLARE_METATYPE(AssetResponseAttach::Flags)
Q_DECLARE_METATYPE(AssetResponseAttach::Links)
#endif // ASSETRESPONSE_H
