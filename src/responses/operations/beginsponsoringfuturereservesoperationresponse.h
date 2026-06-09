#ifndef BEGINSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H
#define BEGINSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H

#include "operationresponse.h"
#include <QObject>
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(BeginSponsoringFutureReservesOperationResponse)




#endif // BEGINSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H
