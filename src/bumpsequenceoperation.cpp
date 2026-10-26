#include "bumpsequenceoperation.h"
#include <stdexcept>

QSTELLAR_BEGIN_NS


BumpSequenceOperation::BumpSequenceOperation(qint64 bumpTo)
{
    // stellar-core answers BUMP_SEQUENCE_BAD_SEQ to a negative bumpTo: it was
    // built and signed here, and the fee paid for nothing. The QML layer
    // already refused it; decoding an existing operation stays as it is.
    if (bumpTo < 0)
        throw std::runtime_error("bumpTo must not be negative");
    m_op.bumpTo=bumpTo;
}

BumpSequenceOperation::BumpSequenceOperation(stellar::BumpSequenceOp& op)
    :m_op(op)
{

}

BumpSequenceOperation::~BumpSequenceOperation()
{

}

qint64 BumpSequenceOperation::getBumpTo() const
{
    return m_op.bumpTo;
}

void BumpSequenceOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &operation)
{
    Q_UNUSED(accountConverter)
    operation.type = stellar::OperationType::BUMP_SEQUENCE;
    operation.operationBumpSequence=m_op;
}

BumpSequenceOperation *BumpSequenceOperation::build(stellar::BumpSequenceOp &op)
{
    return new BumpSequenceOperation(op);
}

BumpSequenceOperation *BumpSequenceOperation::create(qint64 bumpTo)
{
    return new BumpSequenceOperation(bumpTo);
}

BumpSequenceOperation *BumpSequenceOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
QSTELLAR_END_NS
