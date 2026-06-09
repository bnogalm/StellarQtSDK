#ifndef BEGINSPONSORINGFUTURERESERVESOPERATION_H
#define BEGINSPONSORINGFUTURERESERVESOPERATION_H

#include "operation.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class BeginSponsoringFutureReservesOperation : public Operation
{    
    stellar::BeginSponsoringFutureReservesOp m_op;
public:
    BeginSponsoringFutureReservesOperation(QString sponsoredId);
    BeginSponsoringFutureReservesOperation(stellar::BeginSponsoringFutureReservesOp& op);
    QString getSponsoredID();
    void fillOperationBody(AccountConverter& accountConverter, stellar::Operation &operation);


    static BeginSponsoringFutureReservesOperation* build(stellar::BeginSponsoringFutureReservesOp &op);
    static BeginSponsoringFutureReservesOperation* create(QString sponsoredId);
    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return BeginSponsoringFutureReservesOperation object so you can chain methods.
         */
    BeginSponsoringFutureReservesOperation* setSourceAccount(QString sourceAccount);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(BeginSponsoringFutureReservesOperation)

#endif // BEGINSPONSORINGFUTURERESERVESOPERATION_H
