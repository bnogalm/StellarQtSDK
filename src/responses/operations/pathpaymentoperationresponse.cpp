#include "pathpaymentoperationresponse.h"
#include "../../keypair.h"
#include "../../asset.h"
#include "../../assettypenative.h"
PathPaymentOperationResponse::PathPaymentOperationResponse(QNetworkReply *reply)
    : PathPaymentStrictReceiveOperationResponse(reply)
{

}

