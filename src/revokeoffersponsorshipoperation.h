#ifndef REVOKEOFFERSPONSORSHIPOPERATION_H
#define REVOKEOFFERSPONSORSHIPOPERATION_H

#include "operation.h"

class RevokeOfferSponsorshipOperation : public Operation
{
    stellar::RevokeSponsorshipOp m_op;
public:
    RevokeOfferSponsorshipOperation(QString seller, qint64 offerId);
    RevokeOfferSponsorshipOperation(stellar::RevokeSponsorshipOp& op);


    QString getSeller() const;
    qint64 getOfferId() const;


    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &op);
    /**
     * Construct a new RevokeOfferSponsorshipOperation builder from a RevokeSponsorship XDR.
     * @param op {@link RevokeSponsorshipOp}
     */
    static RevokeOfferSponsorshipOperation *build(stellar::RevokeSponsorshipOp &op);
    /**
     * Creates a new RevokeOfferSponsorshipOperation.
     * @param offerId The id of the offer whose sponsorship will be revoked.
     */
    static RevokeOfferSponsorshipOperation *create(QString seller, qint64 offerId);
    /**
     * Sets the source account for this operation.
     * @param sourceAccount The operation's source account.
     * @return Builder object so you can chain methods.
     */
    RevokeOfferSponsorshipOperation *setSourceAccount(QString sourceAccount);
};

#endif // REVOKEOFFERSPONSORSHIPOPERATION_H
