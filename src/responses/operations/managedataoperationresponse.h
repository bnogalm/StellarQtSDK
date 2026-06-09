#ifndef MANAGEDATAOPERATIONRESPONSE_H
#define MANAGEDATAOPERATIONRESPONSE_H

#include <QObject>
#include "operationresponse.h"
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/**
 * Represents ManageDataoperation response.
 * @see <a href="https://www.stellar.org/developers/horizon/reference/resources/operation.html" target="_blank">Operation documentation</a>
 * @see org.stellar.sdk.requests.OperationsRequestBuilder
 * @see org.stellar.sdk.Server#operations()
 */
class ManageDataOperationResponse : public OperationResponse
{
    Q_OBJECT

    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(QString value MEMBER m_value)
    QString m_name;
    QString m_value;

public:
    ManageDataOperationResponse(QNetworkReply *reply=nullptr);
    virtual ~ManageDataOperationResponse();
    QString getName() const;

    QString getValue() const;
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(ManageDataOperationResponse)



Q_DECLARE_METATYPE(qstellar::ManageDataOperationResponse*)
#endif // MANAGEDATAOPERATIONRESPONSE_H
