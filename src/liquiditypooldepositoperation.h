#ifndef LIQUIDITYPOOLDEPOSITOPERATION_H
#define LIQUIDITYPOOLDEPOSITOPERATION_H

#include "operation.h"
#include "price.h"

/**
 * CAP-38 — deposit into a liquidity pool.
 * Amounts are passed as decimal strings (same convention as PaymentOperation).
 */
class LiquidityPoolDepositOperation : public Operation
{
    stellar::LiquidityPoolDepositOp m_op;
public:
    LiquidityPoolDepositOperation(const QByteArray& liquidityPoolID,
                                  QString maxAmountA, QString maxAmountB,
                                  const Price& minPrice, const Price& maxPrice);
    explicit LiquidityPoolDepositOperation(const stellar::LiquidityPoolDepositOp& op);
    ~LiquidityPoolDepositOperation() override = default;

    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation& operation) override;

    /** 32-byte PoolID. */
    QByteArray getLiquidityPoolID() const;
    QString getMaxAmountA() const;
    QString getMaxAmountB() const;
    Price getMinPrice() const;
    Price getMaxPrice() const;

    static LiquidityPoolDepositOperation* build(const stellar::LiquidityPoolDepositOp& op);
    static LiquidityPoolDepositOperation* create(const QByteArray& liquidityPoolID,
                                                 QString maxAmountA, QString maxAmountB,
                                                 const Price& minPrice, const Price& maxPrice);

    LiquidityPoolDepositOperation* setSourceAccount(QString sourceAccount);
};

#endif // LIQUIDITYPOOLDEPOSITOPERATION_H
