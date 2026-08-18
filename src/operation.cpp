#include "operation.h"
#include "inflationoperation.h"

#include <QRegularExpression>
#include <limits>

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
#include "revokeliquiditypoolsponsorshipoperation.h"
#include "revokedatasponsorshipoperation.h"
#include "revokeoffersponsorshipoperation.h"
#include "revokesignersponsorshipoperation.h"
#include "revoketrustlinesponsorshipoperation.h"

#include "clawbackoperation.h"
#include "clawbackclaimablebalanceoperation.h"
#include "settrustlineflagsoperation.h"

#include "liquiditypooldepositoperation.h"
#include "liquiditypoolwithdrawoperation.h"

#include "invokehostfunctionoperation.h"
#include "extendfootprintttloperation.h"
#include "restorefootprintoperation.h"

QSTELLAR_BEGIN_NS

Operation::Operation()
{
}

Operation::~Operation()
{
}

qint64 Operation::toXdrAmount(QString value) {
    value = checkNotNull(value, "value cannot be null").trimmed();
    // Accept only the canonical amount format used by the other SDKs: an optional
    // sign, integer digits, and an optional '.' with decimals. No locale guessing
    // (a comma is rejected, not silently turned into a decimal point — see S2).
    static const QRegularExpression re(QStringLiteral("^(-?)([0-9]+)(?:\\.([0-9]+))?$"));
    const QRegularExpressionMatch m = re.match(value);
    if(!m.hasMatch())
        throw std::runtime_error("invalid amount format: expected [-]digits[.digits]");

    const bool negative = !m.captured(1).isEmpty();
    bool ok = false;
    const qint64 integral = m.captured(2).toLongLong(&ok);
    if(!ok)
        throw std::runtime_error("amount integral part out of range");

    QString fracStr = m.captured(3);
    if(fracStr.length() > Operation::FRACTIONAL){
        for(int i=Operation::FRACTIONAL ; i<fracStr.length(); i++)
            if(fracStr[i]!=QChar('0'))
                throw std::runtime_error("amount with too many decimals");
        fracStr = fracStr.left(Operation::FRACTIONAL);
    }
    qint64 frac = 0;
    if(!fracStr.isEmpty()){
        frac = fracStr.toLongLong(&ok);
        if(!ok)
            throw std::runtime_error("amount fractional part invalid");
        frac *= get_power(quint32(10), quint32(Operation::FRACTIONAL - fracStr.length()));
    }
    // result = integral*ONE + frac, with manual overflow detection (MSVC-safe).
    const qint64 MAXV = std::numeric_limits<qint64>::max();
    if(integral > MAXV / Operation::ONE)
        throw std::runtime_error("amount overflows int64 stroops");
    qint64 result = integral * Operation::ONE;
    if(frac > MAXV - result)
        throw std::runtime_error("amount overflows int64 stroops");
    result += frac;
    return negative ? -result : result;   // sign applied to the whole amount
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

stellar::Operation Operation::toXdr(AccountConverter accountConverter) {
    stellar::Operation xdr;
    if(!this->getSourceAccount().isEmpty()){
        stellar::MuxedAccount& sourceAccount = xdr.sourceAccount.filler();
        sourceAccount = accountConverter.encode(m_sourceAccount);
    }
    this->fillOperationBody(accountConverter, xdr);
    return xdr;
}

QString Operation::toXdrBase64() {
    try {        
        stellar::Operation operation = this->toXdr();
        QByteArray outputStream;
        QDataStream xdrOutputStream(&outputStream,QIODevice::WriteOnly);
        xdrOutputStream << operation;
        return outputStream.toBase64(XDR_BASE64ENCODING);
    } catch (const std::exception& e) {
        Q_UNUSED(e)
        //throw new AssertionError(e);
    }
    return QString();
}

Operation *Operation::fromXdr(AccountConverter accountConverter, stellar::Operation &xdr) {

    Operation* operation=nullptr;
    switch (xdr.type) {
    case stellar::OperationType::CREATE_ACCOUNT:
        operation = CreateAccountOperation::build(xdr.operationCreateAccount);
        break;
    case stellar::OperationType::PAYMENT:
        operation = PaymentOperation::build(accountConverter, xdr.operationPayment);
        break;
    case stellar::OperationType::PATH_PAYMENT_STRICT_RECEIVE:
        operation = PathPaymentOperation::build(accountConverter, xdr.operationPathPaymentStrictReceive);
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
        // Decoding still must build the deprecated op for legacy ledgers.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
        operation = AllowTrustOperation::build(xdr.operationAllowTrust);
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#endif
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
        operation = PathPaymentStrictSendOperation::build(accountConverter, xdr.operationPathPaymentStrictSend);
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
            case stellar::LedgerEntryType::LIQUIDITY_POOL:
              operation = RevokeLiquidityPoolSponsorshipOperation::build(xdr.operationRevokeSponsorship);
              break;
            default:
              throw std::runtime_error("Unknown revoke sponsorship ledger entry type");
          }
          break;
      }
        break;

    case stellar::OperationType::CLAWBACK:
      operation = ClawbackOperation::build(accountConverter, xdr.operationClawback);
      break;
    case stellar::OperationType::CLAWBACK_CLAIMABLE_BALANCE:
      operation = ClawbackClaimableBalanceOperation::build(xdr.operationClawbackClaimableBalance);
      break;
    case stellar::OperationType::SET_TRUST_LINE_FLAGS:
      operation = SetTrustlineFlagsOperation::build(xdr.operationSetTrustLineFlags);
      break;
    case stellar::OperationType::LIQUIDITY_POOL_DEPOSIT:
      operation = LiquidityPoolDepositOperation::build(xdr.operationLiquidityPoolDeposit);
      break;
    case stellar::OperationType::LIQUIDITY_POOL_WITHDRAW:
      operation = LiquidityPoolWithdrawOperation::build(xdr.operationLiquidityPoolWithdraw);
      break;
    case stellar::OperationType::INVOKE_HOST_FUNCTION:
      operation = InvokeHostFunctionOperation::build(xdr.operationInvokeHostFunction);
      break;
    case stellar::OperationType::EXTEND_FOOTPRINT_TTL:
      operation = ExtendFootprintTTLOperation::build(xdr.operationExtendFootprintTtl);
      break;
    case stellar::OperationType::RESTORE_FOOTPRINT:
      operation = RestoreFootprintOperation::build(xdr.operationRestoreFootprint);
      break;
    case stellar::OperationType::INFLATION:
      // A protocol fossil (inflation was disabled), but it still exists in
      // the historical ledger: without this case any envelope carrying it was
      // undecodable and the XDR Inspector threw. It has no body.
      operation = InflationOperation::create();
      break;
    default:
        throw std::runtime_error("Unknown operation body");
    }
    if (xdr.sourceAccount.filled) {                
        operation->setSourceAccount(accountConverter.decode(xdr.sourceAccount.value));
    }
    return operation;
}

Operation *Operation::fromXdr(stellar::Operation &xdr)
{
    return fromXdr(AccountConverter().enableMuxed(),xdr);
}

Operation *Operation::setSourceAccount(QString sourceAccount)
{    
    m_sourceAccount = sourceAccount;
    return this;
}

QString Operation::getSourceAccount() const{
    return m_sourceAccount;
}

QSTELLAR_END_NS
