#include "revokesignersponsorshipoperation.h"


RevokeSignerSponsorshipOperation::RevokeSignerSponsorshipOperation(QString accountId, stellar::SignerKey signer)
{

    auto& sponsorshipSigner = m_op.fillRevokeSponsorshipSigner();
    sponsorshipSigner.accountID = StrKey::encodeToXDRAccountId(accountId);
    sponsorshipSigner.signerKey=signer;

}

RevokeSignerSponsorshipOperation::RevokeSignerSponsorshipOperation(stellar::RevokeSponsorshipOp& op):m_op(op)
{

}

QString RevokeSignerSponsorshipOperation::getAccountId() const
{
    return StrKey::encodeStellarAccountId(m_op.signer.accountID);
}

stellar::SignerKey RevokeSignerSponsorshipOperation::getSigner() const
{
    return m_op.signer.signerKey;
}

void RevokeSignerSponsorshipOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &op)
{
    Q_UNUSED(accountConverter)
    auto& o = op.fillRevokeSponsorshipOp();
    o = m_op;
}

RevokeSignerSponsorshipOperation *RevokeSignerSponsorshipOperation::build(stellar::RevokeSponsorshipOp &op)
{
    return new RevokeSignerSponsorshipOperation(op);
}

RevokeSignerSponsorshipOperation *RevokeSignerSponsorshipOperation::create(QString accountId, stellar::SignerKey signer)
{
    checkNotNull(accountId, "accountId cannot be null");    
    return new RevokeSignerSponsorshipOperation(accountId,signer);
}

RevokeSignerSponsorshipOperation *RevokeSignerSponsorshipOperation::setSourceAccount(QString sourceAccount)
{
    checkNotNull(sourceAccount, "sourceAccount cannot be null");
    Operation::setSourceAccount(sourceAccount);
    return this;
}
