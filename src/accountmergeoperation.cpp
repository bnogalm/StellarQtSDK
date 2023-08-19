#include "accountmergeoperation.h"



AccountMergeOperation::AccountMergeOperation(QString destination)
{
    m_destination = checkNotNull(destination, "destination cannot be null");
}

AccountMergeOperation::~AccountMergeOperation(){
}

QString AccountMergeOperation::getDestination() const {
    return m_destination;
}

void AccountMergeOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &operation) {
    operation.type = stellar::OperationType::ACCOUNT_MERGE;
    operation.operationAccountMerge = accountConverter.encode(m_destination);
}

AccountMergeOperation *AccountMergeOperation::build(stellar::MuxedAccount &operationAccountMerge)
{
    return new AccountMergeOperation(StrKey::encodeStellarMuxedAccount(operationAccountMerge));
}

AccountMergeOperation *AccountMergeOperation::create(KeyPair* destination)
{
    return new AccountMergeOperation(destination->getAccountId());
}

AccountMergeOperation *AccountMergeOperation::create(QString destination)
{
    return new AccountMergeOperation(destination);
}

AccountMergeOperation *AccountMergeOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
