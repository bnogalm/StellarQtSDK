#include "paymentoperation.h"


PaymentOperation::PaymentOperation(KeyPair *destination, Asset *asset, QString amount)
    :m_destination(nullptr)
    ,m_asset(nullptr)
{
    checkNotNull(destination, "destination cannot be null");
    checkNotNull(asset, "asset cannot be null");
    checkNotNull(amount, "amount cannot be null");


    m_op.destination = destination->getXdrPublicKey();// recipient of the payment
    m_op.asset = asset->toXdr();  // what they end up with
    m_op.amount = Operation::toXdrAmount(amount);// amount they end up with
}

PaymentOperation::PaymentOperation(stellar::PaymentOp &op)
    :m_destination(nullptr)
    ,m_asset(nullptr)
    ,m_op(op)
{

}

PaymentOperation::~PaymentOperation()
{
    if(m_destination)
        delete m_destination;
    if(m_asset)
        delete m_asset;
}

PaymentOperation* PaymentOperation::build(stellar::PaymentOp &op)
{
    return new PaymentOperation(op);
}

PaymentOperation *PaymentOperation::create(KeyPair *destination, Asset *asset, QString amount)
{
    return new PaymentOperation(destination,asset,amount);
}

KeyPair *PaymentOperation::getDestination() {
    if(!m_destination)
    {
        //stellar::PublicKey pk = stellar::PublicKey::from(m_op.destination.ed25519);
        m_destination = KeyPair::fromXdrPublicKey(m_op.destination);
    }
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

PaymentOperation *PaymentOperation::setSourceAccount(KeyPair *sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}

PaymentOperation *PaymentOperation::setSourceAccount(KeyPair &sourceAccount)
{
    Operation::setSourceAccount(new KeyPair(sourceAccount));
    return this;
}
