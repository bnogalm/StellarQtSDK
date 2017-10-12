#ifndef CREATEPASSIVEOFFEROPERATION_H
#define CREATEPASSIVEOFFEROPERATION_H
#include "operation.h"


class Asset;
/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#create-passive-offer" target="_blank">CreatePassiveOffer</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
class CreatePassiveOfferOperation : public Operation
{
    Asset* m_selling;
    Asset* m_buying;

    stellar::CreatePassiveOfferOp m_op;

public:
    CreatePassiveOfferOperation(Asset* selling, Asset* buying, QString amount, QString price);
    virtual ~CreatePassiveOfferOperation();
    CreatePassiveOfferOperation(stellar::CreatePassiveOfferOp &op);

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


    void fillOperationBody(stellar::Operation &operation);
    /**
    * Construct a new CreatePassiveOffer builder from a CreatePassiveOfferOp XDR.
    * @param op
    */
    static CreatePassiveOfferOperation* build(stellar::CreatePassiveOfferOp& op);
    static CreatePassiveOfferOperation* create(Asset* selling, Asset* buying, QString amount, QString price);

    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return CreatePassiveOfferOperation object so you can chain methods.
         */
    CreatePassiveOfferOperation* setSourceAccount(KeyPair* sourceAccount);

    /**
         * Sets the source account for this operation making a copy of keypair
         * @param sourceAccount The operation's source account.
         * @return CreatePassiveOfferOperation object so you can chain methods.
         */
    CreatePassiveOfferOperation* setSourceAccount(KeyPair& sourceAccount);
};

#endif // CREATEPASSIVEOFFEROPERATION_H




