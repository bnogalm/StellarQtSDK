#include "revoketrustlinesponsorshipoperation.h"



RevokeTrustlineSponsorshipOperation::RevokeTrustlineSponsorshipOperation(QString accountId, Asset *asset)
{
    auto& revokeSponsorshipLedgerEntry =m_op.fillRevokeSponsorshipLedgerEntry();
    auto& trustLine = revokeSponsorshipLedgerEntry.fillTrustLine();
    trustLine.accountID = StrKey::encodeToXDRAccountId(accountId);
    trustLine.asset = asset->toXdr();
}

RevokeTrustlineSponsorshipOperation::RevokeTrustlineSponsorshipOperation(stellar::RevokeSponsorshipOp &op):m_asset(nullptr),m_op(op)
{

}

RevokeTrustlineSponsorshipOperation::~RevokeTrustlineSponsorshipOperation()
{
    if(m_asset)
        delete m_asset;
}

QString RevokeTrustlineSponsorshipOperation::getAccountId() const
{
    return StrKey::encodeStellarAccountId(m_op.ledgerKey.trustLine.accountID);
}

Asset *RevokeTrustlineSponsorshipOperation::getAsset()
{
    if(!m_asset)
    {
        m_asset = Asset::fromXdr(m_op.ledgerKey.trustLine.asset);
    }
    return m_asset;
}

void RevokeTrustlineSponsorshipOperation::fillOperationBody(stellar::Operation &op)
{
    auto& o = op.fillRevokeSponsorshipOp();
    o = m_op;
}

RevokeTrustlineSponsorshipOperation *RevokeTrustlineSponsorshipOperation::build(stellar::RevokeSponsorshipOp &op)
{
    return new RevokeTrustlineSponsorshipOperation(op);
}

RevokeTrustlineSponsorshipOperation *RevokeTrustlineSponsorshipOperation::create(QString accountId, Asset *asset)
{
    checkNotNull(accountId, "accountId cannot be null");
    checkNotNull(asset, "asset cannot be null");
    return new RevokeTrustlineSponsorshipOperation(accountId,asset);
}

RevokeTrustlineSponsorshipOperation *RevokeTrustlineSponsorshipOperation::setSourceAccount(QString sourceAccount)
{
    checkNotNull(sourceAccount, "sourceAccount cannot be null");
    Operation::setSourceAccount(sourceAccount);
    return this;
}
