#include "paymentoperation.h"


PaymentOperation::PaymentOperation(QString destination, Asset *asset, QString amount)
    :m_asset(nullptr)
{
    m_destination = checkNotNull(destination, "destination cannot be null");
    checkNotNull(asset, "asset cannot be null");
    checkNotNull(amount, "amount cannot be null");


    //m_op.destination = destination->getXdrPublicKey();// recipient of the payment
    m_op.asset = asset->toXdr();  // what they end up with
    m_op.amount = Operation::toXdrAmount(amount);// amount they end up with
}

PaymentOperation::PaymentOperation(stellar::PaymentOp &op)
    :m_asset(nullptr)
    ,m_op(op)
{

}

PaymentOperation::~PaymentOperation()
{
    if(m_asset)
        delete m_asset;
}

PaymentOperation* PaymentOperation::build(stellar::PaymentOp &op)
{
    return new PaymentOperation(op);
}

PaymentOperation *PaymentOperation::create(KeyPair *destination, Asset *asset, QString amount)
{
    return new PaymentOperation(destination->getAccountId(),asset,amount);
}

PaymentOperation *PaymentOperation::create(QString destination, Asset *asset, QString amount)
{
    return new PaymentOperation(destination,asset,amount);
}

QString PaymentOperation::getDestination() const {
    return m_destination;
}

Asset *PaymentOperation::getAsset() {
    if(!m_asset)
    {
        m_asset = Asset::fromXdr(m_op.asset);
    }
    return m_asset;
}

QString PaymentOperation::getAmount() {
    return Operation::fromXdrAmount(m_op.amount);
}

void PaymentOperation::fillOperationBody(stellar::Operation &op) {
    op.type = stellar::OperationType::PAYMENT;
    //payment op is trivial so no need to call constructor
    op.operationPayment = m_op;

}

PaymentOperation *PaymentOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
