#ifndef REVOKEACCOUNTSPONSORSHIPOPERATION_H
#define REVOKEACCOUNTSPONSORSHIPOPERATION_H

#include "operation.h"

class RevokeAccountSponsorshipOperation : public Operation
{
    stellar::RevokeSponsorshipOp m_op;
public:
    RevokeAccountSponsorshipOperation(QString accountID);
    RevokeAccountSponsorshipOperation(stellar::RevokeSponsorshipOp& op);
    QString getAccountId() const;

    void fillOperationBody(stellar::Operation &op);
    static RevokeAccountSponsorshipOperation* build(stellar::RevokeSponsorshipOp &op);
    /**
     * Creates a new RevokeAccountSponsorshipOperation builder.
     * @param accountId The id of the account whose sponsorship will be revoked.
     */
    static RevokeAccountSponsorshipOperation* create(QString accountId);
    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return CreateClaimableBalanceOperation object so you can chain methods.
         */
    RevokeAccountSponsorshipOperation* setSourceAccount(QString sourceAccount);
};

#endif // REVOKEACCOUNTSPONSORSHIPOPERATION_H
