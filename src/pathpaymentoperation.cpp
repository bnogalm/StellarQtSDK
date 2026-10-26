#include "pathpaymentoperation.h"
#include "keypair.h"

QSTELLAR_BEGIN_NS

PathPaymentOperation::PathPaymentOperation()
{
}

PathPaymentOperation::PathPaymentOperation(stellar::PathPaymentStrictReceiveOp &op)
    : PathPaymentStrictReceiveOperation(op)
{
}

PathPaymentOperation::~PathPaymentOperation()
{
}

PathPaymentOperation::PathPaymentOperation(Asset *sendAsset, QString sendMax, QString destination,
                                           Asset *destAsset, QString destAmount, QList<Asset*> path)
    : PathPaymentStrictReceiveOperation(sendAsset, sendMax, destination, destAsset, destAmount, path)
{
}

PathPaymentOperation *PathPaymentOperation::build(AccountConverter &accountConverter,
                                                  stellar::PathPaymentStrictReceiveOp &op)
{
    stellar::PathPaymentStrictReceiveOp filtered = op;
    filtered.destination = accountConverter.filter(op.destination);
    return new PathPaymentOperation(filtered);
}

PathPaymentOperation *PathPaymentOperation::create(Asset *sendAsset, QString sendMax, KeyPair *destination,
                                                   Asset *destAsset, QString destAmount)
{
    return new PathPaymentOperation(sendAsset, sendMax, destination->getAccountId(), destAsset, destAmount);
}

PathPaymentOperation *PathPaymentOperation::create(Asset *sendAsset, QString sendMax, QString destination,
                                                   Asset *destAsset, QString destAmount)
{
    return new PathPaymentOperation(sendAsset, sendMax, destination, destAsset, destAmount);
}

PathPaymentOperation *PathPaymentOperation::setPath(QList<Asset *> path)
{
    PathPaymentStrictReceiveOperation::setPath(path);
    return this;
}

PathPaymentOperation *PathPaymentOperation::setSourceAccount(QString sourceAccount)
{
    PathPaymentStrictReceiveOperation::setSourceAccount(sourceAccount);
    return this;
}

QSTELLAR_END_NS
