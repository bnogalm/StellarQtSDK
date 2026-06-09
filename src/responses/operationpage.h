#ifndef OPERATIONPAGE_H
#define OPERATIONPAGE_H

#include <QObject>
#include "page.h"
#include "operations/operationresponse.h"
#include "qstellar_namespace.h"


QSTELLAR_BEGIN_NS

class OperationPage : public Page<OperationResponse>
{
public:
    OperationPage(QNetworkReply *reply);

    // PageBase interface
public:
    void processRecords(const QJsonArray &records);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(OperationPage)




#endif // OPERATIONPAGE_H
