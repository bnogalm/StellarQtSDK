#include "beginsponsoringfuturereservesoperationresponse.h"

BeginSponsoringFutureReservesOperationResponse::BeginSponsoringFutureReservesOperationResponse(QNetworkReply* reply)
    :OperationResponse(reply)
{

}

BeginSponsoringFutureReservesOperationResponse::~BeginSponsoringFutureReservesOperationResponse()
{

}

QString BeginSponsoringFutureReservesOperationResponse::getSponsoredID() const
{
    return m_sponsoredID;
}

void BeginSponsoringFutureReservesOperationResponse::setSponsoredID(QString sponsoredID)
{
    if (m_sponsoredID == sponsoredID)
        return;

    m_sponsoredID = sponsoredID;
    emit sponsoredIDChanged();
}
