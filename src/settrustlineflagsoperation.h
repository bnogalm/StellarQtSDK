#ifndef SETTRUSTLINEFLAGSOPERATION_H
#define SETTRUSTLINEFLAGSOPERATION_H

#include "operation.h"
/**
 *
 * Represents a Set Trustline Flags operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
class SetTrustlineFlagsOperation : public Operation
{
    stellar::SetTrustLineFlagsOp m_op;
    Asset* m_asset;
public:
    SetTrustlineFlagsOperation(stellar::SetTrustLineFlagsOp& op);
    SetTrustlineFlagsOperation(QString trustor, Asset* asset, QFlags<stellar::TrustLineFlags> clearFlags, QFlags<stellar::TrustLineFlags> setFlags);

    ~SetTrustlineFlagsOperation();

    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);
    /**
     * The account owning of the trustline.
     */
    QString getTrustor() {
      return StrKey::encodeStellarAccountId(m_op.trustor);
    }

    /**
     * The asset of the trustline.
     */
    Asset* getAsset();

    /**
     * The flags to be set.
     */
    QFlags<stellar::TrustLineFlags> getSetFlags();

    /**
     * The flags to be cleared.
     */
    QFlags<stellar::TrustLineFlags> getClearFlags();



    static SetTrustlineFlagsOperation* build(stellar::SetTrustLineFlagsOp &op);
    static SetTrustlineFlagsOperation* create(QString trustor, Asset* asset, QFlags<stellar::TrustLineFlags> clearFlags, QFlags<stellar::TrustLineFlags> setFlags);
    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return SetTrustlineFlagsOperation object so you can chain methods.
         */
    SetTrustlineFlagsOperation* setSourceAccount(QString sourceAccount);
};

#endif // SETTRUSTLINEFLAGSOPERATION_H
