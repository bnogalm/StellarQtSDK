#ifndef CLAWBACKOPERATION_H
#define CLAWBACKOPERATION_H

#include "operation.h"
#include "qstellar_namespace.h"
QSTELLAR_FWD(Asset)
/**
 *
 * Represents a Clawback operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
QSTELLAR_BEGIN_NS

class ClawbackOperation : public Operation
{
    stellar::ClawbackOp m_op;
    Asset* m_asset;
public:
    ClawbackOperation(QString from, Asset* asset, QString amount);
    ClawbackOperation(stellar::ClawbackOp& op);
    virtual ~ClawbackOperation();
    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);
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

    static ClawbackOperation* build(AccountConverter& accountConverter, stellar::ClawbackOp &op);
    static ClawbackOperation* create(QString from, Asset* asset, QString amount);
    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return ClawbackOperation object so you can chain methods.
         */
    ClawbackOperation* setSourceAccount(QString sourceAccount);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(ClawbackOperation)

#endif // CLAWBACKOPERATION_H
