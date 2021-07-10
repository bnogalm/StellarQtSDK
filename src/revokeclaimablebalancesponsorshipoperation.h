#ifndef REVOKECLAIMABLEBALANCESPONSORSHIPOPERATION_H
#define REVOKECLAIMABLEBALANCESPONSORSHIPOPERATION_H

#include "operation.h"

class RevokeClaimableBalanceSponsorshipOperation : public Operation
{
    stellar::RevokeSponsorshipOp m_op;
public:
    RevokeClaimableBalanceSponsorshipOperation(QString balanceID);
    RevokeClaimableBalanceSponsorshipOperation(stellar::RevokeSponsorshipOp& op);
    QString getBalanceId() const;
    void fillOperationBody(stellar::Operation &op);
    /**
     * Construct a new RevokeClaimableBalanceSponsorshipOperation builder from a RevokeSponsorship XDR.
     * @param op {@link RevokeSponsorshipOp}
     */
    static RevokeClaimableBalanceSponsorshipOperation *build(stellar::RevokeSponsorshipOp &op);
    /**
     * Creates a new RevokeClaimableBalanceSponsorshipOperation builder.
     * @param balanceId The id of the claimable balance whose sponsorship will be revoked.
     */
    static RevokeClaimableBalanceSponsorshipOperation *create(QString balanceId);
    /**
     * Sets the source account for this operation.
     * @param sourceAccount The operation's source account.
     * @return Builder object so you can chain methods.
     */
    RevokeClaimableBalanceSponsorshipOperation *setSourceAccount(QString sourceAccount);
};

#endif // REVOKECLAIMABLEBALANCESPONSORSHIPOPERATION_H
