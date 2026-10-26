#include "extendfootprintttloperation.h"
#include <new>

QSTELLAR_BEGIN_NS


ExtendFootprintTTLOperation::ExtendFootprintTTLOperation(quint32 extendTo)
    : m_extendTo(extendTo)
{
}

ExtendFootprintTTLOperation::ExtendFootprintTTLOperation(const stellar::ExtendFootprintTTLOp& op)
    : m_extendTo(op.extendTo)
{
}

void ExtendFootprintTTLOperation::fillOperationBody(AccountConverter& accountConverter, stellar::Operation& operation)
{
    Q_UNUSED(accountConverter)
    operation.type = stellar::OperationType::EXTEND_FOOTPRINT_TTL;
    // The body lives in a union whose members are not constructed for us, so
    // without this the ExtensionPoint's four bytes were whatever the memory
    // held: an invalid ext on the wire and a hash that changed run to run.
    new (&operation.operationExtendFootprintTtl) stellar::ExtendFootprintTTLOp();
    operation.operationExtendFootprintTtl.extendTo = m_extendTo;
}

ExtendFootprintTTLOperation* ExtendFootprintTTLOperation::build(const stellar::ExtendFootprintTTLOp& op)
{
    return new ExtendFootprintTTLOperation(op);
}

ExtendFootprintTTLOperation* ExtendFootprintTTLOperation::create(quint32 extendTo)
{
    return new ExtendFootprintTTLOperation(extendTo);
}

ExtendFootprintTTLOperation* ExtendFootprintTTLOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
QSTELLAR_END_NS
