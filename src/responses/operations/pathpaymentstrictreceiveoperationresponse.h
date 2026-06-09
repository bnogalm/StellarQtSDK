#ifndef PATHPAYMENTSTRICTRECEIVEOPERATIONRESPONSE_H
#define PATHPAYMENTSTRICTRECEIVEOPERATIONRESPONSE_H

#include "pathpaymentbaseoperationresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(PathPaymentStrictReceiveOperationResponse)



Q_DECLARE_METATYPE(qstellar::PathPaymentStrictReceiveOperationResponse*)
#endif // PATHPAYMENTSTRICTRECEIVEOPERATIONRESPONSE_H
