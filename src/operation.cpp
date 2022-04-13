#include "operation.h"

#include "accountmergeoperation.h"
#include "createaccountoperation.h"
#include "paymentoperation.h"
#include "pathpaymentoperation.h"
#include "pathpaymentstrictreceiveoperation.h"
#include "pathpaymentstrictsendoperation.h"
#include "allowtrustoperation.h"
#include "changetrustoperation.h"
#include "createpassivesellofferoperation.h"
#include "managedataoperation.h"
#include "setoptionsoperation.h"
#include "managesellofferoperation.h"
#include "bumpsequenceoperation.h"
#include "managebuyofferoperation.h"

#include "createclaimablebalanceoperation.h"
#include "claimclaimablebalanceoperation.h"
#include "beginsponsoringfuturereservesoperation.h"
#include "endsponsoringfuturereservesoperation.h"

#include "revokeaccountsponsorshipoperation.h"
#include "revokeclaimablebalancesponsorshipoperation.h"
#include "revokedatasponsorshipoperation.h"
#include "revokeoffersponsorshipoperation.h"
#include "revokesignersponsorshipoperation.h"
#include "revoketrustlinesponsorshipoperation.h"

#include "clawbackoperation.h"
#include "clawbackclaimablebalanceoperation.h"
#include "settrustlineflagsoperation.h"


Operation::Operation()
{
}

Operation::~Operation()
{
}

qint64 Operation::toXdrAmount(QString value) {
    value = checkNotNull(value, "value cannot be null");
    value = value.replace(',','.');
    if(!value.contains('.'))
        return value.toLongLong()*Operation::ONE;
    QStringList parse = value.split('.');
    if(parse[1].length()<Operation::FRACTIONAL)
        return parse[0].toLongLong() * Operation::ONE + parse[1].toLongLong() *  get_power(quint32(10),quint32((Operation::FRACTIONAL-parse[1].length())));
    for(int i=Operation::FRACTIONAL ; i<parse[1].length();i++){
        if(parse[1][i]!=QChar('0')){
            throw std::runtime_error("amount with too many decimals");
        }
    }
    return parse[0].toLongLong() * Operation::ONE + parse[1].left(Operation::FRACTIONAL).toLongLong();
}


QString Operation::fromXdrAmount(qint64 value) {
    qint64 n= value/Operation::ONE;
    qint64 decimal = value - n*Operation::ONE;
    if(decimal==0)
        return  QString::number(value/Operation::ONE);
    else{
        return Util::removeTailChars(QString("%1.%2").arg(n).arg(decimal,7,10,QChar('0')),'0');
    }
}

stellar::Operation Operation::toXdr() {
    stellar::Operation xdr;
    if(!this->getSourceAccount().isEmpty()){
        stellar::MuxedAccount& sourceAccount = xdr.sourceAccount.filler();
        sourceAccount = StrKey::encodeToXDRMuxedAccount(m_sourceAccount);
    }
    this->fillOperationBody(xdr);
    return xdr;
}

QString Operation::toXdrBase64() {
    try {
        stellar::Operation operation = this->toXdr();
        QByteArray outputStream;
        QDataStream xdrOutputStream(&outputStream,QIODevice::WriteOnly);
        xdrOutputStream << operation;
        return outputStream.toBase64(XDR_BASE64ENCODING);
    } catch (std::exception e) {
        //throw new AssertionError(e);
    }
    return QString();
}

