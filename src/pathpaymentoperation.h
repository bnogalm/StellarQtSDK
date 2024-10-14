#ifndef PATHPAYMENTOPERATION_H
#define PATHPAYMENTOPERATION_H



#include "pathpaymentstrictreceiveoperation.h"

/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#path-payment" target="_blank">PathPayment</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
class PathPaymentOperation : public Operation
{
    Asset* m_sendAsset;    
    Asset* m_destAsset;
    QList<Asset*> m_path;

    stellar::PathPaymentStrictReceiveOp m_op;
public:
    PathPaymentOperation();
    PathPaymentOperation(stellar::PathPaymentStrictReceiveOp &op);

    virtual ~PathPaymentOperation();

    PathPaymentOperation(Asset* sendAsset, QString sendMax, QString destination,
                         Asset* destAsset, QString destAmount, QList<Asset*> path = QList<Asset*>());

    /**
     * The asset deducted from the sender's account.
     */
    Asset* getSendAsset();

    /**
     * The maximum amount of send asset to deduct (excluding fees)
     */
    QString getSendMax();

    /**
     * Account that receives the payment.
     */
    QString getDestination() const;

    /**
     * The asset the destination account receives.
     */
    Asset* getDestAsset();

    /**
     * The amount of destination asset the destination account receives.
     */
    QString getDestAmount();

    /**
     * The assets (other than send asset and destination asset) involved in the offers the path takes. For example, if you can only find a path from USD to EUR through XLM and BTC, the path would be USD -&raquo; XLM -&raquo; BTC -&raquo; EUR and the path would contain XLM and BTC.
     */
    QList<Asset*> getPath();
    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &op);

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

#endif // PATHPAYMENTOPERATION_H
