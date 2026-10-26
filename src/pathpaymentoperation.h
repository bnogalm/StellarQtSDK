#ifndef PATHPAYMENTOPERATION_H
#define PATHPAYMENTOPERATION_H



#include "qstellar_namespace.h"
#include "pathpaymentstrictreceiveoperation.h"

/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#path-payment" target="_blank">PathPayment</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
QSTELLAR_BEGIN_NS

/**
 * The pre-protocol-12 name of PathPaymentStrictReceiveOperation, kept for
 * 1.0.x callers. It is one now: Operation::fromXdr still returns this class
 * for a PATH_PAYMENT_STRICT_RECEIVE (so a 1.0.x dynamic_cast to it keeps
 * working), and dynamic_cast<PathPaymentStrictReceiveOperation*> works on it
 * too — it gave nullptr for every decoded strict-receive payment, the class
 * being a separate copy of the same code.
 */
class PathPaymentOperation : public PathPaymentStrictReceiveOperation
{
public:
    PathPaymentOperation();
    PathPaymentOperation(stellar::PathPaymentStrictReceiveOp &op);

    virtual ~PathPaymentOperation();

    PathPaymentOperation(Asset* sendAsset, QString sendMax, QString destination,
                         Asset* destAsset, QString destAmount, QList<Asset*> path = QList<Asset*>());

    static PathPaymentOperation * build(AccountConverter& accountConverter, stellar::PathPaymentStrictReceiveOp &op);
    static PathPaymentOperation * create(Asset* sendAsset, QString sendMax, KeyPair* destination,
                                         Asset* destAsset, QString destAmount);

    static PathPaymentOperation * create(Asset* sendAsset, QString sendMax, QString destination,
                                         Asset* destAsset, QString destAmount);
    /**
     * Sets path for this operation
     * @param path The assets (other than send asset and destination asset) involved in the offers the path takes. For example, if you can only find a path from USD to EUR through XLM and BTC, the path would be USD -&raquo; XLM -&raquo; BTC -&raquo; EUR and the path field would contain XLM and BTC.
     * @return PathPaymentOperation object so you can chain methods.
     */
    PathPaymentOperation* setPath(QList<Asset*> path);

    /**
     * Sets the source account for this operation.
     * @param sourceAccount The operation's source account.
     * @return PathPaymentOperation object so you can chain methods.
     */
    PathPaymentOperation* setSourceAccount(QString sourceAccount);

};

QSTELLAR_END_NS

QSTELLAR_ALIAS(PathPaymentOperation)

#endif // PATHPAYMENTOPERATION_H
