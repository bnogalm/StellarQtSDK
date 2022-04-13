#ifndef CLAWBACKOPERATION_H
#define CLAWBACKOPERATION_H

#include "operation.h"
/**
 *
 * Represents a Clawback operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
class ClawbackOperation : public Operation
{
    stellar::ClawbackOp m_op;
    Asset* m_asset;
public:
    ClawbackOperation(QString from, Asset* asset, QString amount);
    ClawbackOperation(stellar::ClawbackOp& op);
    ~ClawbackOperation();
    void fillOperationBody(stellar::Operation &operation);
    /**
     * The account owning of the trustline.
     */
    QString getFrom() const;

    /**
     * The amount to be clawed back.
     */
    QString getAmount() const;

    /**
     * The asset to be clawed back.
     */
    Asset* getAsset();

    static ClawbackOperation* build(stellar::ClawbackOp &op);
    static ClawbackOperation* create(QString from, Asset* asset, QString amount);
    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return ClawbackOperation object so you can chain methods.
         */
    ClawbackOperation* setSourceAccount(QString sourceAccount);
};

#endif // CLAWBACKOPERATION_H
