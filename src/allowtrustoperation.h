#ifndef ALLOWTRUSTOPERATION_H
#define ALLOWTRUSTOPERATION_H
#include "operation.h"


/**
 * Represents <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html#allow-trust" target="_blank">AllowTrust</a> operation.
 * @see <a href="https://www.stellar.org/developers/learn/concepts/list-of-operations.html" target="_blank">List of Operations</a>
 */
class AllowTrustOperation : public Operation
{
    KeyPair* m_trustor;

    stellar::AllowTrustOp m_op;

public:


    /**
     * Creates a new AllowTrust op.
     * @param trustor The account of the recipient of the trustline.
     * @param assetCode The asset of the trustline the source account is authorizing. For example, if a gateway wants to allow another account to hold its USD credit, the type is USD.
     * @param authorize Flag indicating whether the trustline is authorized.
     */
    AllowTrustOperation(KeyPair* trustor, QString assetCode, bool authorize);

    AllowTrustOperation(stellar::AllowTrustOp& op);
    virtual ~AllowTrustOperation();

    /**
     * The account of the recipient of the trustline.
     */
    KeyPair* getTrustor();
    /**
     * The asset of the trustline the source account is authorizing. For example, if a gateway wants to allow another account to hold its USD credit, the type is USD.
     */
    QString getAssetCode();

    /**
     * Flag indicating whether the trustline is authorized.
     */
    bool getAuthorize();


    void fillOperationBody(stellar::Operation &operation);


    static AllowTrustOperation* build(stellar::AllowTrustOp &op);
    static AllowTrustOperation* create(KeyPair* trustor, QString assetCode, bool authorize);

    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return AllowTrustOperation object so you can chain methods.
         */
    AllowTrustOperation* setSourceAccount(KeyPair* sourceAccount);

    /**
         * Sets the source account for this operation making a copy of keypair
         * @param sourceAccount The operation's source account.
         * @return AllowTrustOperation object so you can chain methods.
         */
    AllowTrustOperation* setSourceAccount(KeyPair& sourceAccount);

};


#endif // ALLOWTRUSTOPERATION_H
