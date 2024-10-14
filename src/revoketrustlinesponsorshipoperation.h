#ifndef REVOKETRUSTLINESPONSORSHIPOPERATION_H
#define REVOKETRUSTLINESPONSORSHIPOPERATION_H

#include "operation.h"
#include <QObject>
#include "asset.h"

class RevokeTrustlineSponsorshipOperation : public Operation
{
    Asset* m_asset;
    stellar::RevokeSponsorshipOp m_op;
public:
    RevokeTrustlineSponsorshipOperation(QString accountId, Asset* asset);
    RevokeTrustlineSponsorshipOperation(stellar::RevokeSponsorshipOp& op);
    virtual ~RevokeTrustlineSponsorshipOperation();
    QString getAccountId() const;

    Asset* getAsset();

    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &op);
    /**
     * Construct a new RevokeTrustlineSponsorshipOperation builder from a RevokeSponsorship XDR.
     * @param op {@link RevokeSponsorshipOp}
     */
    static RevokeTrustlineSponsorshipOperation *build(stellar::RevokeSponsorshipOp &op);
    /**
     * Creates a new RevokeTrustlineSponsorshipOperation builder.
     * @param accountId The id of the account whose trustline will be revoked.
     * @param asset The asset of the trustline which will be revoked.
     */
    static RevokeTrustlineSponsorshipOperation *create(QString accountId, Asset* asset);
    /**
     * Sets the source account for this operation.
     * @param sourceAccount The operation's source account.
     * @return Builder object so you can chain methods.
     */
    RevokeTrustlineSponsorshipOperation *setSourceAccount(QString sourceAccount);
};

#endif // REVOKETRUSTLINESPONSORSHIPOPERATION_H
