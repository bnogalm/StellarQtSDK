#include "liquiditypoolwithdrawoperation.h"

#include <cstring>
#include <stdexcept>

LiquidityPoolWithdrawOperation::LiquidityPoolWithdrawOperation(
    const QByteArray& liquidityPoolID,
    QString amount,
    QString minAmountA, QString minAmountB)
{
    if (liquidityPoolID.size() != 32) {
        throw std::runtime_error("LiquidityPoolID must be exactly 32 bytes");
    }
    std::memcpy(m_op.liquidityPoolID, liquidityPoolID.constData(), 32);
    m_op.amount = Operation::toXdrAmount(amount);
    m_op.minAmountA = Operation::toXdrAmount(minAmountA);
    m_op.minAmountB = Operation::toXdrAmount(minAmountB);
}

LiquidityPoolWithdrawOperation::LiquidityPoolWithdrawOperation(const stellar::LiquidityPoolWithdrawOp& op)
    : m_op(op)
{
}

void LiquidityPoolWithdrawOperation::fillOperationBody(AccountConverter& accountConverter, stellar::Operation& operation)
{
    Q_UNUSED(accountConverter)
    operation.type = stellar::OperationType::LIQUIDITY_POOL_WITHDRAW;
    operation.operationLiquidityPoolWithdraw = m_op;
}

QByteArray LiquidityPoolWithdrawOperation::getLiquidityPoolID() const
{
    return QByteArray(reinterpret_cast<const char*>(m_op.liquidityPoolID), 32);
}

QString LiquidityPoolWithdrawOperation::getAmount() const
{
    return Operation::fromXdrAmount(m_op.amount);
}

QString LiquidityPoolWithdrawOperation::getMinAmountA() const
{
    return Operation::fromXdrAmount(m_op.minAmountA);
}

QString LiquidityPoolWithdrawOperation::getMinAmountB() const
{
    return Operation::fromXdrAmount(m_op.minAmountB);
}

LiquidityPoolWithdrawOperation* LiquidityPoolWithdrawOperation::build(const stellar::LiquidityPoolWithdrawOp& op)
{
    return new LiquidityPoolWithdrawOperation(op);
}

LiquidityPoolWithdrawOperation* LiquidityPoolWithdrawOperation::create(
    const QByteArray& liquidityPoolID,
    QString amount,
    QString minAmountA, QString minAmountB)
{
    return new LiquidityPoolWithdrawOperation(liquidityPoolID, amount, minAmountA, minAmountB);
}

LiquidityPoolWithdrawOperation* LiquidityPoolWithdrawOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
