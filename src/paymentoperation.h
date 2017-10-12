#ifndef PAYMENTOPERATION_H
#define PAYMENTOPERATION_H

#include "operation.h"
#include "asset.h"

/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#payment" target="_blank">Payment</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
class PaymentOperation : public Operation
{
    KeyPair* m_destination;
    Asset* m_asset;
    stellar::PaymentOp m_op;
public:


    PaymentOperation(KeyPair* destination, Asset* asset, QString amount);
    PaymentOperation(stellar::PaymentOp &op);
    virtual ~PaymentOperation();

    static PaymentOperation * build(stellar::PaymentOp &op);

    static PaymentOperation * create(KeyPair* destination, Asset* asset, QString amount);

    /**
     * Account that receives the payment.
     */
    KeyPair* getDestination();

    /**
     * Asset to send to the destination account.
     */
    Asset* getAsset();

    /**
     * Amount of the asset to send.
     */
    QString getAmount();


    void fillOperationBody(stellar::Operation &op);

    /**
      * Sets the source account for this operation.
      * @param sourceAccount The operation's source account.
      * @return PaymentOperation object so you can chain methods.
      */
    PaymentOperation* setSourceAccount(KeyPair* sourceAccount);

    /**
      * Sets the source account for this operation making a copy of keypair
      * @param sourceAccount The operation's source account.
      * @return PaymentOperation object so you can chain methods.
      */
    PaymentOperation* setSourceAccount(KeyPair& sourceAccount);
};

#endif // PAYMENTOPERATION_H
