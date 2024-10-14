#include "endsponsoringfuturereservesoperation.h"

EndSponsoringFutureReservesOperation::EndSponsoringFutureReservesOperation()
{

}

void EndSponsoringFutureReservesOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &operation)
{
    Q_UNUSED(accountConverter)
    operation.type = stellar::OperationType::END_SPONSORING_FUTURE_RESERVES;
}

EndSponsoringFutureReservesOperation *EndSponsoringFutureReservesOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}

EndSponsoringFutureReservesOperation *EndSponsoringFutureReservesOperation::create()
{
    return new EndSponsoringFutureReservesOperation();
}

EndSponsoringFutureReservesOperation *EndSponsoringFutureReservesOperation::build()
{
    return new EndSponsoringFutureReservesOperation();
}
