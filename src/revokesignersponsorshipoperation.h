#ifndef REVOKESIGNERSPONSORSHIPOPERATION_H
#define REVOKESIGNERSPONSORSHIPOPERATION_H

#include "operation.h"

class RevokeSignerSponsorshipOperation : public Operation
{

    stellar::RevokeSponsorshipOp m_op;
public:
    RevokeSignerSponsorshipOperation(QString accountId, stellar::SignerKey signer);
    RevokeSignerSponsorshipOperation(stellar::RevokeSponsorshipOp& op);
    QString getAccountId() const;
    stellar::SignerKey getSigner() const;

    void fillOperationBody(stellar::Operation &op);
    /**
     * Construct a new RevokeSignerSponsorshipOperation builder from a RevokeSponsorship XDR.
     * @param op {@link RevokeSponsorshipOp}
     */
    static RevokeSignerSponsorshipOperation *build(stellar::RevokeSponsorshipOp &op);
    /**
     * Creates a new RevokeSignerSponsorshipOperation.
     * @param accountId The id of the account whose signer will be revoked.
     * @param signer The signer whose sponsorship which will be revoked.
     */
    static RevokeSignerSponsorshipOperation *create(QString accountId, stellar::SignerKey signer);
    /**
     * Sets the source account for this operation.
     * @param sourceAccount The operation's source account.
     * @return Builder object so you can chain methods.
     */
    RevokeSignerSponsorshipOperation *setSourceAccount(QString sourceAccount);
};

#endif // REVOKESIGNERSPONSORSHIPOPERATION_H
