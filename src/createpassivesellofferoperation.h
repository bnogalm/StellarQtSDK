#ifndef CREATEPASSIVESELLOFFEROPERATION_H
#define CREATEPASSIVESELLOFFEROPERATION_H
#include "operation.h"
#include "qstellar_namespace.h"
#include "price.h"
QSTELLAR_FWD(Asset)

/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#create-passive-sell-offer" target="_blank">CreatePassiveSellOffer</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
QSTELLAR_BEGIN_NS

class CreatePassiveSellOfferOperation : public Operation
{
    Asset* m_selling;
    Asset* m_buying;

    stellar::CreatePassiveSellOfferOp m_op;

public:
    CreatePassiveSellOfferOperation(Asset* selling, Asset* buying, QString amount, QString price);

    /**
     * Same as the one above, but taking the price as an EXACT FRACTION.
     * The `QString price` variant only approximates (see Price(QString)).
     */
    CreatePassiveSellOfferOperation(Asset* selling, Asset* buying, QString amount, const Price& price);
    virtual ~CreatePassiveSellOfferOperation();
    CreatePassiveSellOfferOperation(stellar::CreatePassiveSellOfferOp &op);

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


    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);
    /**
    * Construct a new CreatePassiveSellOfferOperation builder from a CreatePassiveSellOfferOp XDR.
    * @param op
    */
    static CreatePassiveSellOfferOperation* build(stellar::CreatePassiveSellOfferOp& op);
    static CreatePassiveSellOfferOperation* create(Asset* selling, Asset* buying, QString amount, QString price);

    /** Variant taking the price as an exact fraction (see the ctor). */
    static CreatePassiveSellOfferOperation* create(Asset* selling, Asset* buying, QString amount, const Price& price);

    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return CreatePassiveSellOfferOperation object so you can chain methods.
         */
    CreatePassiveSellOfferOperation* setSourceAccount(QString sourceAccount);

};



QSTELLAR_END_NS

QSTELLAR_ALIAS(CreatePassiveSellOfferOperation)

#endif // CREATEPASSIVESELLOFFEROPERATION_H
