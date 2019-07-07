#include "genericoperation.h"
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>

#include "operations/createaccountoperationresponse.h"
#include "operations/paymentoperationresponse.h"
#include "operations/accountmergeoperationresponse.h"
#include "operations/pathpaymentoperationresponse.h"
#include "operations/inflationoperationresponse.h"
#include "operations/allowtrustoperationresponse.h"
#include "operations/changetrustoperationresponse.h"
#include "operations/createpassiveofferoperationresponse.h"
#include "operations/managedataoperationresponse.h"
#include "operations/setoptionsoperationresponse.h"
#include "operations/managesellofferoperationresponse.h"
#include "operations/bumpsequenceoperationresponse.h"
#include "operations/managebuyofferoperationresponse.h"

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
    QJsonObject obj = doc.object();
    int type = obj.value("type").toInt();
    switch (type) {
    case 0:
        m_operation = new CreateAccountOperationResponse(this->m_reply); break;
    case 1:
        m_operation = new PaymentOperationResponse(this->m_reply); break;
    case 2:
        m_operation = new PathPaymentOperationResponse(this->m_reply); break;
    case 3:
        m_operation = new ManageSellOfferOperationResponse(this->m_reply); break;
    case 4:
        m_operation = new CreatePassiveSellOfferOperationResponse(this->m_reply); break;
    case 5:
        m_operation = new SetOptionsOperationResponse(this->m_reply); break;
    case 6:
        m_operation = new ChangeTrustOperationResponse(this->m_reply); break;
    case 7:
        m_operation = new AllowTrustOperationResponse(this->m_reply); break;
    case 8:
        m_operation = new AccountMergeOperationResponse(this->m_reply); break;
    case 9:
        m_operation = new InflationOperationResponse(this->m_reply); break;
    case 10:
        m_operation = new ManageDataOperationResponse(this->m_reply); break;
    case 11:
        m_operation = new BumpSequenceOperationResponse(this->m_reply); break;
    case 12:
        m_operation = new ManageBuyOfferOperationResponse(this->m_reply); break;
    default:
        throw std::runtime_error("Invalid operation type");
    }
    Response::fillObject(m_operation->metaObject(),m_operation,obj,true);//reply will not emit finished again, so it will not get filled. Anyway this way we skip parsing if json is valid again
}

