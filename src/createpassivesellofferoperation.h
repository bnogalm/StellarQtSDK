#ifndef CREATEPASSIVESELLOFFEROPERATION_H
#define CREATEPASSIVESELLOFFEROPERATION_H
#include "operation.h"


class Asset;

/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#create-passive-sell-offer" target="_blank">CreatePassiveSellOffer</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
class CreatePassiveSellOfferOperation : public Operation
{
    Asset* m_selling;
    Asset* m_buying;

    stellar::CreatePassiveSellOfferOp m_op;

public:
    CreatePassiveSellOfferOperation(Asset* selling, Asset* buying, QString amount, QString price);
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


    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);
    /**
    * Construct a new CreatePassiveSellOfferOperation builder from a CreatePassiveSellOfferOp XDR.
    * @param op
    */
    static CreatePassiveSellOfferOperation* build(stellar::CreatePassiveSellOfferOp& op);
    static CreatePassiveSellOfferOperation* create(Asset* selling, Asset* buying, QString amount, QString price);

    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return CreatePassiveSellOfferOperation object so you can chain methods.
         */
    CreatePassiveSellOfferOperation* setSourceAccount(QString sourceAccount);

};



#endif // CREATEPASSIVESELLOFFEROPERATION_H
