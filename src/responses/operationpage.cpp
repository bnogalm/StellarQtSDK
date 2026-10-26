#include "operationpage.h"
#include <QJsonObject>
#include <QJsonArray>
#include <limits>
#include <memory>
#include <stdexcept>
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
#include "operations/invokehostfunctionoperationresponse.h"
#include "operations/extendfootprintttloperationresponse.h"
#include "operations/restorefootprintoperationresponse.h"
#include "operations/liquiditypooldepositoperationresponse.h"
#include "operations/liquiditypoolwithdrawoperationresponse.h"
#include "operations/clawbackoperationresponse.h"
#include "operations/clawbackclaimablebalanceoperationresponse.h"
#include "operations/settrustlineflagsoperationresponse.h"

QSTELLAR_BEGIN_NS


OperationPage::OperationPage(QNetworkReply *reply):Page<OperationResponse>(reply)
{

}

OperationResponse *OperationPage::createRecord(const QJsonObject &record, const QString &field)
{
    // type_i, exactly: it used to be read with toInt(), so a record without
    // it, or with it as text, became a CreateAccountOperationResponse.
    const QString typeField = field.isEmpty() ? QStringLiteral("type_i") : field + QStringLiteral(".type_i");
    const int typeI = static_cast<int>(ResponseConverters::exactInteger(
        record.value(QStringLiteral("type_i")), typeField, 0, std::numeric_limits<qint32>::max()));
    OperationResponse * op=nullptr;
    stellar::OperationType type = static_cast<stellar::OperationType>(typeI);
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
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
        op = new AllowTrustOperationResponse();
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#endif
        break;
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
    case stellar::OperationType::INVOKE_HOST_FUNCTION:
        op = new InvokeHostFunctionOperationResponse();break;
    case stellar::OperationType::EXTEND_FOOTPRINT_TTL:
        op = new ExtendFootprintTTLOperationResponse();break;
    case stellar::OperationType::RESTORE_FOOTPRINT:
        op = new RestoreFootprintOperationResponse();break;
    case stellar::OperationType::LIQUIDITY_POOL_DEPOSIT:
        op = new LiquidityPoolDepositOperationResponse();break;
    case stellar::OperationType::LIQUIDITY_POOL_WITHDRAW:
        op = new LiquidityPoolWithdrawOperationResponse();break;
    // These three had classes but no case: a page holding a clawback was
    // refused whole ("Invalid operation type").
    case stellar::OperationType::CLAWBACK:
        op = new ClawbackOperationResponse();break;
    case stellar::OperationType::CLAWBACK_CLAIMABLE_BALANCE:
        op = new ClawbackClaimableBalanceOperationResponse();break;
    case stellar::OperationType::SET_TRUST_LINE_FLAGS:
        op = new SetTrustLineFlagsOperationResponse();break;
      default:
        // A type this SDK does not know yet (a later protocol's). It is kept
        // as a plain OperationResponse, its common members read and json()
        // holding the rest, as js and py hand such a record back. It refused
        // the whole page, operations().operation(id), and every stream event
        // of that type.
        op = new OperationResponse();break;
    }
    return op;
}

void OperationPage::processRecords(const QJsonArray &records)
{
    for(int i=0;i<records.size();i++){
        const QJsonObject obj = recordObject(records, i);
        std::unique_ptr<OperationResponse> op(createRecord(obj, recordField(i)));
        op->setParent(this);
        Response::fillObject(op->metaObject(), op.get(), obj, true, recordField(i));
        this->m_records.append(op.release());
    }
}
QSTELLAR_END_NS
