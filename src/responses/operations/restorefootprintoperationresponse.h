#ifndef RESTOREFOOTPRINTOPERATIONRESPONSE_H
#define RESTOREFOOTPRINTOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"

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
Q_DECLARE_METATYPE(RestoreFootprintOperationResponse*)
#endif // RESTOREFOOTPRINTOPERATIONRESPONSE_H
