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
    QString m_destination;
public:

    AccountMergeOperation(QString destination);

    virtual ~AccountMergeOperation();
    /**
     * The account that receives the remaining XLM balance of the source account.
     */
    QString getDestination() const;
    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);

    /**
     * @brief build an AccountMergeOperation setting destination from an xdr operation
     * @param operationAccountMerge body of an xdr operation
     * @return AccountMergeOperation without source account setted
     */
    static AccountMergeOperation * build(stellar::MuxedAccount &operationAccountMerge);

    static AccountMergeOperation *create(KeyPair* destination);
    static AccountMergeOperation *create(QString destination);

    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return AccountMergeOperation object so you can chain methods.
         */
    AccountMergeOperation* setSourceAccount(QString sourceAccount);

};

#endif // ACCOUNTMERGEOPERATION_H






