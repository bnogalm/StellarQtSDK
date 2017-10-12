#include "manageofferoperation.h"
#include "asset.h"
#include "price.h"
ManageOfferOperation::ManageOfferOperation()
{

}

ManageOfferOperation::~ManageOfferOperation()
{
    if(m_assetSelling)
        delete m_assetSelling;
    if(m_assetBuying)
        delete m_assetBuying;
}

ManageOfferOperation::ManageOfferOperation(stellar::ManageOfferOp op)
    :m_op(op),m_assetSelling(nullptr),m_assetBuying(nullptr)
{
}

ManageOfferOperation::ManageOfferOperation(Asset *selling, Asset *buying, QString amount, QString price, qint64 offerId)
    :m_assetSelling(nullptr),m_assetBuying(nullptr)
{
    checkNotNull(selling, "selling cannot be null");
    checkNotNull(buying, "buying cannot be null");
    checkNotNull(amount, "amount cannot be null");
    checkNotNull(price, "price cannot be null");
    m_op.selling = selling->toXdr();
    m_op.buying = buying->toXdr();
    m_op.price = Price(price).toXdr();
    m_op.amount = Operation::toXdrAmount(amount);
    // offerId can be null
    m_op.offerID = offerId;
}

Asset* ManageOfferOperation::getSelling() {
    if(!m_assetSelling)
        m_assetSelling = Asset::fromXdr(m_op.selling);
    return m_assetSelling;
}

Asset *ManageOfferOperation::getBuying() {
    if(!m_assetBuying)
        m_assetBuying = Asset::fromXdr(m_op.buying);
    return m_assetBuying;
}

QString ManageOfferOperation::getAmount() {
    return Operation::fromXdrAmount(m_op.amount);
}

QString ManageOfferOperation::getPrice() {
    return Price::toString(m_op.price.n,m_op.price.d);
}

qint64 ManageOfferOperation::getOfferId() {
    return m_op.offerID;
}

void ManageOfferOperation::fillOperationBody(stellar::Operation &op) {
    op.operationManageOffer = m_op;
    op.type = stellar::OperationType::MANAGE_OFFER;
}

ManageOfferOperation *ManageOfferOperation::build(stellar::ManageOfferOp &op)
{
    ManageOfferOperation *manageOffer = new ManageOfferOperation(op);
    return manageOffer;

}

ManageOfferOperation *ManageOfferOperation::create(Asset *selling, Asset *buying, QString amount, QString price) {
    return new ManageOfferOperation(selling,buying,amount,price,0);
}

ManageOfferOperation *ManageOfferOperation::setOfferId(qint64 offerId) {
    m_op.offerID = offerId;
    return this;
}

ManageOfferOperation *ManageOfferOperation::setSourceAccount(KeyPair *sourceAccount) {
    Operation::setSourceAccount(sourceAccount);
    return this;
}

ManageOfferOperation *ManageOfferOperation::setSourceAccount(KeyPair &sourceAccount)
{
    Operation::setSourceAccount(new KeyPair(sourceAccount));
    return this;
}
