#ifndef CREATECLAIMABLEBALANCEOPERATION_H
#define CREATECLAIMABLEBALANCEOPERATION_H

#include "operation.h"
#include "asset.h"
#include "claimant.h"

class CreateClaimableBalanceOperation : public Operation
{
    stellar::CreateClaimableBalanceOp m_op;
    Asset* m_asset;
public:
    CreateClaimableBalanceOperation(QString amount, Asset* asset, QList<Claimant> claimants);
    CreateClaimableBalanceOperation(stellar::CreateClaimableBalanceOp& op);
    virtual ~CreateClaimableBalanceOperation();
    Asset* getAsset();
    QString getAmount();
    QList<Claimant> getClaimants();



    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);
    static CreateClaimableBalanceOperation* build(stellar::CreateClaimableBalanceOp &op);
    /**
     * Creates a new CreateClaimableBalance builder.
     * @param amount The amount which can be claimed.
     * @param asset The asset which can be claimed/
     * @param claimants The list of entities which can claim the balance.
     */
    static CreateClaimableBalanceOperation* create(QString amount, Asset* asset, QList<Claimant> claimants);
    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return CreateClaimableBalanceOperation object so you can chain methods.
         */
    CreateClaimableBalanceOperation* setSourceAccount(QString sourceAccount);
};

#endif // CREATECLAIMABLEBALANCEOPERATION_H
