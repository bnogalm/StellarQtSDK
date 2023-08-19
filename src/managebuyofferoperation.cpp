#include "managebuyofferoperation.h"
#include "asset.h"
#include "price.h"
ManageBuyOfferOperation::ManageBuyOfferOperation()
{

}

ManageBuyOfferOperation::~ManageBuyOfferOperation()
{
    if(m_assetSelling)
        delete m_assetSelling;
    if(m_assetBuying)
        delete m_assetBuying;
}

ManageBuyOfferOperation::ManageBuyOfferOperation(stellar::ManageBuyOfferOp op)
    :m_op(op),m_assetSelling(nullptr),m_assetBuying(nullptr)
{
}

ManageBuyOfferOperation::ManageBuyOfferOperation(Asset *selling, Asset *buying, QString amount, QString price, qint64 offerId)
    :m_assetSelling(nullptr),m_assetBuying(nullptr)
{
    checkNotNull(selling, "selling cannot be null");
    checkNotNull(buying, "buying cannot be null");
    checkNotNull(amount, "amount cannot be null");
    checkNotNull(price, "price cannot be null");
    m_op.selling = selling->toXdr();
    m_op.buying = buying->toXdr();
    m_op.price = Price(price).toXdr();
    m_op.buyAmount = Operation::toXdrAmount(amount);
    // offerId can be null
    m_op.offerID = offerId;
}

Asset* ManageBuyOfferOperation::getSelling() {
    if(!m_assetSelling)
        m_assetSelling = Asset::fromXdr(m_op.selling);
    return m_assetSelling;
}

Asset *ManageBuyOfferOperation::getBuying() {
    if(!m_assetBuying)
        m_assetBuying = Asset::fromXdr(m_op.buying);
    return m_assetBuying;
}

QString ManageBuyOfferOperation::getAmount() {
    return Operation::fromXdrAmount(m_op.buyAmount);
}

QString ManageBuyOfferOperation::getPrice() {
    return Price::toString(m_op.price.n,m_op.price.d);
}

qint64 ManageBuyOfferOperation::getOfferId() {
    return m_op.offerID;
}

void ManageBuyOfferOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &op) {
    Q_UNUSED(accountConverter)
    op.operationManageBuyOffer = m_op;
    op.type = stellar::OperationType::MANAGE_BUY_OFFER;
}

ManageBuyOfferOperation *ManageBuyOfferOperation::build(stellar::ManageBuyOfferOp &op)
{
    ManageBuyOfferOperation *manageOffer = new ManageBuyOfferOperation(op);
    return manageOffer;

}

ManageBuyOfferOperation *ManageBuyOfferOperation::create(Asset *selling, Asset *buying, QString amount, QString price) {
    return new ManageBuyOfferOperation(selling,buying,amount,price,0);
}

ManageBuyOfferOperation *ManageBuyOfferOperation::setOfferId(qint64 offerId) {
    m_op.offerID = offerId;
    return this;
}

ManageBuyOfferOperation *ManageBuyOfferOperation::setSourceAccount(QString sourceAccount) {
    Operation::setSourceAccount(sourceAccount);
    return this;
}
