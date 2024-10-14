#include "pathpaymentstrictsendoperation.h"


PathPaymentStrictSendOperation::PathPaymentStrictSendOperation()
:m_sendAsset(nullptr)
,m_destAsset(nullptr)
{

}

PathPaymentStrictSendOperation::PathPaymentStrictSendOperation(stellar::PathPaymentStrictSendOp &op)
    :m_sendAsset(nullptr)  
  ,m_destAsset(nullptr)
  ,m_op(op){

}

PathPaymentStrictSendOperation::~PathPaymentStrictSendOperation()
{
    if(m_sendAsset)
        delete m_sendAsset;
    if(m_destAsset)
        delete m_destAsset;
    for(auto a : m_path)
    {
        delete a;
    }
}

PathPaymentStrictSendOperation::PathPaymentStrictSendOperation(Asset *sendAsset, QString sendAmount, QString destination, Asset *destAsset, QString destMin, QList<Asset*> path)
    :m_sendAsset(nullptr)    
    ,m_destAsset(nullptr){
    checkNotNull(sendAsset, "sendAsset cannot be null");
    checkNotNull(sendAmount, "sendAmount cannot be null");    
    checkNotNull(destAsset, "destAsset cannot be null");
    checkNotNull(destMin, "destMin cannot be null");
    checkArgument(path.size() <= 5, "The maximum number of assets in the path is 5");

    m_op.sendAsset = sendAsset->toXdr();
    m_op.sendAmount = Operation::toXdrAmount(sendAmount);
    m_op.destination = StrKey::encodeToXDRMuxedAccount(checkNotNull(destination, "destination cannot be null"));

    m_op.destAsset = destAsset->toXdr();
    m_op.destMin = Operation::toXdrAmount(destMin);
    for(int i=0;i<path.size();i++){
        m_op.path.append(path[i]->toXdr());
    }
}

Asset *PathPaymentStrictSendOperation::getSendAsset() {
    if(!m_sendAsset)
        m_sendAsset = Asset::fromXdr(m_op.sendAsset);
    return m_sendAsset;
}

QString PathPaymentStrictSendOperation::getSendAmount() {
    return Operation::fromXdrAmount(m_op.sendAmount);
}

QString PathPaymentStrictSendOperation::getDestination() const{
    return StrKey::encodeStellarMuxedAccount(m_op.destination);
}

Asset *PathPaymentStrictSendOperation::getDestAsset() {
    if(!m_destAsset)
        m_destAsset = Asset::fromXdr(m_op.destAsset);
    return m_destAsset;
}

QString PathPaymentStrictSendOperation::getDestMin() {
    return Operation::fromXdrAmount(m_op.destMin);
}

QList<Asset *> PathPaymentStrictSendOperation::getPath() {
    if(m_path.isEmpty()){
        for (int i = 0; i < m_op.path.value.length(); i++) {
            m_path.append(Asset::fromXdr(m_op.path.value[i]));
        }
    }
    return m_path;
}

void PathPaymentStrictSendOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &op)
{
    auto& o = op.fillPathPaymentStrictSendOp();
    o = m_op;
    o.destination = accountConverter.filter(o.destination);
}

PathPaymentStrictSendOperation *PathPaymentStrictSendOperation::build(AccountConverter &accountConverter, stellar::PathPaymentStrictSendOp &op)
{
    auto result = new PathPaymentStrictSendOperation(op);
    result->m_op.destination = accountConverter.filter(op.destination);
    return result;
}

PathPaymentStrictSendOperation *PathPaymentStrictSendOperation::create(Asset* sendAsset, QString sendMax, KeyPair* destination,
                                                   Asset* destAsset, QString destAmount)
{
    return new PathPaymentStrictSendOperation(sendAsset,sendMax,destination->getAccountId(),destAsset,destAmount);
}

PathPaymentStrictSendOperation *PathPaymentStrictSendOperation::create(Asset *sendAsset, QString sendMax, QString destination, Asset *destAsset, QString destAmount)
{
    return new PathPaymentStrictSendOperation(sendAsset,sendMax,destination,destAsset,destAmount);
}

PathPaymentStrictSendOperation *PathPaymentStrictSendOperation::setPath(QList<Asset *> path) {
    checkArgument(path.length() <= 5, "The maximum number of assets in the path is 5");
    for(auto a : m_path)
    {
        delete a;
    }
    m_op.path.clear();
    for(int i=0;i<path.size();i++){
        m_op.path.append(path[i]->toXdr());
    }
    return this;
}

PathPaymentStrictSendOperation *PathPaymentStrictSendOperation::setSourceAccount(QString sourceAccount) {
    Operation::setSourceAccount(sourceAccount);
    return this;
}

