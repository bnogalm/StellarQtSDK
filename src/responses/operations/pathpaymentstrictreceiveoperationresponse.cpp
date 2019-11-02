#include "pathpaymentstrictreceiveoperationresponse.h"


PathPaymentStrictReceiveOperationResponse::PathPaymentStrictReceiveOperationResponse(QNetworkReply *reply):PathPaymentBaseOperationResponse(reply)
{

}

PathPaymentStrictReceiveOperationResponse::~PathPaymentStrictReceiveOperationResponse()
{

}

QString PathPaymentStrictReceiveOperationResponse::getSourceMax() const{
    return m_sourceMax;
}
