#include "beginsponsoringfuturereservesoperation.h"


BeginSponsoringFutureReservesOperation::BeginSponsoringFutureReservesOperation(QString sponsoredId)
{
    checkNotNull(sponsoredId, "sponsoredId cannot be null");
    m_op.sponsoredID=StrKey::encodeToXDRAccountId(sponsoredId);
}

BeginSponsoringFutureReservesOperation::BeginSponsoringFutureReservesOperation(stellar::BeginSponsoringFutureReservesOp &op):m_op(op)
{
}

QString BeginSponsoringFutureReservesOperation::getSponsoredID()
{
    return StrKey::encodeStellarAccountId(m_op.sponsoredID);
}

void BeginSponsoringFutureReservesOperation::fillOperationBody(stellar::Operation &operation)
{
    operation.type = stellar::OperationType::BEGIN_SPONSORING_FUTURE_RESERVES;
    operation.operationBeginSponsoringFutureReserves=m_op;
}

BeginSponsoringFutureReservesOperation *BeginSponsoringFutureReservesOperation::build(stellar::BeginSponsoringFutureReservesOp &op)
{
    return new BeginSponsoringFutureReservesOperation(op);
}

BeginSponsoringFutureReservesOperation *BeginSponsoringFutureReservesOperation::create(QString sponsoredId)
{
    return new BeginSponsoringFutureReservesOperation(sponsoredId);
}

BeginSponsoringFutureReservesOperation *BeginSponsoringFutureReservesOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
