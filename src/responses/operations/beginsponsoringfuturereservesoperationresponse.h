#ifndef BEGINSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H
#define BEGINSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H

#include "operationresponse.h"
#include <QObject>
/**
 * Represents BeginSponsoringFutureReserves operation response.
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class BeginSponsoringFutureReservesOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString sponsored_id READ getSponsoredID WRITE setSponsoredID NOTIFY sponsoredIDChanged)
    QString m_sponsoredID;

public:
    BeginSponsoringFutureReservesOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~BeginSponsoringFutureReservesOperationResponse();
    QString getSponsoredID() const;
public slots:
    void setSponsoredID(QString sponsoredID);
signals:
    void sponsoredIDChanged();
};

#endif // BEGINSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H
