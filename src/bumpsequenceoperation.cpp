#include "bumpsequenceoperation.h"

BumpSequenceOperation::BumpSequenceOperation(qint64 bumpTo)
{
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
