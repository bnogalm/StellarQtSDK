#include "revokeaccountsponsorshipoperation.h"

RevokeAccountSponsorshipOperation::RevokeAccountSponsorshipOperation(QString accountID)
{    
    auto& ledgerKey = m_op.fillRevokeSponsorshipLedgerEntry();
    auto& account = ledgerKey.fillAccount();
    account.accountID = StrKey::encodeToXDRAccountId(accountID);
}

RevokeAccountSponsorshipOperation::RevokeAccountSponsorshipOperation(stellar::RevokeSponsorshipOp &op):m_op(op)
{    
}

QString RevokeAccountSponsorshipOperation::getAccountId() const{
    return StrKey::encodeStellarAccountId(m_op.ledgerKey.account.accountID);
}

void RevokeAccountSponsorshipOperation::fillOperationBody(stellar::Operation &op)
{
    auto& o = op.fillRevokeSponsorshipOp();
    o = m_op;
}

RevokeAccountSponsorshipOperation *RevokeAccountSponsorshipOperation::build(stellar::RevokeSponsorshipOp &op)
{
    return new RevokeAccountSponsorshipOperation(op);
}

RevokeAccountSponsorshipOperation *RevokeAccountSponsorshipOperation::create(QString accountId)
{
    checkNotNull(accountId, "accountId cannot be null");
    return new RevokeAccountSponsorshipOperation(accountId);
}

RevokeAccountSponsorshipOperation *RevokeAccountSponsorshipOperation::setSourceAccount(QString sourceAccount)
{
    checkNotNull(sourceAccount, "sourceAccount cannot be null");
    Operation::setSourceAccount(sourceAccount);
    return this;
}
