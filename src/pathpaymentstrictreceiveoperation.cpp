#include "pathpaymentstrictreceiveoperation.h"


PathPaymentStrictReceiveOperation::PathPaymentStrictReceiveOperation()
:m_sendAsset(nullptr)
,m_destAsset(nullptr)
{

}

PathPaymentStrictReceiveOperation::PathPaymentStrictReceiveOperation(stellar::PathPaymentStrictReceiveOp &op)
    :m_sendAsset(nullptr)  
  ,m_destAsset(nullptr)
  ,m_op(op){

}

PathPaymentStrictReceiveOperation::~PathPaymentStrictReceiveOperation()
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

PathPaymentStrictReceiveOperation::PathPaymentStrictReceiveOperation(Asset *sendAsset, QString sendMax, QString destination, Asset *destAsset, QString destAmount, QList<Asset*> path)
    :m_sendAsset(nullptr)    
    ,m_destAsset(nullptr){
    checkNotNull(sendAsset, "sendAsset cannot be null");
    checkNotNull(sendMax, "sendMax cannot be null");    
    checkNotNull(destAsset, "destAsset cannot be null");
    checkNotNull(destAmount, "destAmount cannot be null");
    checkArgument(path.size() <= 5, "The maximum number of assets in the path is 5");

    m_op.sendAsset = sendAsset->toXdr();
    m_op.sendMax = Operation::toXdrAmount(sendMax);    
    m_op.destination = StrKey::encodeToXDRMuxedAccount(checkNotNull(destination, "destination cannot be null"));

    m_op.destAsset = destAsset->toXdr();
    m_op.destAmount = Operation::toXdrAmount(destAmount);
    for(int i=0;i<path.size();i++){
        m_op.path.append(path[i]->toXdr());
    }
}

Asset *PathPaymentStrictReceiveOperation::getSendAsset() {
    if(!m_sendAsset)
        m_sendAsset = Asset::fromXdr(m_op.sendAsset);
    return m_sendAsset;
}

QString PathPaymentStrictReceiveOperation::getSendMax() {
    return Operation::fromXdrAmount(m_op.sendMax);
}

QString PathPaymentStrictReceiveOperation::getDestination() const{
    return StrKey::encodeStellarAccountId(StrKey::muxedAccountToAccountId(m_op.destination));
}

Asset *PathPaymentStrictReceiveOperation::getDestAsset() {
    if(!m_destAsset)
        m_destAsset = Asset::fromXdr(m_op.destAsset);
    return m_destAsset;
}

QString PathPaymentStrictReceiveOperation::getDestAmount() {
    return Operation::fromXdrAmount(m_op.destAmount);
}

QList<Asset *> PathPaymentStrictReceiveOperation::getPath() {
    if(m_path.isEmpty()){
        for (int i = 0; i < m_op.path.value.length(); i++) {
            m_path.append(Asset::fromXdr(m_op.path.value[i]));
        }
    }
    return m_path;
}

void PathPaymentStrictReceiveOperation::fillOperationBody(stellar::Operation &op)
{
    auto& o = op.fillPathPaymentStrictReceiveOp();
    o = m_op;
}

PathPaymentStrictReceiveOperation *PathPaymentStrictReceiveOperation::build(stellar::PathPaymentStrictReceiveOp &op)
{
    return new PathPaymentStrictReceiveOperation(op);
}

PathPaymentStrictReceiveOperation *PathPaymentStrictReceiveOperation::create(Asset* sendAsset, QString sendMax, KeyPair* destination,
                                                   Asset* destAsset, QString destAmount)
{
    return new PathPaymentStrictReceiveOperation(sendAsset,sendMax,destination->getAccountId(),destAsset,destAmount);
}

PathPaymentStrictReceiveOperation *PathPaymentStrictReceiveOperation::create(Asset *sendAsset, QString sendMax, QString destination, Asset *destAsset, QString destAmount)
{
    return new PathPaymentStrictReceiveOperation(sendAsset,sendMax,destination,destAsset,destAmount);
}

PathPaymentStrictReceiveOperation *PathPaymentStrictReceiveOperation::setPath(QList<Asset *> path) {
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

PathPaymentStrictReceiveOperation *PathPaymentStrictReceiveOperation::setSourceAccount(QString sourceAccount) {
    Operation::setSourceAccount(sourceAccount);
    return this;
}
