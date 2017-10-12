#ifndef ACCOUNTMERGEOPERATION_H
#define ACCOUNTMERGEOPERATION_H
#include "operation.h"
#include "xdr/stellartransaction.h"
/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#account-merge" target="_blank">AccountMerge</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */

class AccountMergeOperation : public Operation
{
    KeyPair *m_destination;
    stellar::AccountID m_op;
public:

    AccountMergeOperation(KeyPair* destination);
    AccountMergeOperation(stellar::AccountID& op);
    virtual ~AccountMergeOperation();
    /**
     * The account that receives the remaining XLM balance of the source account.
     */
    KeyPair* getDestination();
    void fillOperationBody(stellar::Operation &operation);

    /**
     * @brief build an AccountMergeOperation setting destination from an xdr operation
     * @param operationAccountMerge body of an xdr operation
     * @return AccountMergeOperation without source account setted
     */
    static AccountMergeOperation * build(stellar::AccountID &operationAccountMerge);

    static AccountMergeOperation *create(KeyPair* destination);

    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return AccountMergeOperation object so you can chain methods.
         */
    AccountMergeOperation* setSourceAccount(KeyPair* sourceAccount);

    /**
         * Sets the source account for this operation making a copy of keypair
         * @param sourceAccount The operation's source account.
         * @return AccountMergeOperation object so you can chain methods.
         */
    AccountMergeOperation* setSourceAccount(KeyPair& sourceAccount);
};

#endif // ACCOUNTMERGEOPERATION_H






