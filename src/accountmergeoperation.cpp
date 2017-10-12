#include "accountmergeoperation.h"



AccountMergeOperation::AccountMergeOperation(KeyPair *destination)
    :m_destination(nullptr)
{
    checkNotNull(destination, "destination cannot be null");

    m_op= destination->getXdrPublicKey();
}

AccountMergeOperation::AccountMergeOperation(stellar::AccountID &op)
    :m_destination(nullptr),m_op(op)
{

}

AccountMergeOperation::~AccountMergeOperation(){
    if(m_destination)
    {
        delete m_destination;
    }
}

KeyPair *AccountMergeOperation::getDestination() {
    if(!m_destination){
        m_destination = KeyPair::fromXdrPublicKey(m_op);
    }
    return m_destination;
}

void AccountMergeOperation::fillOperationBody(stellar::Operation &operation) {
    operation.type = stellar::OperationType::ACCOUNT_MERGE;
    operation.operationAccountMerge = m_op;
}

AccountMergeOperation *AccountMergeOperation::build(stellar::AccountID &operationAccountMerge)
{
    return new AccountMergeOperation(operationAccountMerge);
}

AccountMergeOperation *AccountMergeOperation::create(KeyPair* destination)
{
    return new AccountMergeOperation(destination);
}

AccountMergeOperation *AccountMergeOperation::setSourceAccount(KeyPair *sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}

AccountMergeOperation *AccountMergeOperation::setSourceAccount(KeyPair &sourceAccount)
{
    Operation::setSourceAccount(new KeyPair(sourceAccount));
    return this;
}
