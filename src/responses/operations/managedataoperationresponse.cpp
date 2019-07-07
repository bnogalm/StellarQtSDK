#include "managedataoperationresponse.h"

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
