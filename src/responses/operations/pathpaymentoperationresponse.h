#ifndef PATHPAYMENTOPERATIONRESPONSE_H
#define PATHPAYMENTOPERATIONRESPONSE_H

#include <QObject>
#include "pathpaymentstrictreceiveoperationresponse.h"

class Asset;
/**
 * Represents PathPayment operation response.
 * @deprecated Will be removed in version 0.11.0, use {@link PathPaymentStrictReceiveOperationResponse}
 */
class PathPaymentOperationResponse : public PathPaymentStrictReceiveOperationResponse
{

public:
    PathPaymentOperationResponse(QNetworkReply* reply=nullptr);    


};
Q_DECLARE_METATYPE(PathPaymentOperationResponse*)
#endif // PATHPAYMENTOPERATIONRESPONSE_H
