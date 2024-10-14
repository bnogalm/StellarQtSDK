#ifndef CREATEACCOUNTOPERATION_H
#define CREATEACCOUNTOPERATION_H
#include "operation.h"

/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#create-account" target="_blank">CreateAccount</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
class CreateAccountOperation : public Operation
{
    KeyPair * m_destination;
    stellar::CreateAccountOp m_op;

public:
    CreateAccountOperation(KeyPair* destination, QString startingBalance);
    CreateAccountOperation(QString destination, QString startingBalance);
    CreateAccountOperation(stellar::CreateAccountOp &op);
    virtual ~CreateAccountOperation();

    static CreateAccountOperation* build(stellar::CreateAccountOp& op);
    static CreateAccountOperation* create(KeyPair* destination, QString startingBalance);

    /**
     * Amount of XLM to send to the newly created account.
     */
    QString getStartingBalance();

    /**
     * Account that is created and funded
     */
    KeyPair* getDestination();

    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);
    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return CreateAccountOperation object so you can chain methods.
         */
    CreateAccountOperation* setSourceAccount(QString sourceAccount);


};

#endif // CREATEACCOUNTOPERATION_H
