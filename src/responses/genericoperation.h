#ifndef GENERICOPERATION_H
#define GENERICOPERATION_H

#include <QObject>
#include "response.h"
#include "operations/operationresponse.h"
/**
 * @brief The GenericOperation class
 * Helper class to deserialize correct OperationResponse
 */
class GenericOperation : public Response
{

    OperationResponse* m_operation;
public:
    GenericOperation(QNetworkReply * reply=nullptr);
    virtual ~GenericOperation();

    Q_INVOKABLE OperationResponse* operation() const;

    virtual void loadFromJson(QByteArray data);
};

#endif // GENERICOPERATION_H
