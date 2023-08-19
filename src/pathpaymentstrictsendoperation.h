#ifndef PATHPAYMENTSTRICTSENDOPERATION_H
#define PATHPAYMENTSTRICTSENDOPERATION_H

#include "operation.h"

#include "asset.h"
/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#path-payment-strict-receive" target="_blank">PathPaymentStrictReceive</a> operation.
 *
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
class PathPaymentStrictSendOperation : public Operation
{
    Asset* m_sendAsset;    
    Asset* m_destAsset;
    QList<Asset*> m_path;

    stellar::PathPaymentStrictSendOp m_op;
public:
    PathPaymentStrictSendOperation();
    PathPaymentStrictSendOperation(stellar::PathPaymentStrictSendOp &op);

    virtual ~PathPaymentStrictSendOperation();

    PathPaymentStrictSendOperation(Asset* sendAsset, QString sendAmount, QString destination,
                         Asset* destAsset, QString destMin, QList<Asset*> path = QList<Asset*>());

    /**
     * The asset deducted from the sender's account.
     */
    Asset* getSendAsset();

    /**
     * The amount of send asset to deduct (excluding fees)
     */
    QString getSendAmount();

    /**
     * Account that receives the payment.
     */
    QString getDestination() const;

    /**
     * The asset the destination account receives.
     */
    Asset* getDestAsset();

    /**
     * The minimum amount of destination asset the destination account receives.
     */
    QString getDestMin();

    /**
     * The assets (other than send asset and destination asset) involved in the offers the path takes. For example, if you can only find a path from USD to EUR through XLM and BTC, the path would be USD -&raquo; XLM -&raquo; BTC -&raquo; EUR and the path would contain XLM and BTC.
     */
    QList<Asset*> getPath();
    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &op);

    static PathPaymentStrictSendOperation * build(AccountConverter &accountConverter, stellar::PathPaymentStrictSendOp &op);
    static PathPaymentStrictSendOperation * create(Asset* sendAsset, QString sendMax, KeyPair* destination,
                                         Asset* destAsset, QString destAmount);

    static PathPaymentStrictSendOperation * create(Asset* sendAsset, QString sendMax, QString destination,
                                         Asset* destAsset, QString destAmount);
    /**
     * Sets path for this operation
     * @param path The assets (other than send asset and destination asset) involved in the offers the path takes. For example, if you can only find a path from USD to EUR through XLM and BTC, the path would be USD -&raquo; XLM -&raquo; BTC -&raquo; EUR and the path field would contain XLM and BTC.
     * @return PathPaymentStrictSendOperation object so you can chain methods.
     */
    PathPaymentStrictSendOperation* setPath(QList<Asset*> path);

    /**
     * Sets the source account for this operation.
     * @param sourceAccount The operation's source account.
     * @return PathPaymentStrictSendOperation object so you can chain methods.
     */
    PathPaymentStrictSendOperation* setSourceAccount(QString sourceAccount);

};


#endif // PATHPAYMENTSTRICTSENDOPERATION_H
