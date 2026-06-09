#ifndef PATHPAYMENTOPERATIONRESPONSE_H
#define PATHPAYMENTOPERATIONRESPONSE_H

#include <QObject>
#include "pathpaymentstrictreceiveoperationresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

class Asset;/**
 * Represents PathPayment operation response.
 * @deprecated Will be removed in version 0.11.0, use {@link PathPaymentStrictReceiveOperationResponse}
 */
class PathPaymentOperationResponse : public PathPaymentStrictReceiveOperationResponse
{

public:
    PathPaymentOperationResponse(QNetworkReply* reply=nullptr);    


};

QSTELLAR_END_NS

QSTELLAR_ALIAS(PathPaymentOperationResponse)



Q_DECLARE_METATYPE(qstellar::PathPaymentOperationResponse*)
#endif // PATHPAYMENTOPERATIONRESPONSE_H
