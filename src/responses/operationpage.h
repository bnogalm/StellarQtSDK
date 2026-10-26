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

    /**
     * A new, unfilled record of the class for the operation type in
     * `record`'s type_i, which must be a whole number in [0, 2^31); throws
     * std::runtime_error naming `field`.type_i otherwise. A type this SDK
     * does not know gives a plain OperationResponse: getType() names it and
     * json() holds all its members, as js and py keep such a record. The
     * caller owns it.
     */
    static OperationResponse* createRecord(const QJsonObject& record, const QString& field);
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(OperationPage)




#endif // OPERATIONPAGE_H
