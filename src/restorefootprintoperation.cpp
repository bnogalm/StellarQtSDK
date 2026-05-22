#include "restorefootprintoperation.h"

RestoreFootprintOperation::RestoreFootprintOperation(const stellar::RestoreFootprintOp&)
{
}

void RestoreFootprintOperation::fillOperationBody(AccountConverter& accountConverter, stellar::Operation& operation)
{
    Q_UNUSED(accountConverter)
    operation.type = stellar::OperationType::RESTORE_FOOTPRINT;
    // No fields beyond the ext member; default-init.
}

RestoreFootprintOperation* RestoreFootprintOperation::build(const stellar::RestoreFootprintOp& op)
{
    return new RestoreFootprintOperation(op);
}

RestoreFootprintOperation* RestoreFootprintOperation::create()
{
    return new RestoreFootprintOperation();
}

RestoreFootprintOperation* RestoreFootprintOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
