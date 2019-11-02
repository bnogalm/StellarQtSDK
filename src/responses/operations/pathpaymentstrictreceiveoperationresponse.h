#ifndef PATHPAYMENTSTRICTRECEIVEOPERATIONRESPONSE_H
#define PATHPAYMENTSTRICTRECEIVEOPERATIONRESPONSE_H

#include "pathpaymentbaseoperationresponse.h"

/**
 * Represents PATH_PAYMENT_STRICT_RECEIVE operation response.
 *
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class PathPaymentStrictReceiveOperationResponse : public PathPaymentBaseOperationResponse
{
    Q_OBJECT
    Q_PROPERTY(QString source_max MEMBER m_sourceMax)
    QString m_sourceMax;
public:
    PathPaymentStrictReceiveOperationResponse(QNetworkReply* reply=nullptr);
    virtual ~PathPaymentStrictReceiveOperationResponse();

    QString getSourceMax() const;
};
Q_DECLARE_METATYPE(PathPaymentStrictReceiveOperationResponse*)
#endif // PATHPAYMENTSTRICTRECEIVEOPERATIONRESPONSE_H
