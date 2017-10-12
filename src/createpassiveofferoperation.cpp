#include "createpassiveofferoperation.h"
#include "asset.h"
#include "price.h"

CreatePassiveOfferOperation::CreatePassiveOfferOperation(Asset *selling, Asset *buying, QString amount, QString price)
    :m_selling(nullptr)
    ,m_buying(nullptr)
{
    checkNotNull(selling, "selling cannot be null");
    checkNotNull(buying, "buying cannot be null");
    checkNotNull(amount, "amount cannot be null");
    checkNotNull(price, "price cannot be null");

    m_op.selling = selling->toXdr();
    m_op.buying = buying->toXdr();
    m_op.amount = Operation::toXdrAmount(amount);
    m_op.price = Price(price).toXdr();
}

CreatePassiveOfferOperation::~CreatePassiveOfferOperation()
{
    if(m_selling)
        delete m_selling;
    if(m_buying)
        delete m_buying;
}

CreatePassiveOfferOperation::CreatePassiveOfferOperation(stellar::CreatePassiveOfferOp &op)
:m_selling(nullptr)
,m_buying(nullptr)
,m_op(op)
{

}

Asset *CreatePassiveOfferOperation::getSelling() {
    if(!m_selling){
        m_selling= Asset::fromXdr(m_op.selling);
    }
    return m_selling;
}

Asset *CreatePassiveOfferOperation::getBuying() {
    if(!m_buying){
        m_buying= Asset::fromXdr(m_op.buying);
    }
    return m_buying;
}

QString CreatePassiveOfferOperation::getAmount() {
    return Operation::fromXdrAmount(m_op.amount);;
}

QString CreatePassiveOfferOperation::getPrice() {
    return Price::toString(m_op.price.n,m_op.price.d);;
}

void CreatePassiveOfferOperation::fillOperationBody(stellar::Operation &operation)
{
    operation.type = stellar::OperationType::CREATE_PASSIVE_OFFER;
    operation.operationCreatePassiveOffer = m_op;
}

CreatePassiveOfferOperation *CreatePassiveOfferOperation::build(stellar::CreatePassiveOfferOp &op)
{
    return new CreatePassiveOfferOperation(op);
}

CreatePassiveOfferOperation *CreatePassiveOfferOperation::create(Asset* selling, Asset* buying, QString amount, QString price)
{
    return new CreatePassiveOfferOperation(selling, buying, amount, price);
}

CreatePassiveOfferOperation *CreatePassiveOfferOperation::setSourceAccount(KeyPair *sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}

CreatePassiveOfferOperation *CreatePassiveOfferOperation::setSourceAccount(KeyPair &sourceAccount)
{
    Operation::setSourceAccount(new KeyPair(sourceAccount));
    return this;
}
