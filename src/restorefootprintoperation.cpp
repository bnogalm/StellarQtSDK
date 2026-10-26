#include "restorefootprintoperation.h"
#include <new>

QSTELLAR_BEGIN_NS


RestoreFootprintOperation::RestoreFootprintOperation(const stellar::RestoreFootprintOp&)
{
}

void RestoreFootprintOperation::fillOperationBody(AccountConverter& accountConverter, stellar::Operation& operation)
{
    Q_UNUSED(accountConverter)
    operation.type = stellar::OperationType::RESTORE_FOOTPRINT;
    // No fields beyond the ext member — which still has to be constructed:
    // the union does not do it, and its four bytes went out uninitialised.
    new (&operation.operationRestoreFootprint) stellar::RestoreFootprintOp();
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
QSTELLAR_END_NS
