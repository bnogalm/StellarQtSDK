#include "revokedatasponsorshipoperation.h"

QSTELLAR_BEGIN_NS


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
    // A leading BOM kept, as ManageDataOperation::getName keeps it: dropped,
    // "\xEF\xBB\xBFk" read as "k", the name of another entry, and TxRep
    // got past the check that refuses a hidden U+FEFF (only its read-back
    // check then caught it).
    return Util::fromUtf8KeepingBom(m_op.ledgerKey.data.dataName.binary());
}

void RevokeDataSponsorshipOperation::fillOperationBody(AccountConverter& accountConverter, stellar::Operation &op)
{    
    Q_UNUSED(accountConverter)
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
QSTELLAR_END_NS
