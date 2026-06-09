#include "pathpaymentstrictsendoperationresponse.h"

QSTELLAR_BEGIN_NS


PathPaymentStrictSendOperationResponse::PathPaymentStrictSendOperationResponse(QNetworkReply *reply)
    :PathPaymentBaseOperationResponse(reply)
{

}

PathPaymentStrictSendOperationResponse::~PathPaymentStrictSendOperationResponse()
{

}

QString PathPaymentStrictSendOperationResponse::getDestinationMin() const
{
    return m_destinationMin;
}
QSTELLAR_END_NS
