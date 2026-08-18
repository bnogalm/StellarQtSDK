#ifndef MANAGESELLOFFEROPERATION_H
#define MANAGESELLOFFEROPERATION_H


#include "operation.h"
#include "xdr/stellartransaction.h"
#include "qstellar_namespace.h"
#include "price.h"
QSTELLAR_FWD(Asset)
/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#manage-sell-offer" target="_blank">ManageSellOffer</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
QSTELLAR_BEGIN_NS

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
     * Same as the one above, but taking the price as an EXACT FRACTION.
     * The `QString price` variant only approximates: it goes through
     * `Price(QString)`, which truncates to 11 characters and re-derives the
     * fraction over 1e9, so a price like 7/9 becomes 777777777/1000000000.
     * Use this overload when the exact n/d matters (XDR round-trip, txrep,
     * or a price read from the network).
     */
    ManageSellOfferOperation(Asset* selling, Asset* buying, QString amount, const Price& price, qint64 offerId);

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
     * The price as the EXACT FRACTION that goes into the XDR.
     * `getPrice()` returns a decimal string derived from n/d, so rebuilding a
     * Price from it goes through the `Price(QString)` approximation again and
     * does not always give back the same fraction. This is the authoritative
     * value.
     */
    Price getPriceR() const;

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

    /** Variant taking the price as an exact fraction (see the ctor). */
    static ManageSellOfferOperation* create(Asset* selling, Asset* buying, QString amount, const Price& price);

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(ManageSellOfferOperation)

#endif // MANAGESELLOFFEROPERATION_H
