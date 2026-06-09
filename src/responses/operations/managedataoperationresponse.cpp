#include "managedataoperationresponse.h"

QSTELLAR_BEGIN_NS


ManageDataOperationResponse::ManageDataOperationResponse(QNetworkReply *reply)
    : OperationResponse(reply)
{

}

ManageDataOperationResponse::~ManageDataOperationResponse(){}

QString ManageDataOperationResponse::getName() const
{
    return m_name;
}

QString ManageDataOperationResponse::getValue() const
{
    return m_value;
}
QSTELLAR_END_NS