Operation *Operation::fromXdr(stellar::Operation &xdr) {

    Operation* operation=nullptr;
    switch (xdr.type) {
    case stellar::OperationType::CREATE_ACCOUNT:
        operation = CreateAccountOperation::build(xdr.operationCreateAccount);
        break;
    case stellar::OperationType::PAYMENT:
        operation = PaymentOperation::build(xdr.operationPayment);
        break;
    case stellar::OperationType::PATH_PAYMENT_STRICT_RECEIVE:
        operation = PathPaymentOperation::build(xdr.operationPathPaymentStrictReceive);
        break;
    case stellar::OperationType::MANAGE_SELL_OFFER:
        operation = ManageSellOfferOperation::build(xdr.operationManageSellOffer);
        break;
    case stellar::OperationType::CREATE_PASSIVE_SELL_OFFER:
        operation = CreatePassiveSellOfferOperation::build(xdr.operationCreatePassiveSellOffer);
        break;
    case stellar::OperationType::SET_OPTIONS:
        operation = SetOptionsOperation::build(xdr.operationSetOptions);
        break;
    case stellar::OperationType::CHANGE_TRUST:
        operation = ChangeTrustOperation::build(xdr.operationChangeTrust);
        break;
    case stellar::OperationType::ALLOW_TRUST:
        operation = AllowTrustOperation::build(xdr.operationAllowTrust);
        break;
    case stellar::OperationType::ACCOUNT_MERGE:
        operation = AccountMergeOperation::build(xdr.operationAccountMerge);
        break;
    case stellar::OperationType::MANAGE_DATA:
        operation = ManageDataOperation::build(xdr.operationManageData);
        break;
    case stellar::OperationType::BUMP_SEQUENCE:
        operation = BumpSequenceOperation::build(xdr.operationBumpSequence);
        break;
    case stellar::OperationType::MANAGE_BUY_OFFER:
        operation = ManageBuyOfferOperation::build(xdr.operationManageBuyOffer);
        break;
    case stellar::OperationType::PATH_PAYMENT_STRICT_SEND:
        operation = PathPaymentStrictSendOperation::build(xdr.operationPathPaymentStrictSend);
        break;
    case stellar::OperationType::CREATE_CLAIMABLE_BALANCE:
      operation = CreateClaimableBalanceOperation::build(xdr.operationCreateClaimableBalance);
      break;
    case stellar::OperationType::CLAIM_CLAIMABLE_BALANCE:
      operation = ClaimClaimableBalanceOperation::build(xdr.operationClaimClaimableBalance);
      break;
    case stellar::OperationType::BEGIN_SPONSORING_FUTURE_RESERVES:
      operation = BeginSponsoringFutureReservesOperation::build(xdr.operationBeginSponsoringFutureReserves);
      break;
    case stellar::OperationType::END_SPONSORING_FUTURE_RESERVES:
      operation = EndSponsoringFutureReservesOperation::build();
      break;
    case stellar::OperationType::REVOKE_SPONSORSHIP:
      switch (xdr.operationRevokeSponsorship.type) {
        case stellar::RevokeSponsorshipType::REVOKE_SPONSORSHIP_SIGNER:
          operation = RevokeSignerSponsorshipOperation::build(xdr.operationRevokeSponsorship);
          break;
        case stellar::RevokeSponsorshipType::REVOKE_SPONSORSHIP_LEDGER_ENTRY:
          switch (xdr.operationRevokeSponsorship.ledgerKey.type) {
            case stellar::LedgerEntryType::DATA:
              operation = RevokeDataSponsorshipOperation::build(xdr.operationRevokeSponsorship);
              break;
            case stellar::LedgerEntryType::OFFER:
              operation = RevokeOfferSponsorshipOperation::build(xdr.operationRevokeSponsorship);
              break;
            case stellar::LedgerEntryType::ACCOUNT:
              operation = RevokeAccountSponsorshipOperation::build(xdr.operationRevokeSponsorship);
              break;
            case stellar::LedgerEntryType::TRUSTLINE:
              operation = RevokeTrustlineSponsorshipOperation::build(xdr.operationRevokeSponsorship);
              break;
            case stellar::LedgerEntryType::CLAIMABLE_BALANCE:
              operation = RevokeClaimableBalanceSponsorshipOperation::build(xdr.operationRevokeSponsorship);
              break;
            default:
              throw std::runtime_error("Unknown revoke sponsorship ledger entry type");
          }
          break;
      }
        break;

    case stellar::OperationType::CLAWBACK:
      operation = ClawbackOperation::build(xdr.operationClawback);
      break;
    case stellar::OperationType::CLAWBACK_CLAIMABLE_BALANCE:
      operation = ClawbackClaimableBalanceOperation::build(xdr.operationClawbackClaimableBalance);
      break;
    case stellar::OperationType::SET_TRUST_LINE_FLAGS:
      operation = SetTrustlineFlagsOperation::build(xdr.operationSetTrustLineFlags);
      break;
    default:
        throw std::runtime_error("Unknown operation body");
    }
    if (xdr.sourceAccount.filled) {        
        operation->setSourceAccount(StrKey::encodeStellarAccountId(StrKey::muxedAccountToAccountId(xdr.sourceAccount.value)));
    }
    return operation;
}

Operation *Operation::setSourceAccount(QString sourceAccount)
{    
    m_sourceAccount = sourceAccount;
    return this;
}

QString Operation::getSourceAccount() const{
    return m_sourceAccount;
}
