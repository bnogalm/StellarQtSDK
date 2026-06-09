#ifndef GENERICOPERATION_H
#define GENERICOPERATION_H

#include <QObject>
#include "response.h"
#include "operations/operationresponse.h"
#include "qstellar_namespace.h"
QSTELLAR_BEGIN_NS

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

QSTELLAR_END_NS

QSTELLAR_ALIAS(GenericOperation)




#endif // GENERICOPERATION_H
