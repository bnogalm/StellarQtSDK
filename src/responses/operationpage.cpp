#include "operationpage.h"
#include <QJsonObject>
#include <QJsonArray>
#include "operations/createaccountoperationresponse.h"
#include "operations/paymentoperationresponse.h"
#include "operations/accountmergeoperationresponse.h"
#include "operations/pathpaymentstrictreceiveoperationresponse.h"
#include "operations/pathpaymentstrictsendoperationresponse.h"
#include "operations/inflationoperationresponse.h"
#include "operations/allowtrustoperationresponse.h"
#include "operations/changetrustoperationresponse.h"
#include "operations/createpassiveofferoperationresponse.h"
#include "operations/managedataoperationresponse.h"
#include "operations/setoptionsoperationresponse.h"
#include "operations/managesellofferoperationresponse.h"
#include "operations/bumpsequenceoperationresponse.h"
#include "operations/managesellofferoperationresponse.h"
#include "operations/managebuyofferoperationresponse.h"
#include "operations/createclaimablebalanceoperationresponse.h"
#include "operations/claimclaimablebalanceoperationresponse.h"
#include "operations/beginsponsoringfuturereservesoperationresponse.h"
#include "operations/endsponsoringfuturereservesoperationresponse.h"
#include "operations/revokesponsorshipoperationresponse.h"

OperationPage::OperationPage(QNetworkReply *reply):Page<OperationResponse>(reply)
{

}

void OperationPage::processRecords(const QJsonArray &records)
{
    for(int i=0;i<records.size();i++){
        QJsonObject obj = records.at(i).toObject();

        Response * op=nullptr;
        stellar::OperationType type = static_cast<stellar::OperationType>(obj.value("type_i").toInt(-1));
        switch (type) {
          case stellar::OperationType::CREATE_ACCOUNT:
            op = new CreateAccountOperationResponse();break;
          case stellar::OperationType::PAYMENT:
            op = new PaymentOperationResponse();break;
          case stellar::OperationType::PATH_PAYMENT_STRICT_RECEIVE:
            op = new PathPaymentStrictReceiveOperationResponse();break;
          case stellar::OperationType::MANAGE_SELL_OFFER:
            op = new ManageSellOfferOperationResponse();break;
          case stellar::OperationType::CREATE_PASSIVE_SELL_OFFER:
            op = new CreatePassiveSellOfferOperationResponse();break;
          case stellar::OperationType::SET_OPTIONS:
            op = new SetOptionsOperationResponse();break;
          case stellar::OperationType::CHANGE_TRUST:
            op = new ChangeTrustOperationResponse();break;
          case stellar::OperationType::ALLOW_TRUST:
            op = new AllowTrustOperationResponse();break;
          case stellar::OperationType::ACCOUNT_MERGE:
            op = new AccountMergeOperationResponse();break;
          case stellar::OperationType::INFLATION:
            op = new InflationOperationResponse();break;
          case stellar::OperationType::MANAGE_DATA:
            op = new ManageDataOperationResponse();break;
          case stellar::OperationType::BUMP_SEQUENCE:
            op = new BumpSequenceOperationResponse();break;
          case stellar::OperationType::MANAGE_BUY_OFFER:
            op = new ManageBuyOfferOperationResponse();break;
          case stellar::OperationType::PATH_PAYMENT_STRICT_SEND:
            op = new PathPaymentStrictSendOperationResponse();break;
        case stellar::OperationType::CREATE_CLAIMABLE_BALANCE:
            op = new CreateClaimableBalanceOperationResponse();break;
        case stellar::OperationType::CLAIM_CLAIMABLE_BALANCE:
            op = new ClaimClaimableBalanceOperationResponse();break;
        case stellar::OperationType::BEGIN_SPONSORING_FUTURE_RESERVES:
            op = new BeginSponsoringFutureReservesOperationResponse();break;
        case stellar::OperationType::END_SPONSORING_FUTURE_RESERVES:
            op = new EndSponsoringFutureReservesOperationResponse();break;
        case stellar::OperationType::REVOKE_SPONSORSHIP:
            op = new RevokeSponsorshipOperationResponse();break;
          default:
            throw std::runtime_error("Invalid operation type");
        }
        if(op){
            op->setParent(this);
            Response::fillObject(op->metaObject(),op,obj,true);
            this->m_records.append(op);
        }
    }
}
