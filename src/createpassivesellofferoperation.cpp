#include "createpassivesellofferoperation.h"
#include "asset.h"
#include "price.h"

CreatePassiveSellOfferOperation::CreatePassiveSellOfferOperation(Asset *selling, Asset *buying, QString amount, QString price)
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

CreatePassiveSellOfferOperation::~CreatePassiveSellOfferOperation()
{
    if(m_selling)
        delete m_selling;
    if(m_buying)
        delete m_buying;
}

CreatePassiveSellOfferOperation::CreatePassiveSellOfferOperation(stellar::CreatePassiveSellOfferOp &op)
:m_selling(nullptr)
,m_buying(nullptr)
,m_op(op)
{

}

Asset *CreatePassiveSellOfferOperation::getSelling() {
    if(!m_selling){
        m_selling= Asset::fromXdr(m_op.selling);
    }
    return m_selling;
}

Asset *CreatePassiveSellOfferOperation::getBuying() {
    if(!m_buying){
        m_buying= Asset::fromXdr(m_op.buying);
    }
    return m_buying;
}

QString CreatePassiveSellOfferOperation::getAmount() {
    return Operation::fromXdrAmount(m_op.amount);;
}

QString CreatePassiveSellOfferOperation::getPrice() {
    return Price::toString(m_op.price.n,m_op.price.d);;
}

void CreatePassiveSellOfferOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &operation)
{
    Q_UNUSED(accountConverter)
    operation.type = stellar::OperationType::CREATE_PASSIVE_SELL_OFFER;
    operation.operationCreatePassiveSellOffer = m_op;
}

CreatePassiveSellOfferOperation *CreatePassiveSellOfferOperation::build(stellar::CreatePassiveSellOfferOp &op)
{
    return new CreatePassiveSellOfferOperation(op);
}

CreatePassiveSellOfferOperation *CreatePassiveSellOfferOperation::create(Asset* selling, Asset* buying, QString amount, QString price)
{
    return new CreatePassiveSellOfferOperation(selling, buying, amount, price);
}

CreatePassiveSellOfferOperation *CreatePassiveSellOfferOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
