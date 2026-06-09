#include "pathpaymentstrictreceiveoperationresponse.h"

QSTELLAR_BEGIN_NS



PathPaymentStrictReceiveOperationResponse::PathPaymentStrictReceiveOperationResponse(QNetworkReply *reply):PathPaymentBaseOperationResponse(reply)
{

}

PathPaymentStrictReceiveOperationResponse::~PathPaymentStrictReceiveOperationResponse()
{

}

QString PathPaymentStrictReceiveOperationResponse::getSourceMax() const{
    return m_sourceMax;
}
QSTELLAR_END_NS
