#include "revokeclaimablebalancesponsorshipoperation.h"
#include "util.h"

RevokeClaimableBalanceSponsorshipOperation::RevokeClaimableBalanceSponsorshipOperation(QString balanceID)
{    
    auto& ledgerKey = m_op.fillRevokeSponsorshipLedgerEntry();
    auto& claimableBalance = ledgerKey.fillClaimableBalance();


    QByteArray ba = Util::hexToBytes(balanceID);
    if(ba.size()!=(int)sizeof(claimableBalance.balanceID))
        throw std::runtime_error("invalid hash size");
    QDataStream stream(&ba,QIODevice::ReadOnly);
    stream >> claimableBalance.balanceID;

}

RevokeClaimableBalanceSponsorshipOperation::RevokeClaimableBalanceSponsorshipOperation(stellar::RevokeSponsorshipOp &op):m_op(op)
{

}

QString RevokeClaimableBalanceSponsorshipOperation::getBalanceId() const
{
    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream << m_op.ledgerKey.claimableBalance.balanceID;
    return Util::bytesToHex(data);
    //return Util::bytesToHex(QByteArray::fromRawData((const char*)(m_op.ledgerKey.claimableBalance.balanceID.v0),sizeof(m_op.ledgerKey.claimableBalance.balanceID.v0)));
}

void RevokeClaimableBalanceSponsorshipOperation::fillOperationBody(stellar::Operation &op)
{
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
