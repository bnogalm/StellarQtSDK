#include "inflationoperation.h"

QSTELLAR_BEGIN_NS


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
QSTELLAR_END_NS
