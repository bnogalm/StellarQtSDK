#ifndef CHANGETRUSTOPERATION_H
#define CHANGETRUSTOPERATION_H
#include "operation.h"
#include "qstellar_namespace.h"
QSTELLAR_FWD(Asset)
QSTELLAR_FWD(ChangeTrustAsset)

/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#change-trust" target="_blank">ChangeTrust</a>
 * operation. CAP-38 extends the underlying XDR with a `LIQUIDITY_POOL_SHARE`
 * variant — construct via the `ChangeTrustAsset*` ctor for that case.
 *
 * Neither `asset` nor `line` arguments are owned; the operation captures XDR.
 */
QSTELLAR_BEGIN_NS

class ChangeTrustOperation : public Operation
{
    Asset* m_asset;                 // lazy cache for getAsset() (non-LP only)
    ChangeTrustAsset* m_lineCache;  // lazy cache for getLine()
    stellar::ChangeTrustOp m_op;

public:
    /** Legacy (non-LP) ctor. `asset` is not owned. */
    ChangeTrustOperation(Asset* asset, QString limit);

    /** CAP-38 ctor — accepts both regular assets and pool shares. `line` is not owned. */
    ChangeTrustOperation(ChangeTrustAsset* line, QString limit);

    ChangeTrustOperation(stellar::ChangeTrustOp& op);
    virtual ~ChangeTrustOperation();

    /**
     * The asset of the trustline. Returns nullptr for the POOL_SHARE variant —
     * use `getLine()` instead in that case.
     */
    Asset* getAsset();

    /** ChangeTrustAsset wrapper (covers all 4 variants). Lazy-built. */
    ChangeTrustAsset* getLine();

    /**
     * The limit of the trustline. For example, if a gateway extends a trustline of up to 200 USD to a user, the limit is 200.
     */
    QString getLimit();
    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);
    static ChangeTrustOperation* build(stellar::ChangeTrustOp &op);
    static ChangeTrustOperation* create(Asset* asset, QString limit);
    static ChangeTrustOperation* create(ChangeTrustAsset* line, QString limit);

    ChangeTrustOperation* setSourceAccount(QString sourceAccount);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(ChangeTrustOperation)

#endif // CHANGETRUSTOPERATION_H
