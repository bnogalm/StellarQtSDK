#ifndef CREATEACCOUNTOPERATION_H
#define CREATEACCOUNTOPERATION_H
#include "operation.h"
#include "qstellar_namespace.h"

/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#create-account" target="_blank">CreateAccount</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
QSTELLAR_BEGIN_NS

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
     * Same, with the destination as a G-address. The constructor already
     * accepted a QString; only the factory was missing, which forced callers
     * that hold text (a QML front end, a parser) to build a throwaway KeyPair
     * just to ask. PaymentOperation::create already offers both.
     */
    static CreateAccountOperation* create(QString destination, QString startingBalance);

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(CreateAccountOperation)

#endif // CREATEACCOUNTOPERATION_H
