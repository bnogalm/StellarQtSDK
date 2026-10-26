#ifndef REVOKETRUSTLINESPONSORSHIPOPERATION_H
#define REVOKETRUSTLINESPONSORSHIPOPERATION_H

#include "operation.h"
#include "qstellar_namespace.h"
#include <QObject>
#include <QByteArray>
#include "asset.h"

QSTELLAR_BEGIN_NS

class RevokeTrustlineSponsorshipOperation : public Operation
{
    Asset* m_asset;
    stellar::RevokeSponsorshipOp m_op;
public:
    RevokeTrustlineSponsorshipOperation(QString accountId, Asset* asset);
    /**
     * The trustline that holds shares of a liquidity pool (CAP-38): its key
     * names the pool, not an asset.
     * @param accountId The id of the account whose trustline will be revoked.
     * @param liquidityPoolId The pool id, exactly 32 raw bytes (anything else throws).
     */
    RevokeTrustlineSponsorshipOperation(QString accountId, const QByteArray& liquidityPoolId);
    RevokeTrustlineSponsorshipOperation(stellar::RevokeSponsorshipOp& op);
    virtual ~RevokeTrustlineSponsorshipOperation();
    QString getAccountId() const;

    /**
     * The asset of the trustline, or nullptr when the trustline holds shares
     * of a liquidity pool, which is no asset (see getLiquidityPoolID()).
     */
    Asset* getAsset();

    /** True when the trustline holds shares of a liquidity pool. */
    bool isLiquidityPoolShare() const;

    /**
     * The pool id (32 raw bytes) when the trustline holds shares of a
     * liquidity pool; an empty QByteArray otherwise.
     */
    QByteArray getLiquidityPoolID() const;

    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &op);
    /**
     * Construct a new RevokeTrustlineSponsorshipOperation builder from a RevokeSponsorship XDR.
     * @param op {@link RevokeSponsorshipOp}
     */
    static RevokeTrustlineSponsorshipOperation *build(stellar::RevokeSponsorshipOp &op);
    /**
     * Creates a new RevokeTrustlineSponsorshipOperation builder.
     * @param accountId The id of the account whose trustline will be revoked.
     * @param asset The asset of the trustline which will be revoked.
     */
    static RevokeTrustlineSponsorshipOperation *create(QString accountId, Asset* asset);
    /**
     * Creates a new RevokeTrustlineSponsorshipOperation builder for the
     * trustline that holds shares of a liquidity pool.
     * @param accountId The id of the account whose trustline will be revoked.
     * @param liquidityPoolId The pool id, exactly 32 raw bytes (anything else throws).
     */
    static RevokeTrustlineSponsorshipOperation *create(QString accountId, const QByteArray& liquidityPoolId);
    /**
     * Sets the source account for this operation.
     * @param sourceAccount The operation's source account.
     * @return Builder object so you can chain methods.
     */
    RevokeTrustlineSponsorshipOperation *setSourceAccount(QString sourceAccount);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(RevokeTrustlineSponsorshipOperation)

#endif // REVOKETRUSTLINESPONSORSHIPOPERATION_H
