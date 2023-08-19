#include "createaccountoperation.h"


CreateAccountOperation::CreateAccountOperation(KeyPair *destination, QString startingBalance)
    :m_destination(nullptr)
{
    checkNotNull(destination, "destination cannot be null");
    checkNotNull(startingBalance, "startingBalance cannot be null");

    m_op.destination = destination->getXdrPublicKey();
    m_op.startingBalance = Operation::toXdrAmount(startingBalance);

}
CreateAccountOperation::CreateAccountOperation(QString destination, QString startingBalance)
    :m_destination(nullptr)
{
    checkNotNull(destination, "destination cannot be null");
    checkNotNull(startingBalance, "startingBalance cannot be null");

    m_op.destination = StrKey::encodeToXDRAccountId(destination);
    m_op.startingBalance = Operation::toXdrAmount(startingBalance);

}

CreateAccountOperation::CreateAccountOperation(stellar::CreateAccountOp &op):m_destination(nullptr),m_op(op)
{

}

CreateAccountOperation::~CreateAccountOperation(){
    if(m_destination)
        delete m_destination;
}

CreateAccountOperation* CreateAccountOperation::build(stellar::CreateAccountOp &op)
{
    return new CreateAccountOperation(op);
}

CreateAccountOperation *CreateAccountOperation::create(KeyPair* destination, QString startingBalance)
{
    return new CreateAccountOperation(destination, startingBalance);
}

QString CreateAccountOperation::getStartingBalance() {
    return Operation::fromXdrAmount(m_op.startingBalance);
}

KeyPair *CreateAccountOperation::getDestination() {
    if(!m_destination){
        m_destination = KeyPair::fromXdrPublicKey(m_op.destination);
    }
    return m_destination;
}

void CreateAccountOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &operation)
{
    Q_UNUSED(accountConverter)
    operation.type = stellar::OperationType::CREATE_ACCOUNT;
    operation.operationCreateAccount = m_op;
}

CreateAccountOperation *CreateAccountOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}
