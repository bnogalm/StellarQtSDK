#ifndef LIQUIDITYPOOLRESPONSE_H
#define LIQUIDITYPOOLRESPONSE_H

#include "response.h"
#include "link.h"
#include "assetamount.h"

namespace LiquidityPoolResponseAttach
{
/** Links exposed by `/liquidity_pools/{id}`. */
class Links {
    Q_GADGET
    Q_PROPERTY(Link self MEMBER m_self)
    Q_PROPERTY(Link operations MEMBER m_operations)
    Q_PROPERTY(Link transactions MEMBER m_transactions)
    Link m_self;
    Link m_operations;
    Link m_transactions;
public:
    Link& getSelf()         { return m_self; }
    Link& getOperations()   { return m_operations; }
    Link& getTransactions() { return m_transactions; }

    bool operator==(const Links& o) const
    {
        return m_self == o.m_self
            && m_operations == o.m_operations
            && m_transactions == o.m_transactions;
    }
    bool operator!=(const Links& o) const { return !(*this == o); }
};
} // namespace

/**
 * Represents a Stellar liquidity pool as returned by Horizon.
 * @see https://developers.stellar.org/docs/data/apis/horizon/api-reference/resources/liquiditypools
 */
class LiquidityPoolResponse : public Response
{
    Q_OBJECT
    Q_PROPERTY(QString id MEMBER m_id)
    Q_PROPERTY(QString paging_token MEMBER m_pagingToken)
    Q_PROPERTY(qint32 fee_bp MEMBER m_feeBp)
    Q_PROPERTY(QString type MEMBER m_type)
    Q_PROPERTY(qint64 total_trustlines MEMBER m_totalTrustlines)
    Q_PROPERTY(QString total_shares MEMBER m_totalShares)
    Q_PROPERTY(QList<AssetAmount> reserves MEMBER m_reserves)
    Q_PROPERTY(qint64 last_modified_ledger MEMBER m_lastModifiedLedger)
    Q_PROPERTY(QString last_modified_time MEMBER m_lastModifiedTime)
    Q_PROPERTY(LiquidityPoolResponseAttach::Links _links MEMBER m_links)

    QString m_id;
    QString m_pagingToken;
    qint32  m_feeBp = 0;
    QString m_type;
    qint64  m_totalTrustlines = 0;
    QString m_totalShares;
    QList<AssetAmount> m_reserves;
    qint64  m_lastModifiedLedger = 0;
    QString m_lastModifiedTime;
    LiquidityPoolResponseAttach::Links m_links;

public:
    Q_INVOKABLE explicit LiquidityPoolResponse(QNetworkReply* reply = nullptr)
        : Response(reply) {}

    QString             getId() const                  { return m_id; }
    QString             getPagingToken() const         { return m_pagingToken; }
    qint32              getFeeBp() const               { return m_feeBp; }
    QString             getType() const                { return m_type; }
    qint64              getTotalTrustlines() const     { return m_totalTrustlines; }
    QString             getTotalShares() const         { return m_totalShares; }
    QList<AssetAmount>  getReserves() const            { return m_reserves; }
    qint64              getLastModifiedLedger() const  { return m_lastModifiedLedger; }
    QString             getLastModifiedTime() const    { return m_lastModifiedTime; }
    LiquidityPoolResponseAttach::Links getLinks() const { return m_links; }
};
Q_DECLARE_METATYPE(LiquidityPoolResponseAttach::Links)
Q_DECLARE_METATYPE(LiquidityPoolResponse*)

#endif // LIQUIDITYPOOLRESPONSE_H
