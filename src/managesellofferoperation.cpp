#include "managesellofferoperation.h"
#include "asset.h"
#include "price.h"
ManageSellOfferOperation::ManageSellOfferOperation()
{

}

ManageSellOfferOperation::~ManageSellOfferOperation()
{
    if(m_assetSelling)
        delete m_assetSelling;
    if(m_assetBuying)
        delete m_assetBuying;
}

ManageSellOfferOperation::ManageSellOfferOperation(stellar::ManageSellOfferOp op)
    :m_op(op),m_assetSelling(nullptr),m_assetBuying(nullptr)
{
}

ManageSellOfferOperation::ManageSellOfferOperation(Asset *selling, Asset *buying, QString amount, QString price, qint64 offerId)
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

Asset* ManageSellOfferOperation::getSelling() {
    if(!m_assetSelling)
        m_assetSelling = Asset::fromXdr(m_op.selling);
    return m_assetSelling;
}

Asset *ManageSellOfferOperation::getBuying() {
    if(!m_assetBuying)
        m_assetBuying = Asset::fromXdr(m_op.buying);
    return m_assetBuying;
}

QString ManageSellOfferOperation::getAmount() {
    return Operation::fromXdrAmount(m_op.amount);
}

QString ManageSellOfferOperation::getPrice() {
    return Price::toString(m_op.price.n,m_op.price.d);
}

qint64 ManageSellOfferOperation::getOfferId() {
    return m_op.offerID;
}

void ManageSellOfferOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &op) {
    Q_UNUSED(accountConverter)
    op.operationManageSellOffer = m_op;
    op.type = stellar::OperationType::MANAGE_SELL_OFFER;
}

ManageSellOfferOperation *ManageSellOfferOperation::build(stellar::ManageSellOfferOp &op)
{
    ManageSellOfferOperation *manageOffer = new ManageSellOfferOperation(op);
    return manageOffer;

}

ManageSellOfferOperation *ManageSellOfferOperation::create(Asset *selling, Asset *buying, QString amount, QString price) {
    return new ManageSellOfferOperation(selling,buying,amount,price,0);
}

ManageSellOfferOperation *ManageSellOfferOperation::setOfferId(qint64 offerId) {
    m_op.offerID = offerId;
    return this;
}

ManageSellOfferOperation *ManageSellOfferOperation::setSourceAccount(QString sourceAccount) {
    Operation::setSourceAccount(sourceAccount);
    return this;
}
