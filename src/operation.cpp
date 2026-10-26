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
    // A decimal number, read exactly, in the forms js-stellar-base (BigNumber)
    // and py-stellar-sdk (Decimal) read: an optional sign, digits with an
    // optional point (".5" and "5." included) and an optional exponent
    // ("1e3", "5E-1"). No locale guessing: a comma is refused, not taken for
    // a point. Nothing is rounded: more than seven decimals that are not
    // zeros, or a value outside int64 stroops, is refused. Only
    // [-]digits[.digits] used to be read, and ".5", "+1" or "1e2" were
    // refused where js and py sign them. '_' between digits (js and py) and
    // 0x / 0b / 0o (js) are refused on purpose: see the header.
    static const QRegularExpression re(
        QStringLiteral("^([+-]?)([0-9]*)(?:\\.([0-9]*))?(?:[eE]([+-]?[0-9]+))?$"));
    const QRegularExpressionMatch m = re.match(value);
    const QString integralDigits = m.hasMatch() ? m.captured(2) : QString();
    const QString fractionDigits = m.hasMatch() ? m.captured(3) : QString();
    if (!m.hasMatch() || (integralDigits.isEmpty() && fractionDigits.isEmpty()))
        throw std::runtime_error("invalid amount format: expected a decimal number such as 10, 10.5, .5 or 1e3");
    const bool negative = m.captured(1) == QLatin1String("-");

    // The digits without the point, and where the point goes in stroops.
    QString digits = integralDigits + fractionDigits;
    while (digits.size() > 1 && digits.at(0) == QLatin1Char('0'))
        digits.remove(0, 1);
    qint64 exponent = 0;
    if (!m.captured(4).isEmpty()) {
        bool ok = false;
        exponent = m.captured(4).toLongLong(&ok);
        // A longer exponent is out of range, or zero.
        if (!ok || exponent > 1000000 || exponent < -1000000)
            exponent = m.captured(4).startsWith(QLatin1Char('-')) ? -1000000 : 1000000;
    }
    if (digits == QLatin1String("0"))
        return 0;                                   // zero, whatever the exponent or sign
    qint64 shift = exponent - fractionDigits.size() + Operation::FRACTIONAL;
    if (shift < 0) {
        // Digits past the seventh decimal: only zeros may go.
        if (-shift >= digits.size())
            throw std::runtime_error("amount with too many decimals");
        const QString dropped = digits.right(int(-shift));
        for (const QChar c : dropped)
            if (c != QLatin1Char('0'))
                throw std::runtime_error("amount with too many decimals");
        digits.chop(int(-shift));
        shift = 0;
    }
    if (digits.size() + shift > 19)
        throw std::runtime_error("amount overflows int64 stroops");
    digits.append(QString(int(shift), QLatin1Char('0')));
    bool ok = false;
    const quint64 stroops = digits.toULongLong(&ok);
    if (!ok || stroops > quint64(std::numeric_limits<qint64>::max()))
        throw std::runtime_error("amount overflows int64 stroops");
    return negative ? -qint64(stroops) : qint64(stroops);
}

qint64 Operation::toXdrAmountPositive(QString value)
{
    const qint64 stroops = toXdrAmount(value);
    if (stroops <= 0)
        throw std::runtime_error(QStringLiteral("amount must be more than zero, not \"%1\"")
                                     .arg(value).toStdString());
    return stroops;
}

qint64 Operation::toXdrAmountNonNegative(QString value)
{
    const qint64 stroops = toXdrAmount(value);
    if (stroops < 0)
        throw std::runtime_error("amount cannot be negative");
    return stroops;
}


QString Operation::fromXdrAmount(qint64 value) {
    // The magnitude as unsigned (INT64_MIN has no positive qint64), and the
    // sign in front: with a negative value the remainder is negative too, and
    // -15000000 came out as "-1.-5", -5000000 as "0.-5" (the sign lost).
    const bool negative = value < 0;
    const quint64 magnitude = negative ? quint64(0) - quint64(value) : quint64(value);
    const quint64 one = quint64(Operation::ONE);
    const quint64 n = magnitude / one;
    const quint64 decimal = magnitude % one;
    const QString sign = negative ? QStringLiteral("-") : QString();
    if(decimal==0)
        return sign + QString::number(n);
    return sign + Util::removeTailChars(QString("%1.%2").arg(n).arg(decimal,7,10,QChar('0')),'0');
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
    // Neither swallowed nor unchecked: any failure used to come back as an
    // empty string, and a write that failed half way as the base64 of the
    // half that was written. Either is now an exception.
    stellar::Operation operation = this->toXdr();
    return xdr::encodeOrThrow(operation, "the operation").toBase64(XDR_BASE64ENCODING);
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
              // Credit and pool-share trustlines alike: the key holds a
              // TrustLineAsset, and the operation keeps it as it came.
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
        default:
          // It fell through with no operation: nullptr came back, and a
          // source account, when there was one, was set on it.
          throw std::runtime_error("Unknown revoke sponsorship type");
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
