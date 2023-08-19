#ifndef REVOKEDATASPONSORSHIPOPERATION_H
#define REVOKEDATASPONSORSHIPOPERATION_H

#include "operation.h"

class RevokeDataSponsorshipOperation : public Operation
{
    stellar::RevokeSponsorshipOp m_op;
public:
    RevokeDataSponsorshipOperation(QString accountID, QString dataName);
    RevokeDataSponsorshipOperation(stellar::RevokeSponsorshipOp& op);
    QString getAccountId() const;
    QString getDataName() const;


    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &op);
    /**
     * Construct a new RevokeDataSponsorshipOperation builder from a RevokeSponsorship XDR.
     * @param op {@link RevokeSponsorshipOp}
     */
    static RevokeDataSponsorshipOperation *build(stellar::RevokeSponsorshipOp &op);
    /**
     * Creates a new RevokeDataSponsorshipOperation.
     * @param accountId The id of the account whose data entry will be revoked.
     * @param dataName The name of the data entry which will be revoked.
     */
    static RevokeDataSponsorshipOperation *create(QString accountId, QString dataName);
    /**
     * Sets the source account for this operation.
     * @param sourceAccount The operation's source account.
     * @return Builder object so you can chain methods.
     */
    RevokeDataSponsorshipOperation *setSourceAccount(QString sourceAccount);
};

#endif // REVOKEDATASPONSORSHIPOPERATION_H
