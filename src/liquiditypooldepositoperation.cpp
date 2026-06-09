#include "liquiditypooldepositoperation.h"

#include <cstring>
#include <stdexcept>

QSTELLAR_BEGIN_NS


LiquidityPoolDepositOperation::LiquidityPoolDepositOperation(
    const QByteArray& liquidityPoolID,
    QString maxAmountA, QString maxAmountB,
    const Price& minPrice, const Price& maxPrice)
{
    if (liquidityPoolID.size() != 32) {
        throw std::runtime_error("LiquidityPoolID must be exactly 32 bytes");
    }
    std::memcpy(m_op.liquidityPoolID, liquidityPoolID.constData(), 32);
    m_op.maxAmountA = Operation::toXdrAmount(maxAmountA);
    m_op.maxAmountB = Operation::toXdrAmount(maxAmountB);
    m_op.minPrice.n = minPrice.getNumerator();
    m_op.minPrice.d = minPrice.getDenominator();
    m_op.maxPrice.n = maxPrice.getNumerator();
    m_op.maxPrice.d = maxPrice.getDenominator();
}

LiquidityPoolDepositOperation::LiquidityPoolDepositOperation(const stellar::LiquidityPoolDepositOp& op)
    : m_op(op)
{
}

void LiquidityPoolDepositOperation::fillOperationBody(AccountConverter& accountConverter, stellar::Operation& operation)
{
    Q_UNUSED(accountConverter)
    operation.type = stellar::OperationType::LIQUIDITY_POOL_DEPOSIT;
    operation.operationLiquidityPoolDeposit = m_op;
}

QByteArray LiquidityPoolDepositOperation::getLiquidityPoolID() const
{
    return QByteArray(reinterpret_cast<const char*>(m_op.liquidityPoolID), 32);
}

QString LiquidityPoolDepositOperation::getMaxAmountA() const
{
    return Operation::fromXdrAmount(m_op.maxAmountA);
}

QString LiquidityPoolDepositOperation::getMaxAmountB() const
{
    return Operation::fromXdrAmount(m_op.maxAmountB);
}

Price LiquidityPoolDepositOperation::getMinPrice() const
{
    return Price(m_op.minPrice.n, m_op.minPrice.d);
}

Price LiquidityPoolDepositOperation::getMaxPrice() const
{
    return Price(m_op.maxPrice.n, m_op.maxPrice.d);
}

LiquidityPoolDepositOperation* LiquidityPoolDepositOperation::build(const stellar::LiquidityPoolDepositOp& op)
{
    return new LiquidityPoolDepositOperation(op);
}

LiquidityPoolDepositOperation* LiquidityPoolDepositOperation::create(
    const QByteArray& liquidityPoolID,
    QString maxAmountA, QString maxAmountB,
    const Price& minPrice, const Price& maxPrice)
{
    return new LiquidityPoolDepositOperation(liquidityPoolID, maxAmountA, maxAmountB, minPrice, maxPrice);
}

LiquidityPoolDepositOperation* LiquidityPoolDepositOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
QSTELLAR_END_NS
