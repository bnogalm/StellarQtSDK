#ifndef ENDSPONSORINGFUTURERESERVESOPERATION_H
#define ENDSPONSORINGFUTURERESERVESOPERATION_H

#include "operation.h"

class EndSponsoringFutureReservesOperation : public Operation
{    
public:
    EndSponsoringFutureReservesOperation();
    void fillOperationBody(stellar::Operation &operation)
    {
        operation.type = stellar::OperationType::END_SPONSORING_FUTURE_RESERVES;
    }
    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return CreatePassiveSellOfferOperation object so you can chain methods.
         */
    EndSponsoringFutureReservesOperation* setSourceAccount(QString sourceAccount)
    {
        Operation::setSourceAccount(sourceAccount);
        return this;
    }
};

#endif // ENDSPONSORINGFUTURERESERVESOPERATION_H
