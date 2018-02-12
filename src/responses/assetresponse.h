#ifndef ASSETRESPONSE_H
#define ASSETRESPONSE_H
#include "response.h"
#include "link.h"
#include "../asset.h"
#include "../assettypenative.h"
namespace AssetResponseAttach
{
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
    Q_PROPERTY(QString amount MEMBER m_amount)
    Q_PROPERTY(int num_accounts MEMBER m_numAccounts)
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

    AssetResponseAttach::Flags getFlags() const;

    AssetResponseAttach::Links getLinks() const;

public slots:
    void setAssetType(QString asset_type);
    void setAssetCode(QString asset_code);
    void setAssetIssuer(QString asset_issuer);
};

Q_DECLARE_METATYPE(AssetResponseAttach::Flags)
Q_DECLARE_METATYPE(AssetResponseAttach::Links)
#endif // ASSETRESPONSE_H
