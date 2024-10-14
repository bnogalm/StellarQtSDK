#ifndef MANAGESELLOFFEROPERATION_H
#define MANAGESELLOFFEROPERATION_H


#include "operation.h"
#include "xdr/stellartransaction.h"
class Asset;
/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#manage-sell-offer" target="_blank">ManageSellOffer</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
class ManageSellOfferOperation : public Operation
{

    stellar::ManageSellOfferOp m_op;
    Asset *m_assetSelling;
    Asset *m_assetBuying;
public:
    ManageSellOfferOperation();
    virtual ~ManageSellOfferOperation();

    ManageSellOfferOperation(stellar::ManageSellOfferOp op);
    ManageSellOfferOperation(Asset* selling, Asset* buying, QString amount, QString price, qint64 offerId);

    /**
      * The asset being sold in this operation
      */
    Asset* getSelling();

    /**
      * The asset being bought in this operation
      */
    Asset* getBuying();

    /**
      * Amount of selling being sold.
      */
    QString getAmount();

    /**
      * Price of 1 unit of selling in terms of buying.
      */
    QString getPrice();

    /**
      * The ID of the offer.
      */
    qint64 getOfferId();


    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation& op);

    /**
      * Construct a new ManageSellOfferOperation builder from a ManageSellOfferOp XDR.
      * @param op {@link ManageOfferOp}
      */
    static ManageSellOfferOperation* build(stellar::ManageSellOfferOp& op);



    /**
         * Creates a new ManageOffer. If you want to update existing offer use
         * {@link org.stellar.sdk.ManageSellOfferOperation.Builder#setOfferId(long)}.
         * @param selling The asset being sold in this operation
         * @param buying The asset being bought in this operation
         * @param amount Amount of selling being sold.
         * @param price Price of 1 unit of selling in terms of buying.
         * @throws ArithmeticException when amount has more than 7 decimal places.
         */
    static ManageSellOfferOperation* create(Asset* selling, Asset* buying, QString amount, QString price);

    /**
         * Sets offer ID. <code>0</code> creates a new offer. Set to existing offer ID to change it.
         * @param offerId
         */
    ManageSellOfferOperation* setOfferId(qint64 offerId);

    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return ManageOfferOperation object so you can chain methods.
         */
    ManageSellOfferOperation* setSourceAccount(QString sourceAccount);


};

#endif // MANAGESELLOFFEROPERATION_H
