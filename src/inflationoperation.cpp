#include "inflationoperation.h"

InflationOperation::InflationOperation()
{

}

void InflationOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &operation)
{
    Q_UNUSED(accountConverter)
    operation.type =  stellar::OperationType::INFLATION;
}

InflationOperation *InflationOperation::create()
{
    return new InflationOperation();
}
