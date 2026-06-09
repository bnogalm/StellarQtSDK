#include "pathpaymentoperationresponse.h"
#include "../../keypair.h"
#include "../../asset.h"
#include "../../assettypenative.h"

QSTELLAR_BEGIN_NS

PathPaymentOperationResponse::PathPaymentOperationResponse(QNetworkReply *reply)
    : PathPaymentStrictReceiveOperationResponse(reply)
{

}

QSTELLAR_END_NS
