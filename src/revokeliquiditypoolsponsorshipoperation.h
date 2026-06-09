#ifndef REVOKELIQUIDITYPOOLSPONSORSHIPOPERATION_H
#define REVOKELIQUIDITYPOOLSPONSORSHIPOPERATION_H

#include "operation.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class RevokeLiquidityPoolSponsorshipOperation : public Operation
{
    stellar::RevokeSponsorshipOp m_op;
public:
    RevokeLiquidityPoolSponsorshipOperation(QString liquidityPoolId);
    RevokeLiquidityPoolSponsorshipOperation(stellar::RevokeSponsorshipOp& op);
    QString getLiquidityPoolId() const;
    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &op);
    /**
     * Construct a new RevokeLiquidityPoolSponsorshipOperation builder from a RevokeSponsorship XDR.
     * @param op {@link RevokeSponsorshipOp}
     */
    static RevokeLiquidityPoolSponsorshipOperation *build(stellar::RevokeSponsorshipOp &op);
    /**
     * Creates a new RevokeLiquidityPoolSponsorshipOperation builder.
     * @param liquidityPoolId The id (hex) of the liquidity pool whose sponsorship will be revoked.
     */
    static RevokeLiquidityPoolSponsorshipOperation *create(QString liquidityPoolId);
    /**
     * Sets the source account for this operation.
     * @param sourceAccount The operation's source account.
     * @return Builder object so you can chain methods.
     */
    RevokeLiquidityPoolSponsorshipOperation *setSourceAccount(QString sourceAccount);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(RevokeLiquidityPoolSponsorshipOperation)

#endif // REVOKELIQUIDITYPOOLSPONSORSHIPOPERATION_H
