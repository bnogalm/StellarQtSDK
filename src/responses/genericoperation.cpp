#include "genericoperation.h"
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>

#include "operationpage.h"
#include <memory>
#include <stdexcept>

QSTELLAR_BEGIN_NS


GenericOperation::GenericOperation(QNetworkReply *reply)
    :Response(reply),m_operation(nullptr)
{

}

GenericOperation::~GenericOperation()
{
    if(m_operation)
        delete m_operation;
}

OperationResponse *GenericOperation::operation() const
{
    return m_operation;
}

void GenericOperation::loadFromJson(QByteArray data)
{
    QJsonParseError check;
    QJsonDocument doc = QJsonDocument::fromJson(data,&check);

    if(check.error){
        throw std::runtime_error(check.errorString().toStdString());
    }
    if (!doc.isObject())
        throw std::runtime_error("expected a JSON object");
    QJsonObject obj = doc.object();
    // By type_i, like a page of operations. This read "type", which Horizon
    // writes as a name ("payment"), as a number: every operation became a
    // CreateAccountOperationResponse with its members left empty. Created
    // without the reply, which this object owns and has already read.
    std::unique_ptr<OperationResponse> operation(OperationPage::createRecord(obj, QString()));
    Response::fillObject(operation->metaObject(), operation.get(), obj, true);
    delete m_operation;
    m_operation = operation.release();
}

QSTELLAR_END_NS
