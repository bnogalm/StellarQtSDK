#include "inflationoperation.h"

InflationOperation::InflationOperation()
{

}

void InflationOperation::fillOperationBody(stellar::Operation &operation)
{
    operation.type =  stellar::OperationType::INFLATION;
}

InflationOperation *InflationOperation::create()
{
    return new InflationOperation();
}
