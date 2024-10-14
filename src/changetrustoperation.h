#ifndef CHANGETRUSTOPERATION_H
#define CHANGETRUSTOPERATION_H
#include "operation.h"
class Asset;
/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#change-trust" target="_blank">ChangeTrust</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
class ChangeTrustOperation : public Operation
{
    Asset* m_asset;
    stellar::ChangeTrustOp m_op;

public:
    ChangeTrustOperation(Asset* asset, QString limit);
    ChangeTrustOperation(stellar::ChangeTrustOp& op);
    virtual ~ChangeTrustOperation();

    /**
     * The asset of the trustline. For example, if a gateway extends a trustline of up to 200 USD to a user, the line is USD.
     */
    Asset* getAsset();

    /**
     * The limit of the trustline. For example, if a gateway extends a trustline of up to 200 USD to a user, the limit is 200.
     */
    QString getLimit();
    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);
    static ChangeTrustOperation* build(stellar::ChangeTrustOp &op);
    static ChangeTrustOperation* create(Asset* asset, QString limit);

    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return ChangeTrustOperation object so you can chain methods.
         */
    ChangeTrustOperation* setSourceAccount(QString sourceAccount);

};

#endif // CHANGETRUSTOPERATION_H
