#ifndef MANAGEBUYOFFEROPERATION_H
#define MANAGEBUYOFFEROPERATION_H


#include "operation.h"
#include "xdr/stellartransaction.h"
class Asset;
/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#manage-buy-offer" target="_blank">ManageBuyOffer</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
class ManageBuyOfferOperation : public Operation
{

    stellar::ManageBuyOfferOp m_op;
    Asset *m_assetSelling;
    Asset *m_assetBuying;
public:
    ManageBuyOfferOperation();
    virtual ~ManageBuyOfferOperation();

    ManageBuyOfferOperation(stellar::ManageBuyOfferOp op);
    ManageBuyOfferOperation(Asset* selling, Asset* buying, QString amount, QString price, qint64 offerId);

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
      * Construct a new ManageBuyOfferOperation builder from a ManageBuyOfferOp XDR.
      * @param op {@link ManageOfferOp}
      */
    static ManageBuyOfferOperation* build(stellar::ManageBuyOfferOp& op);



    /**
         * Creates a new ManageBuyOfferOperation. If you want to update existing offer use
         * {@link org.stellar.sdk.ManageSellOfferOperation.Builder#setOfferId(long)}.
         * @param selling The asset being sold in this operation
         * @param buying The asset being bought in this operation
         * @param amount Amount of selling being sold.
         * @param price Price of 1 unit of selling in terms of buying.
         * @throws ArithmeticException when amount has more than 7 decimal places.
         */
    static ManageBuyOfferOperation* create(Asset* selling, Asset* buying, QString amount, QString price);

    /**
         * Sets offer ID. <code>0</code> creates a new offer. Set to existing offer ID to change it.
         * @param offerId
         */
    ManageBuyOfferOperation* setOfferId(qint64 offerId);

    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return ManageBuyOfferOperation object so you can chain methods.
         */
    ManageBuyOfferOperation* setSourceAccount(QString sourceAccount);


};

#endif // MANAGEBUYOFFEROPERATION_H
