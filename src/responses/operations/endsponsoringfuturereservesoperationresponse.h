#ifndef ENDSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H
#define ENDSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H

#include "operationresponse.h"
#include <QObject>
/**
 * Represents EndSponsoringFutureReserves operation response.
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class EndSponsoringFutureReservesOperationResponse : public OperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString begin_sponsor READ getBeginSponsor WRITE setBeginSponsor NOTIFY beginSponsorChanged)

    QString m_beginSponsor;

public:
    EndSponsoringFutureReservesOperationResponse(QNetworkReply* reply=nullptr);
    QString getBeginSponsor() const;
public slots:
    void setBeginSponsor(QString beginSponsor);
signals:
    void beginSponsorChanged();
};

#endif // ENDSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H
