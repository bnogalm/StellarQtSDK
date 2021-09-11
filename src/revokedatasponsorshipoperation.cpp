#include "revokedatasponsorshipoperation.h"

RevokeDataSponsorshipOperation::RevokeDataSponsorshipOperation(QString accountID, QString dataName)
{    
    auto& ledgerKey = m_op.fillRevokeSponsorshipLedgerEntry();
    auto& data = ledgerKey.fillData();
    data.accountID = StrKey::encodeToXDRAccountId(accountID);
    data.dataName = dataName;
}

RevokeDataSponsorshipOperation::RevokeDataSponsorshipOperation(stellar::RevokeSponsorshipOp &op):m_op(op)
{

}

QString RevokeDataSponsorshipOperation::getAccountId() const
{
    return StrKey::encodeStellarAccountId(m_op.ledgerKey.data.accountID);
}

QString RevokeDataSponsorshipOperation::getDataName() const
{
    return m_op.ledgerKey.data.dataName.toString();
}

void RevokeDataSponsorshipOperation::fillOperationBody(stellar::Operation &op)
{    
    auto& o = op.fillRevokeSponsorshipOp();
    o = m_op;
}

RevokeDataSponsorshipOperation *RevokeDataSponsorshipOperation::build(stellar::RevokeSponsorshipOp &op)
{
    return new RevokeDataSponsorshipOperation(op);
}

RevokeDataSponsorshipOperation *RevokeDataSponsorshipOperation::create(QString accountId, QString dataName)
{
    checkNotNull(accountId, "accountId cannot be null");
    return new RevokeDataSponsorshipOperation(accountId,dataName);
}

RevokeDataSponsorshipOperation *RevokeDataSponsorshipOperation::setSourceAccount(QString sourceAccount)
{
    checkNotNull(sourceAccount, "sourceAccount cannot be null");
    Operation::setSourceAccount(sourceAccount);
    return this;
}
