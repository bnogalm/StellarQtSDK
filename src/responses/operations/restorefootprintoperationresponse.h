#ifndef RESTOREFOOTPRINTOPERATIONRESPONSE_H
#define RESTOREFOOTPRINTOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Represents RestoreFootprint operation response (CAP-46).
 *
 * No Soroban-specific fields beyond the OperationResponse base; included
 * for completeness so dispatch can return a typed object.
 */
class RestoreFootprintOperationResponse : public OperationResponse
{
    Q_OBJECT
public:
    explicit RestoreFootprintOperationResponse(QNetworkReply* reply = nullptr);
    ~RestoreFootprintOperationResponse() override = default;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(RestoreFootprintOperationResponse)



Q_DECLARE_METATYPE(qstellar::RestoreFootprintOperationResponse*)
#endif // RESTOREFOOTPRINTOPERATIONRESPONSE_H
