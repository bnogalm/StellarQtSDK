#ifndef PATHPAYMENTSTRICTSENDOPERATIONRESPONSE_H
#define PATHPAYMENTSTRICTSENDOPERATIONRESPONSE_H

#include "pathpaymentbaseoperationresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Represents PATH_PAYMENT_STRICT_SEND operation response.
 *
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class PathPaymentStrictSendOperationResponse : public PathPaymentBaseOperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString destination_min MEMBER m_destinationMin)
    QString m_destinationMin;
public:
    PathPaymentStrictSendOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~PathPaymentStrictSendOperationResponse();

    QString getDestinationMin() const;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(PathPaymentStrictSendOperationResponse)



Q_DECLARE_METATYPE(qstellar::PathPaymentStrictSendOperationResponse*)
#endif // PATHPAYMENTSTRICTSENDOPERATIONRESPONSE_H
