#include "revokeclaimablebalancesponsorshipoperation.h"
#include "util.h"

QSTELLAR_BEGIN_NS


RevokeClaimableBalanceSponsorshipOperation::RevokeClaimableBalanceSponsorshipOperation(QString balanceID)
{    
    auto& ledgerKey = m_op.fillRevokeSponsorshipLedgerEntry();
    auto& claimableBalance = ledgerKey.fillClaimableBalance();
    // The same checked decode as claim and clawback: this one checked nothing
    // after reading, so "000000ff" + 64 hex built a four-byte balance id.
    Util::claimableBalanceIdToXDR(balanceID, claimableBalance.balanceID);
}

RevokeClaimableBalanceSponsorshipOperation::RevokeClaimableBalanceSponsorshipOperation(stellar::RevokeSponsorshipOp &op):m_op(op)
{

}

QString RevokeClaimableBalanceSponsorshipOperation::getBalanceId() const
{
    // Checked, as claim and clawback are: an id type with no arm writes
    // nothing, and came back as an empty balance id.
    return Util::bytesToHex(xdr::encodeOrThrow(m_op.ledgerKey.claimableBalance.balanceID,
                                               "the claimable balance id"));
}

void RevokeClaimableBalanceSponsorshipOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &op)
{
    Q_UNUSED(accountConverter)
    auto& o = op.fillRevokeSponsorshipOp();
    o = m_op;
}
RevokeClaimableBalanceSponsorshipOperation *RevokeClaimableBalanceSponsorshipOperation::build(stellar::RevokeSponsorshipOp &op)
{
    return new RevokeClaimableBalanceSponsorshipOperation(op);
}

RevokeClaimableBalanceSponsorshipOperation *RevokeClaimableBalanceSponsorshipOperation::create(QString balanceId)
{
    checkNotNull(balanceId, "balanceId cannot be null");
    return new RevokeClaimableBalanceSponsorshipOperation(balanceId);
}

RevokeClaimableBalanceSponsorshipOperation *RevokeClaimableBalanceSponsorshipOperation::setSourceAccount(QString sourceAccount)
{
    checkNotNull(sourceAccount, "sourceAccount cannot be null");
    Operation::setSourceAccount(sourceAccount);
    return this;
}
QSTELLAR_END_NS
