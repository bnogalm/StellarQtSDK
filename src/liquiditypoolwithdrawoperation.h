#ifndef LIQUIDITYPOOLWITHDRAWOPERATION_H
#define LIQUIDITYPOOLWITHDRAWOPERATION_H

#include "operation.h"

/**
 * CAP-38 — withdraw from a liquidity pool.
 * Amounts are passed as decimal strings (same convention as PaymentOperation).
 */
class LiquidityPoolWithdrawOperation : public Operation
{
    stellar::LiquidityPoolWithdrawOp m_op;
public:
    LiquidityPoolWithdrawOperation(const QByteArray& liquidityPoolID,
                                   QString amount,
                                   QString minAmountA, QString minAmountB);
    explicit LiquidityPoolWithdrawOperation(const stellar::LiquidityPoolWithdrawOp& op);
    ~LiquidityPoolWithdrawOperation() override = default;

    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation& operation) override;

    /** 32-byte PoolID. */
    QByteArray getLiquidityPoolID() const;
    QString getAmount() const;
    QString getMinAmountA() const;
    QString getMinAmountB() const;

    static LiquidityPoolWithdrawOperation* build(const stellar::LiquidityPoolWithdrawOp& op);
    static LiquidityPoolWithdrawOperation* create(const QByteArray& liquidityPoolID,
                                                  QString amount,
                                                  QString minAmountA, QString minAmountB);

    LiquidityPoolWithdrawOperation* setSourceAccount(QString sourceAccount);
};

#endif // LIQUIDITYPOOLWITHDRAWOPERATION_H
