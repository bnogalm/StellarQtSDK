#ifndef ENDSPONSORINGFUTURERESERVESOPERATION_H
#define ENDSPONSORINGFUTURERESERVESOPERATION_H

#include "operation.h"

class EndSponsoringFutureReservesOperation : public Operation
{    
public:
    EndSponsoringFutureReservesOperation();
    void fillOperationBody(stellar::Operation &operation);
    /**
         * Sets the source account for this operation.
         * @param sourceAccount The operation's source account.
         * @return CreatePassiveSellOfferOperation object so you can chain methods.
         */
    EndSponsoringFutureReservesOperation* setSourceAccount(QString sourceAccount);
    static EndSponsoringFutureReservesOperation* create();
    /**
    * Construct a new EndSponsoringFutureReservesOperation, just to keep API equal to others
    * @param op
    */
    static EndSponsoringFutureReservesOperation* build();
};

#endif // ENDSPONSORINGFUTURERESERVESOPERATION_H
