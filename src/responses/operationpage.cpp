#include "operationpage.h"
#include <QJsonObject>
#include <QJsonArray>
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
#include "operations/manageofferoperationresponse.h"

OperationPage::OperationPage(QNetworkReply *reply):Page<OperationResponse>(reply)
{

}

void OperationPage::processRecords(const QJsonArray &records)
{
    for(int i=0;i<records.size();i++){
        QJsonObject obj = records.at(i).toObject();
        QString type = obj.value("type").toString();
        Response * op=0;
        if(type=="create_account"){
            op = new CreateAccountOperationResponse();
        }
        else if(type =="payment"){
            op = new PaymentOperationResponse();
        }
        else if(type =="allow_trust"){
            op = new AllowTrustOperationResponse();
        }
        else if(type =="change_trust"){
            op = new ChangeTrustOperationResponse();
        }
        else if(type =="set_options"){
            op = new SetOptionsOperationResponse();
        }
        else if(type =="account_merge"){
            op = new AccountMergeOperationResponse();
        }
        else if(type =="manage_offer"){
            op = new ManageOfferOperationResponse();
        }
        else if(type =="path_payment"){
            op = new PathPaymentOperationResponse();
        }
        else if(type =="create_passive_offer"){
            op = new CreatePassiveOfferOperationResponse();
        }
        else if(type =="inflation"){
            op = new InflationOperationResponse();
        }
        else if(type =="manage_data"){
            op = new ManageDataOperationResponse();
        }
        if(op){
            op->setParent(this);
            Response::fillObject(op->metaObject(),op,obj,true);
            this->m_records.append(op);
        }
    }
}
