#include "revokeliquiditypoolsponsorshipoperation.h"
#include "util.h"
#include <cstring>

QSTELLAR_BEGIN_NS


RevokeLiquidityPoolSponsorshipOperation::RevokeLiquidityPoolSponsorshipOperation(QString liquidityPoolId)
{
    auto& ledgerKey = m_op.fillRevokeSponsorshipLedgerEntry();
    auto& liquidityPool = ledgerKey.fillLiquidityPool();

    QByteArray ba = Util::hexToBytes(liquidityPoolId);
    if(ba.size()!=(int)sizeof(liquidityPool.liquidityPoolID))
        throw std::runtime_error("invalid hash size");
    // PoolID is a raw quint8[32]; copy bytes directly (the QDataStream operator
    // for the Hash array type is only visible inside the XDR headers).
    memcpy(liquidityPool.liquidityPoolID, ba.constData(), sizeof(liquidityPool.liquidityPoolID));

}

RevokeLiquidityPoolSponsorshipOperation::RevokeLiquidityPoolSponsorshipOperation(stellar::RevokeSponsorshipOp &op):m_op(op)
{

}

QString RevokeLiquidityPoolSponsorshipOperation::getLiquidityPoolId() const
{
    QByteArray data(reinterpret_cast<const char*>(m_op.ledgerKey.liquidityPool.liquidityPoolID),
                    sizeof(m_op.ledgerKey.liquidityPool.liquidityPoolID));
    return Util::bytesToHex(data);
}

void RevokeLiquidityPoolSponsorshipOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &op)
{
    Q_UNUSED(accountConverter)
    auto& o = op.fillRevokeSponsorshipOp();
    o = m_op;
}
RevokeLiquidityPoolSponsorshipOperation *RevokeLiquidityPoolSponsorshipOperation::build(stellar::RevokeSponsorshipOp &op)
{
    return new RevokeLiquidityPoolSponsorshipOperation(op);
}

RevokeLiquidityPoolSponsorshipOperation *RevokeLiquidityPoolSponsorshipOperation::create(QString liquidityPoolId)
{
    checkNotNull(liquidityPoolId, "liquidityPoolId cannot be null");
    return new RevokeLiquidityPoolSponsorshipOperation(liquidityPoolId);
}

RevokeLiquidityPoolSponsorshipOperation *RevokeLiquidityPoolSponsorshipOperation::setSourceAccount(QString sourceAccount)
{
    checkNotNull(sourceAccount, "sourceAccount cannot be null");
    Operation::setSourceAccount(sourceAccount);
    return this;
}
QSTELLAR_END_NS
