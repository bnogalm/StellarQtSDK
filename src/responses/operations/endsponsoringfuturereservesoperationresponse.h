#ifndef ENDSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H
#define ENDSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H

#include "operationresponse.h"
#include <QObject>
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(EndSponsoringFutureReservesOperationResponse)




#endif // ENDSPONSORINGFUTURERESERVESOPERATIONRESPONSE_H
