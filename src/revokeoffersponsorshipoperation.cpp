#include "revokeoffersponsorshipoperation.h"

RevokeOfferSponsorshipOperation::RevokeOfferSponsorshipOperation(QString seller, qint64 offerId)
{
     auto& ledgerKey = m_op.fillRevokeSponsorshipLedgerEntry();
     auto& offer = ledgerKey.fillOffer();
     offer.sellerID = StrKey::encodeToXDRAccountId(seller);
     offer.offerID = offerId;
}

RevokeOfferSponsorshipOperation::RevokeOfferSponsorshipOperation(stellar::RevokeSponsorshipOp &op):m_op(op)
{

}

QString RevokeOfferSponsorshipOperation::getSeller() const
{
    return StrKey::encodeStellarAccountId(m_op.ledgerKey.offer.sellerID);
}

qint64 RevokeOfferSponsorshipOperation::getOfferId() const
{
    return m_op.ledgerKey.offer.offerID;
}

void RevokeOfferSponsorshipOperation::fillOperationBody(stellar::Operation &op)
{
    op.type = stellar::OperationType::REVOKE_SPONSORSHIP;
    new (&op.operationRevokeSponsorship) stellar::RevokeSponsorshipOp();
    op.operationRevokeSponsorship = m_op;
}

RevokeOfferSponsorshipOperation *RevokeOfferSponsorshipOperation::build(stellar::RevokeSponsorshipOp &op)
{
    return new RevokeOfferSponsorshipOperation(op);
}

RevokeOfferSponsorshipOperation *RevokeOfferSponsorshipOperation::create(QString seller, qint64 offerId)
{
    checkNotNull(seller, "seller cannot be null");
    checkNotNull(offerId, "offerId cannot be null");
    return new RevokeOfferSponsorshipOperation(seller, offerId);
}

RevokeOfferSponsorshipOperation *RevokeOfferSponsorshipOperation::setSourceAccount(QString sourceAccount)
{
    checkNotNull(sourceAccount, "sourceAccount cannot be null");
    Operation::setSourceAccount(sourceAccount);
    return this;
}
