#ifndef OPERATIONPAGE_H
#define OPERATIONPAGE_H

#include <QObject>
#include "page.h"
#include "operations/operationresponse.h"


class OperationPage : public Page<OperationResponse>
{
public:
    OperationPage(QNetworkReply *reply);

    // PageBase interface
public:
    void processRecords(const QJsonArray &records);
};

#endif // OPERATIONPAGE_H
