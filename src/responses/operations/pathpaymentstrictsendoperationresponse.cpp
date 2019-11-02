#include "pathpaymentstrictsendoperationresponse.h"

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
