#include "endsponsoringfuturereservesoperationresponse.h"

QSTELLAR_BEGIN_NS


EndSponsoringFutureReservesOperationResponse::EndSponsoringFutureReservesOperationResponse(QNetworkReply* reply)
    :OperationResponse(reply)
{

}

QString EndSponsoringFutureReservesOperationResponse::getBeginSponsor() const
{
    return m_beginSponsor;
}

void EndSponsoringFutureReservesOperationResponse::setBeginSponsor(QString beginSponsor)
{
    if (m_beginSponsor == beginSponsor)
        return;

    m_beginSponsor = beginSponsor;
    emit beginSponsorChanged();
}
QSTELLAR_END_NS
