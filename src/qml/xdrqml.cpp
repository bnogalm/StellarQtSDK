#include "xdrqml.h"

#include <QDataStream>
#include <QScopedPointer>
#include <stdexcept>

#include "scval/scv.h"
#include "strkey.h"
#include "util.h"

#include "xdr/contractspec.h"
#include "xdr/scval.h"
#include "xdr/sorobanops.h"
#include "xdr/sorobantxdata.h"
#include "xdr/stellartransaction.h"

QSTELLAR_BEGIN_NS

namespace {

QVariantMap field(const QString& name, const QString& value)
{
    QVariantMap row;
    row.insert(QStringLiteral("field"), name);
    row.insert(QStringLiteral("value"), value);
    return row;
}

QVariantMap table(const QVariantList& rows)
{
    QVariantMap out;
    QStringList columns;
    columns << QStringLiteral("field") << QStringLiteral("value");
    out.insert(QStringLiteral("columns"), QVariant(columns));
    out.insert(QStringLiteral("rows"), rows);
    out.insert(QStringLiteral("count"), rows.size());
    return out;
}

// The text as the decoder is given it: without the ASCII whitespace in it,
// at its ends or inside (space, tab, LF, form feed and CR), which
// js-stellar-sdk and py-stellar-sdk both skip (XDR pasted from an email, or a
// dump wrapped at 76 columns, was "not valid base64" here). Nothing else is
// skipped: any other character outside the alphabet is an error, as
// js-stellar-sdk makes it (py-stellar-sdk skips any character it does not
// know, as Qt's lenient decoder did). A form feed was refused; a vertical
// tab, a non-breaking space or another Unicode space at the ends was taken
// (QString::trimmed() took them off), which js refuses.
QByteArray base64Text(const QString& base64)
{
    QByteArray out;
    out.reserve(base64.size());
    for (const QChar c : base64) {
        if (c == QLatin1Char(' ') || c == QLatin1Char('\t') || c == QLatin1Char('\n')
                || c == QLatin1Char('\f') || c == QLatin1Char('\r'))
            continue;
        // A character outside Latin-1 becomes 0, which no decoder takes.
        out.append(c.toLatin1());
    }
    return out;
}

QByteArray decodeBase64(const QString& base64)
{
    // Strict: the lenient decoder skipped characters it did not know, so a
    // typo, or a URL-safe '-' / '_', became other bytes rather than an error.
    const auto raw = QByteArray::fromBase64Encoding(base64Text(base64),
                                                    XDR_BASE64ENCODING | QByteArray::AbortOnBase64DecodingErrors);
    if (!raw)
        throw std::runtime_error("that is not valid base64 (the standard alphabet, + and /, with = padding)");
    if ((*raw).isEmpty())
        throw std::runtime_error("there are no bytes there");
    return *raw;
}

/**
 * The name of a transaction result code.
 *
 * On the wire these are small negative numbers, and the number is the one thing
 * nobody can look up from memory. Showing -5 where the answer is "your sequence
 * number was wrong" is most of the reason failed transactions are confusing.
 */
QString transactionResultName(stellar::TransactionResultCode code)
{
    using C = stellar::TransactionResultCode;
    switch (code) {
    case C::txFEE_BUMP_INNER_SUCCESS: return QStringLiteral("tx_fee_bump_inner_success");
    case C::txSUCCESS:                return QStringLiteral("tx_success");
    case C::txFAILED:                 return QStringLiteral("tx_failed");
    case C::txTOO_EARLY:              return QStringLiteral("tx_too_early");
    case C::txTOO_LATE:               return QStringLiteral("tx_too_late");
    case C::txMISSING_OPERATION:      return QStringLiteral("tx_missing_operation");
    case C::txBAD_SEQ:                return QStringLiteral("tx_bad_seq");
    case C::txBAD_AUTH:               return QStringLiteral("tx_bad_auth");
    case C::txINSUFFICIENT_BALANCE:   return QStringLiteral("tx_insufficient_balance");
    case C::txNO_ACCOUNT:             return QStringLiteral("tx_no_account");
    case C::txINSUFFICIENT_FEE:       return QStringLiteral("tx_insufficient_fee");
    case C::txBAD_AUTH_EXTRA:         return QStringLiteral("tx_bad_auth_extra");
    case C::txINTERNAL_ERROR:         return QStringLiteral("tx_internal_error");
    case C::txNOT_SUPPORTED:          return QStringLiteral("tx_not_supported");
    case C::txFEE_BUMP_INNER_FAILED:  return QStringLiteral("tx_fee_bump_inner_failed");
    case C::txBAD_SPONSORSHIP:        return QStringLiteral("tx_bad_sponsorship");
    // The codes later protocols added: each was read, and shown as "unknown".
    case C::txBAD_MIN_SEQ_AGE_OR_GAP: return QStringLiteral("tx_bad_min_seq_age_or_gap");
    case C::txMALFORMED:              return QStringLiteral("tx_malformed");
    case C::txSOROBAN_INVALID:        return QStringLiteral("tx_soroban_invalid");
    case C::txFROZEN_KEY_ACCESSED:    return QStringLiteral("tx_frozen_key_accessed");
    }
    return QStringLiteral("unknown (%1)").arg(static_cast<qint32>(code));
}

QString operationResultName(stellar::OperationResultCode code)
{
    using C = stellar::OperationResultCode;
    switch (code) {
    case C::opINNER:                return QStringLiteral("op_inner");
    case C::opBAD_AUTH:             return QStringLiteral("op_bad_auth");
    case C::opNO_ACCOUNT:           return QStringLiteral("op_no_account");
    case C::opNOT_SUPPORTED:        return QStringLiteral("op_not_supported");
    case C::opTOO_MANY_SUBENTRIES:  return QStringLiteral("op_too_many_subentries");
    case C::opEXCEEDED_WORK_LIMIT:  return QStringLiteral("op_exceeded_work_limit");
    case C::opTOO_MANY_SPONSORING:  return QStringLiteral("op_too_many_sponsoring");
    }
    return QStringLiteral("unknown (%1)").arg(static_cast<qint32>(code));
}

QString operationTypeName(stellar::OperationType type)
{
    using T = stellar::OperationType;
    switch (type) {
    case T::CREATE_ACCOUNT:                  return QStringLiteral("create_account");
    case T::PAYMENT:                         return QStringLiteral("payment");
    case T::PATH_PAYMENT_STRICT_RECEIVE:     return QStringLiteral("path_payment_strict_receive");
    case T::MANAGE_SELL_OFFER:               return QStringLiteral("manage_sell_offer");
    case T::CREATE_PASSIVE_SELL_OFFER:       return QStringLiteral("create_passive_sell_offer");
    case T::SET_OPTIONS:                     return QStringLiteral("set_options");
    case T::CHANGE_TRUST:                    return QStringLiteral("change_trust");
    case T::ALLOW_TRUST:                     return QStringLiteral("allow_trust");
    case T::ACCOUNT_MERGE:                   return QStringLiteral("account_merge");
    case T::INFLATION:                       return QStringLiteral("inflation");
    case T::MANAGE_DATA:                     return QStringLiteral("manage_data");
    case T::BUMP_SEQUENCE:                   return QStringLiteral("bump_sequence");
    case T::MANAGE_BUY_OFFER:                return QStringLiteral("manage_buy_offer");
    case T::PATH_PAYMENT_STRICT_SEND:        return QStringLiteral("path_payment_strict_send");
    case T::CREATE_CLAIMABLE_BALANCE:        return QStringLiteral("create_claimable_balance");
    case T::CLAIM_CLAIMABLE_BALANCE:         return QStringLiteral("claim_claimable_balance");
    case T::BEGIN_SPONSORING_FUTURE_RESERVES:
        return QStringLiteral("begin_sponsoring_future_reserves");
    case T::END_SPONSORING_FUTURE_RESERVES:
        return QStringLiteral("end_sponsoring_future_reserves");
    case T::REVOKE_SPONSORSHIP:              return QStringLiteral("revoke_sponsorship");
    case T::CLAWBACK:                        return QStringLiteral("clawback");
    case T::CLAWBACK_CLAIMABLE_BALANCE:      return QStringLiteral("clawback_claimable_balance");
    case T::SET_TRUST_LINE_FLAGS:            return QStringLiteral("set_trust_line_flags");
    case T::LIQUIDITY_POOL_DEPOSIT:          return QStringLiteral("liquidity_pool_deposit");
    case T::LIQUIDITY_POOL_WITHDRAW:         return QStringLiteral("liquidity_pool_withdraw");
    case T::INVOKE_HOST_FUNCTION:            return QStringLiteral("invoke_host_function");
    case T::EXTEND_FOOTPRINT_TTL:            return QStringLiteral("extend_footprint_ttl");
    case T::RESTORE_FOOTPRINT:               return QStringLiteral("restore_footprint");
    }
    return QStringLiteral("type %1").arg(static_cast<qint32>(type));
}

/** A code outside its enum. The reader refuses those, so only a result built
 *  by hand gets here. */
template <typename Code>
QString unknownCode(Code code)
{
    return QStringLiteral("code %1").arg(static_cast<qint32>(code));
}

// The XDR name, lower-cased: PAYMENT_UNDERFUNDED is payment_underfunded.
#define XDRQML_CODE(Name) case C::Name: return QString::fromLatin1(#Name).toLower();

QString codeName(stellar::CreateAccountResultCode code)
{
    using C = stellar::CreateAccountResultCode;
    switch (code) {
    XDRQML_CODE(CREATE_ACCOUNT_SUCCESS)
    XDRQML_CODE(CREATE_ACCOUNT_MALFORMED)
    XDRQML_CODE(CREATE_ACCOUNT_UNDERFUNDED)
    XDRQML_CODE(CREATE_ACCOUNT_LOW_RESERVE)
    XDRQML_CODE(CREATE_ACCOUNT_ALREADY_EXIST)
    }
    return unknownCode(code);
}

QString codeName(stellar::PaymentResultCode code)
{
    using C = stellar::PaymentResultCode;
    switch (code) {
    XDRQML_CODE(PAYMENT_SUCCESS)
    XDRQML_CODE(PAYMENT_MALFORMED)
    XDRQML_CODE(PAYMENT_UNDERFUNDED)
    XDRQML_CODE(PAYMENT_SRC_NO_TRUST)
    XDRQML_CODE(PAYMENT_SRC_NOT_AUTHORIZED)
    XDRQML_CODE(PAYMENT_NO_DESTINATION)
    XDRQML_CODE(PAYMENT_NO_TRUST)
    XDRQML_CODE(PAYMENT_NOT_AUTHORIZED)
    XDRQML_CODE(PAYMENT_LINE_FULL)
    XDRQML_CODE(PAYMENT_NO_ISSUER)
    }
    return unknownCode(code);
}

QString codeName(stellar::PathPaymentStrictReceiveResultCode code)
{
    using C = stellar::PathPaymentStrictReceiveResultCode;
    switch (code) {
    XDRQML_CODE(PATH_PAYMENT_STRICT_RECEIVE_SUCCESS)
    XDRQML_CODE(PATH_PAYMENT_STRICT_RECEIVE_MALFORMED)
    XDRQML_CODE(PATH_PAYMENT_STRICT_RECEIVE_UNDERFUNDED)
    XDRQML_CODE(PATH_PAYMENT_STRICT_RECEIVE_SRC_NO_TRUST)
    XDRQML_CODE(PATH_PAYMENT_STRICT_RECEIVE_SRC_NOT_AUTHORIZED)
    XDRQML_CODE(PATH_PAYMENT_STRICT_RECEIVE_NO_DESTINATION)
    XDRQML_CODE(PATH_PAYMENT_STRICT_RECEIVE_NO_TRUST)
    XDRQML_CODE(PATH_PAYMENT_STRICT_RECEIVE_NOT_AUTHORIZED)
    XDRQML_CODE(PATH_PAYMENT_STRICT_RECEIVE_LINE_FULL)
    XDRQML_CODE(PATH_PAYMENT_STRICT_RECEIVE_NO_ISSUER)
    XDRQML_CODE(PATH_PAYMENT_STRICT_RECEIVE_TOO_FEW_OFFERS)
    XDRQML_CODE(PATH_PAYMENT_STRICT_RECEIVE_OFFER_CROSS_SELF)
    XDRQML_CODE(PATH_PAYMENT_STRICT_RECEIVE_OVER_SENDMAX)
    }
    return unknownCode(code);
}

QString codeName(stellar::PathPaymentStrictSendResultCode code)
{
    using C = stellar::PathPaymentStrictSendResultCode;
    switch (code) {
    XDRQML_CODE(PATH_PAYMENT_STRICT_SEND_SUCCESS)
    XDRQML_CODE(PATH_PAYMENT_STRICT_SEND_MALFORMED)
    XDRQML_CODE(PATH_PAYMENT_STRICT_SEND_UNDERFUNDED)
    XDRQML_CODE(PATH_PAYMENT_STRICT_SEND_SRC_NO_TRUST)
    XDRQML_CODE(PATH_PAYMENT_STRICT_SEND_SRC_NOT_AUTHORIZED)
    XDRQML_CODE(PATH_PAYMENT_STRICT_SEND_NO_DESTINATION)
    XDRQML_CODE(PATH_PAYMENT_STRICT_SEND_NO_TRUST)
    XDRQML_CODE(PATH_PAYMENT_STRICT_SEND_NOT_AUTHORIZED)
    XDRQML_CODE(PATH_PAYMENT_STRICT_SEND_LINE_FULL)
    XDRQML_CODE(PATH_PAYMENT_STRICT_SEND_NO_ISSUER)
    XDRQML_CODE(PATH_PAYMENT_STRICT_SEND_TOO_FEW_OFFERS)
    XDRQML_CODE(PATH_PAYMENT_STRICT_SEND_OFFER_CROSS_SELF)
    XDRQML_CODE(PATH_PAYMENT_STRICT_SEND_UNDER_DESTMIN)
    }
    return unknownCode(code);
}

QString codeName(stellar::ManageSellOfferResultCode code)
{
    using C = stellar::ManageSellOfferResultCode;
    switch (code) {
    XDRQML_CODE(MANAGE_SELL_OFFER_SUCCESS)
    XDRQML_CODE(MANAGE_SELL_OFFER_MALFORMED)
    XDRQML_CODE(MANAGE_SELL_OFFER_SELL_NO_TRUST)
    XDRQML_CODE(MANAGE_SELL_OFFER_BUY_NO_TRUST)
    XDRQML_CODE(MANAGE_SELL_OFFER_SELL_NOT_AUTHORIZED)
    XDRQML_CODE(MANAGE_SELL_OFFER_BUY_NOT_AUTHORIZED)
    XDRQML_CODE(MANAGE_SELL_OFFER_LINE_FULL)
    XDRQML_CODE(MANAGE_SELL_OFFER_UNDERFUNDED)
    XDRQML_CODE(MANAGE_SELL_OFFER_CROSS_SELF)
    XDRQML_CODE(MANAGE_SELL_OFFER_SELL_NO_ISSUER)
    XDRQML_CODE(MANAGE_SELL_OFFER_BUY_NO_ISSUER)
    XDRQML_CODE(MANAGE_SELL_OFFER_NOT_FOUND)
    XDRQML_CODE(MANAGE_SELL_OFFER_LOW_RESERVE)
    }
    return unknownCode(code);
}

QString codeName(stellar::ManageBuyOfferResultCode code)
{
    using C = stellar::ManageBuyOfferResultCode;
    switch (code) {
    XDRQML_CODE(MANAGE_BUY_OFFER_SUCCESS)
    XDRQML_CODE(MANAGE_BUY_OFFER_MALFORMED)
    XDRQML_CODE(MANAGE_BUY_OFFER_SELL_NO_TRUST)
    XDRQML_CODE(MANAGE_BUY_OFFER_BUY_NO_TRUST)
    XDRQML_CODE(MANAGE_BUY_OFFER_SELL_NOT_AUTHORIZED)
    XDRQML_CODE(MANAGE_BUY_OFFER_BUY_NOT_AUTHORIZED)
    XDRQML_CODE(MANAGE_BUY_OFFER_LINE_FULL)
    XDRQML_CODE(MANAGE_BUY_OFFER_UNDERFUNDED)
    XDRQML_CODE(MANAGE_BUY_OFFER_CROSS_SELF)
    XDRQML_CODE(MANAGE_BUY_OFFER_SELL_NO_ISSUER)
    XDRQML_CODE(MANAGE_BUY_OFFER_BUY_NO_ISSUER)
    XDRQML_CODE(MANAGE_BUY_OFFER_NOT_FOUND)
    XDRQML_CODE(MANAGE_BUY_OFFER_LOW_RESERVE)
    }
    return unknownCode(code);
}

QString codeName(stellar::SetOptionsResultCode code)
{
    using C = stellar::SetOptionsResultCode;
    switch (code) {
    XDRQML_CODE(SET_OPTIONS_SUCCESS)
    XDRQML_CODE(SET_OPTIONS_LOW_RESERVE)
    XDRQML_CODE(SET_OPTIONS_TOO_MANY_SIGNERS)
    XDRQML_CODE(SET_OPTIONS_BAD_FLAGS)
    XDRQML_CODE(SET_OPTIONS_INVALID_INFLATION)
    XDRQML_CODE(SET_OPTIONS_CANT_CHANGE)
    XDRQML_CODE(SET_OPTIONS_UNKNOWN_FLAG)
    XDRQML_CODE(SET_OPTIONS_THRESHOLD_OUT_OF_RANGE)
    XDRQML_CODE(SET_OPTIONS_BAD_SIGNER)
    XDRQML_CODE(SET_OPTIONS_INVALID_HOME_DOMAIN)
    XDRQML_CODE(SET_OPTIONS_AUTH_REVOCABLE_REQUIRED)
    }
    return unknownCode(code);
}

QString codeName(stellar::ChangeTrustResultCode code)
{
    using C = stellar::ChangeTrustResultCode;
    switch (code) {
    XDRQML_CODE(CHANGE_TRUST_SUCCESS)
    XDRQML_CODE(CHANGE_TRUST_MALFORMED)
    XDRQML_CODE(CHANGE_TRUST_NO_ISSUER)
    XDRQML_CODE(CHANGE_TRUST_INVALID_LIMIT)
    XDRQML_CODE(CHANGE_TRUST_LOW_RESERVE)
    XDRQML_CODE(CHANGE_TRUST_SELF_NOT_ALLOWED)
    XDRQML_CODE(CHANGE_TRUST_TRUST_LINE_MISSING)
    XDRQML_CODE(CHANGE_TRUST_CANNOT_DELETE)
    XDRQML_CODE(CHANGE_TRUST_NOT_AUTH_MAINTAIN_LIABILITIES)
    }
    return unknownCode(code);
}

QString codeName(stellar::AllowTrustResultCode code)
{
    using C = stellar::AllowTrustResultCode;
    switch (code) {
    XDRQML_CODE(ALLOW_TRUST_SUCCESS)
    XDRQML_CODE(ALLOW_TRUST_MALFORMED)
    XDRQML_CODE(ALLOW_TRUST_NO_TRUST_LINE)
    XDRQML_CODE(ALLOW_TRUST_TRUST_NOT_REQUIRED)
    XDRQML_CODE(ALLOW_TRUST_CANT_REVOKE)
    XDRQML_CODE(ALLOW_TRUST_SELF_NOT_ALLOWED)
    XDRQML_CODE(ALLOW_TRUST_LOW_RESERVE)
    }
    return unknownCode(code);
}

QString codeName(stellar::AccountMergeResultCode code)
{
    using C = stellar::AccountMergeResultCode;
    switch (code) {
    XDRQML_CODE(ACCOUNT_MERGE_SUCCESS)
    XDRQML_CODE(ACCOUNT_MERGE_MALFORMED)
    XDRQML_CODE(ACCOUNT_MERGE_NO_ACCOUNT)
    XDRQML_CODE(ACCOUNT_MERGE_IMMUTABLE_SET)
    XDRQML_CODE(ACCOUNT_MERGE_HAS_SUB_ENTRIES)
    XDRQML_CODE(ACCOUNT_MERGE_SEQNUM_TOO_FAR)
    XDRQML_CODE(ACCOUNT_MERGE_DEST_FULL)
    XDRQML_CODE(ACCOUNT_MERGE_IS_SPONSOR)
    }
    return unknownCode(code);
}

QString codeName(stellar::InflationResultCode code)
{
    using C = stellar::InflationResultCode;
    switch (code) {
    XDRQML_CODE(INFLATION_SUCCESS)
    XDRQML_CODE(INFLATION_NOT_TIME)
    }
    return unknownCode(code);
}

QString codeName(stellar::ManageDataResultCode code)
{
    using C = stellar::ManageDataResultCode;
    switch (code) {
    XDRQML_CODE(MANAGE_DATA_SUCCESS)
    XDRQML_CODE(MANAGE_DATA_NOT_SUPPORTED_YET)
    XDRQML_CODE(MANAGE_DATA_NAME_NOT_FOUND)
    XDRQML_CODE(MANAGE_DATA_LOW_RESERVE)
    XDRQML_CODE(MANAGE_DATA_INVALID_NAME)
    }
    return unknownCode(code);
}

QString codeName(stellar::BumpSequenceResultCode code)
{
    using C = stellar::BumpSequenceResultCode;
    switch (code) {
    XDRQML_CODE(BUMP_SEQUENCE_SUCCESS)
    XDRQML_CODE(BUMP_SEQUENCE_BAD_SEQ)
    }
    return unknownCode(code);
}

QString codeName(stellar::CreateClaimableBalanceResultCode code)
{
    using C = stellar::CreateClaimableBalanceResultCode;
    switch (code) {
    XDRQML_CODE(CREATE_CLAIMABLE_BALANCE_SUCCESS)
    XDRQML_CODE(CREATE_CLAIMABLE_BALANCE_MALFORMED)
    XDRQML_CODE(CREATE_CLAIMABLE_BALANCE_LOW_RESERVE)
    XDRQML_CODE(CREATE_CLAIMABLE_BALANCE_NO_TRUST)
    XDRQML_CODE(CREATE_CLAIMABLE_BALANCE_NOT_AUTHORIZED)
    XDRQML_CODE(CREATE_CLAIMABLE_BALANCE_UNDERFUNDED)
    }
    return unknownCode(code);
}

QString codeName(stellar::ClaimClaimableBalanceResultCode code)
{
    using C = stellar::ClaimClaimableBalanceResultCode;
    switch (code) {
    XDRQML_CODE(CLAIM_CLAIMABLE_BALANCE_SUCCESS)
    XDRQML_CODE(CLAIM_CLAIMABLE_BALANCE_DOES_NOT_EXIST)
    XDRQML_CODE(CLAIM_CLAIMABLE_BALANCE_CANNOT_CLAIM)
    XDRQML_CODE(CLAIM_CLAIMABLE_BALANCE_LINE_FULL)
    XDRQML_CODE(CLAIM_CLAIMABLE_BALANCE_NO_TRUST)
    XDRQML_CODE(CLAIM_CLAIMABLE_BALANCE_NOT_AUTHORIZED)
    XDRQML_CODE(CLAIM_CLAIMABLE_BALANCE_TRUSTLINE_FROZEN)
    }
    return unknownCode(code);
}

QString codeName(stellar::BeginSponsoringFutureReservesResultCode code)
{
    using C = stellar::BeginSponsoringFutureReservesResultCode;
    switch (code) {
    XDRQML_CODE(BEGIN_SPONSORING_FUTURE_RESERVES_SUCCESS)
    XDRQML_CODE(BEGIN_SPONSORING_FUTURE_RESERVES_MALFORMED)
    XDRQML_CODE(BEGIN_SPONSORING_FUTURE_RESERVES_ALREADY_SPONSORED)
    XDRQML_CODE(BEGIN_SPONSORING_FUTURE_RESERVES_RECURSIVE)
    }
    return unknownCode(code);
}

QString codeName(stellar::EndSponsoringFutureReservesResultCode code)
{
    using C = stellar::EndSponsoringFutureReservesResultCode;
    switch (code) {
    XDRQML_CODE(END_SPONSORING_FUTURE_RESERVES_SUCCESS)
    XDRQML_CODE(END_SPONSORING_FUTURE_RESERVES_NOT_SPONSORED)
    }
    return unknownCode(code);
}

QString codeName(stellar::RevokeSponsorshipResultCode code)
{
    using C = stellar::RevokeSponsorshipResultCode;
    switch (code) {
    XDRQML_CODE(REVOKE_SPONSORSHIP_SUCCESS)
    XDRQML_CODE(REVOKE_SPONSORSHIP_DOES_NOT_EXIST)
    XDRQML_CODE(REVOKE_SPONSORSHIP_NOT_SPONSOR)
    XDRQML_CODE(REVOKE_SPONSORSHIP_LOW_RESERVE)
    XDRQML_CODE(REVOKE_SPONSORSHIP_ONLY_TRANSFERABLE)
    XDRQML_CODE(REVOKE_SPONSORSHIP_MALFORMED)
    }
    return unknownCode(code);
}

QString codeName(stellar::ClawbackResultCode code)
{
    using C = stellar::ClawbackResultCode;
    switch (code) {
    XDRQML_CODE(CLAWBACK_SUCCESS)
    XDRQML_CODE(CLAWBACK_MALFORMED)
    XDRQML_CODE(CLAWBACK_NOT_CLAWBACK_ENABLED)
    XDRQML_CODE(CLAWBACK_NO_TRUST)
    XDRQML_CODE(CLAWBACK_UNDERFUNDED)
    }
    return unknownCode(code);
}

QString codeName(stellar::ClawbackClaimableBalanceResultCode code)
{
    using C = stellar::ClawbackClaimableBalanceResultCode;
    switch (code) {
    XDRQML_CODE(CLAWBACK_CLAIMABLE_BALANCE_SUCCESS)
    XDRQML_CODE(CLAWBACK_CLAIMABLE_BALANCE_DOES_NOT_EXIST)
    XDRQML_CODE(CLAWBACK_CLAIMABLE_BALANCE_NOT_ISSUER)
    XDRQML_CODE(CLAWBACK_CLAIMABLE_BALANCE_NOT_CLAWBACK_ENABLED)
    }
    return unknownCode(code);
}

QString codeName(stellar::SetTrustLineFlagsResultCode code)
{
    using C = stellar::SetTrustLineFlagsResultCode;
    switch (code) {
    XDRQML_CODE(SET_TRUST_LINE_FLAGS_SUCCESS)
    XDRQML_CODE(SET_TRUST_LINE_FLAGS_MALFORMED)
    XDRQML_CODE(SET_TRUST_LINE_FLAGS_NO_TRUST_LINE)
    XDRQML_CODE(SET_TRUST_LINE_FLAGS_CANT_REVOKE)
    XDRQML_CODE(SET_TRUST_LINE_FLAGS_INVALID_STATE)
    XDRQML_CODE(SET_TRUST_LINE_FLAGS_LOW_RESERVE)
    }
    return unknownCode(code);
}

QString codeName(stellar::LiquidityPoolDepositResultCode code)
{
    using C = stellar::LiquidityPoolDepositResultCode;
    switch (code) {
    XDRQML_CODE(LIQUIDITY_POOL_DEPOSIT_SUCCESS)
    XDRQML_CODE(LIQUIDITY_POOL_DEPOSIT_MALFORMED)
    XDRQML_CODE(LIQUIDITY_POOL_DEPOSIT_NO_TRUST)
    XDRQML_CODE(LIQUIDITY_POOL_DEPOSIT_NOT_AUTHORIZED)
    XDRQML_CODE(LIQUIDITY_POOL_DEPOSIT_UNDERFUNDED)
    XDRQML_CODE(LIQUIDITY_POOL_DEPOSIT_LINE_FULL)
    XDRQML_CODE(LIQUIDITY_POOL_DEPOSIT_BAD_PRICE)
    XDRQML_CODE(LIQUIDITY_POOL_DEPOSIT_POOL_FULL)
    XDRQML_CODE(LIQUIDITY_POOL_DEPOSIT_TRUSTLINE_FROZEN)
    }
    return unknownCode(code);
}

QString codeName(stellar::LiquidityPoolWithdrawResultCode code)
{
    using C = stellar::LiquidityPoolWithdrawResultCode;
    switch (code) {
    XDRQML_CODE(LIQUIDITY_POOL_WITHDRAW_SUCCESS)
    XDRQML_CODE(LIQUIDITY_POOL_WITHDRAW_MALFORMED)
    XDRQML_CODE(LIQUIDITY_POOL_WITHDRAW_NO_TRUST)
    XDRQML_CODE(LIQUIDITY_POOL_WITHDRAW_UNDERFUNDED)
    XDRQML_CODE(LIQUIDITY_POOL_WITHDRAW_LINE_FULL)
    XDRQML_CODE(LIQUIDITY_POOL_WITHDRAW_UNDER_MINIMUM)
    XDRQML_CODE(LIQUIDITY_POOL_WITHDRAW_TRUSTLINE_FROZEN)
    }
    return unknownCode(code);
}

QString codeName(stellar::InvokeHostFunctionResultCode code)
{
    using C = stellar::InvokeHostFunctionResultCode;
    switch (code) {
    XDRQML_CODE(INVOKE_HOST_FUNCTION_SUCCESS)
    XDRQML_CODE(INVOKE_HOST_FUNCTION_MALFORMED)
    XDRQML_CODE(INVOKE_HOST_FUNCTION_TRAPPED)
    XDRQML_CODE(INVOKE_HOST_FUNCTION_RESOURCE_LIMIT_EXCEEDED)
    XDRQML_CODE(INVOKE_HOST_FUNCTION_ENTRY_ARCHIVED)
    XDRQML_CODE(INVOKE_HOST_FUNCTION_INSUFFICIENT_REFUNDABLE_FEE)
    }
    return unknownCode(code);
}

QString codeName(stellar::ExtendFootprintTTLResultCode code)
{
    using C = stellar::ExtendFootprintTTLResultCode;
    switch (code) {
    XDRQML_CODE(EXTEND_FOOTPRINT_TTL_SUCCESS)
    XDRQML_CODE(EXTEND_FOOTPRINT_TTL_MALFORMED)
    XDRQML_CODE(EXTEND_FOOTPRINT_TTL_RESOURCE_LIMIT_EXCEEDED)
    XDRQML_CODE(EXTEND_FOOTPRINT_TTL_INSUFFICIENT_REFUNDABLE_FEE)
    }
    return unknownCode(code);
}

QString codeName(stellar::RestoreFootprintResultCode code)
{
    using C = stellar::RestoreFootprintResultCode;
    switch (code) {
    XDRQML_CODE(RESTORE_FOOTPRINT_SUCCESS)
    XDRQML_CODE(RESTORE_FOOTPRINT_MALFORMED)
    XDRQML_CODE(RESTORE_FOOTPRINT_RESOURCE_LIMIT_EXCEEDED)
    XDRQML_CODE(RESTORE_FOOTPRINT_INSUFFICIENT_REFUNDABLE_FEE)
    }
    return unknownCode(code);
}
#undef XDRQML_CODE

/**
 * The per-operation code, by name, for every operation.
 *
 * Only payments, account creation and trust lines used to be named: every
 * other operation showed a dash, and trust-line codes newer than
 * CHANGE_TRUST_SELF_NOT_ALLOWED a bare number, so a failure the network
 * explained (a frozen trustline, a trapped contract, a claimable balance that
 * cannot be claimed) said nothing. Each switch names every code of its enum
 * and has no default, so a code added to the XDR and not here is a -Wswitch
 * warning rather than a silent number.
 */
QString operationDetail(const stellar::OperationResult& result)
{
    using T = stellar::OperationType;
    switch (result.type) {
    case T::CREATE_ACCOUNT:                   return codeName(result.createAccountResult.code);
    case T::PAYMENT:                          return codeName(result.paymentResult.code);
    case T::PATH_PAYMENT_STRICT_RECEIVE:      return codeName(result.pathPaymentStrictReceiveResult.code);
    case T::MANAGE_SELL_OFFER:                return codeName(result.manageSellOfferResult.code);
    case T::CREATE_PASSIVE_SELL_OFFER:        return codeName(result.createPassiveOfferResult.code);
    case T::SET_OPTIONS:                      return codeName(result.setOptionsResult.code);
    case T::CHANGE_TRUST:                     return codeName(result.changeTrustResult.code);
    case T::ALLOW_TRUST:                      return codeName(result.allowTrustResult.code);
    case T::ACCOUNT_MERGE:                    return codeName(result.accountMergeResult.code);
    case T::INFLATION:                        return codeName(result.inflationResult.code);
    case T::MANAGE_DATA:                      return codeName(result.manageDataResult.code);
    case T::BUMP_SEQUENCE:                    return codeName(result.bumpSequenceResult.code);
    case T::MANAGE_BUY_OFFER:                 return codeName(result.manageBuyOfferResult.code);
    case T::PATH_PAYMENT_STRICT_SEND:         return codeName(result.pathPaymentStrictSendResult.code);
    case T::CREATE_CLAIMABLE_BALANCE:         return codeName(result.createClaimableBalanceResult.code);
    case T::CLAIM_CLAIMABLE_BALANCE:          return codeName(result.claimClaimableBalanceResult.code);
    case T::BEGIN_SPONSORING_FUTURE_RESERVES: return codeName(result.beginSponsoringFutureReservesResult.code);
    case T::END_SPONSORING_FUTURE_RESERVES:   return codeName(result.endSponsoringFutureReservesResult.code);
    case T::REVOKE_SPONSORSHIP:               return codeName(result.revokeSponsorshipResult.code);
    case T::CLAWBACK:                         return codeName(result.clawbackResult.code);
    case T::CLAWBACK_CLAIMABLE_BALANCE:       return codeName(result.clawbackClaimableBalanceResult.code);
    case T::SET_TRUST_LINE_FLAGS:             return codeName(result.setTrustLineFlagsResult.code);
    case T::LIQUIDITY_POOL_DEPOSIT:           return codeName(result.liquidityPoolDepositResult.code);
    case T::LIQUIDITY_POOL_WITHDRAW:          return codeName(result.liquidityPoolWithdrawResult.code);
    case T::INVOKE_HOST_FUNCTION:             return codeName(result.invokeHostFunctionResult.code);
    case T::EXTEND_FOOTPRINT_TTL:             return codeName(result.extendFootprintTTLResult.code);
    case T::RESTORE_FOOTPRINT:                return codeName(result.restoreFootprintResult.code);
    }
    return QStringLiteral("\u2014");
}

QString envelopeTypeName(stellar::EnvelopeType type)
{
    using T = stellar::EnvelopeType;
    switch (type) {
    case T::ENVELOPE_TYPE_TX_V0:     return QStringLiteral("v0 (pre-protocol 13)");
    case T::ENVELOPE_TYPE_TX:        return QStringLiteral("v1");
    case T::ENVELOPE_TYPE_TX_FEE_BUMP: return QStringLiteral("fee bump");
    default: break;
    }
    return QStringLiteral("type %1").arg(static_cast<qint32>(type));
}

// ─── Per-type decoders ───────────────────────────────────────────────

QVariantMap decodeEnvelope(const QByteArray& raw)
{
    QDataStream in(raw);
    stellar::TransactionEnvelope envelope;
    in >> envelope;
    if (in.status() != QDataStream::Ok || !in.atEnd())
        throw std::runtime_error("the envelope is truncated or followed by other bytes");

    QVariantList rows;
    rows.append(field(QStringLiteral("envelope"), envelopeTypeName(envelope.type)));

    if (envelope.type == stellar::EnvelopeType::ENVELOPE_TYPE_TX) {
        const auto& tx = envelope.v1.tx;
        rows.append(field(QStringLiteral("fee"), QString::number(tx.fee)));
        rows.append(field(QStringLiteral("sequence"), QString::number(tx.seqNum)));
        rows.append(field(QStringLiteral("operations"),
                          QString::number(tx.operations.value.size())));
        for (int i = 0; i < tx.operations.value.size(); ++i) {
            rows.append(field(QStringLiteral("  operation %1").arg(i + 1),
                              operationTypeName(tx.operations.value.at(i).type)));
        }
        rows.append(field(QStringLiteral("signatures"),
                          QString::number(envelope.v1.signatures.value.size())));
    } else if (envelope.type == stellar::EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP) {
        rows.append(field(QStringLiteral("fee"), QString::number(envelope.feeBump.tx.fee)));
        rows.append(field(QStringLiteral("signatures"),
                          QString::number(envelope.feeBump.signatures.value.size())));
    } else if (envelope.type == stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0) {
        rows.append(field(QStringLiteral("fee"), QString::number(envelope.v0.tx.fee)));
        rows.append(field(QStringLiteral("sequence"),
                          QString::number(envelope.v0.tx.seqNum)));
        rows.append(field(QStringLiteral("operations"),
                          QString::number(envelope.v0.tx.operations.value.size())));
        rows.append(field(QStringLiteral("signatures"),
                          QString::number(envelope.v0.signatures.value.size())));
    }
    return table(rows);
}

QVariantMap decodeResult(const QByteArray& raw)
{
    QDataStream in(raw);
    stellar::TransactionResult result;
    in >> result;
    if (in.status() != QDataStream::Ok || !in.atEnd())
        throw std::runtime_error("the result is truncated or followed by other bytes");

    QVariantList rows;
    rows.append(field(QStringLiteral("fee charged"), QString::number(result.feeCharged)));
    rows.append(field(QStringLiteral("result"), transactionResultName(result.code)));

    const bool feeBump =
        result.code == stellar::TransactionResultCode::txFEE_BUMP_INNER_SUCCESS
        || result.code == stellar::TransactionResultCode::txFEE_BUMP_INNER_FAILED;

    const QVector<stellar::OperationResult>& operations =
        feeBump ? result.innerResultPair.result.results : result.results;

    if (feeBump) {
        rows.append(field(QStringLiteral("inner result"),
                          transactionResultName(result.innerResultPair.result.code)));
    }

    for (int i = 0; i < operations.size(); ++i) {
        const stellar::OperationResult& op = operations.at(i);
        // A transaction-level code means nothing was attempted; an
        // operation-level one means it was, and this is the operation that
        // refused.
        rows.append(field(QStringLiteral("operation %1").arg(i + 1),
                          operationResultName(op.code) == QLatin1String("op_inner")
                              ? QStringLiteral("%1 \u2014 %2")
                                    .arg(operationTypeName(op.type), operationDetail(op))
                              : operationResultName(op.code)));
    }
    if (operations.isEmpty())
        rows.append(field(QStringLiteral("operations"),
                          QStringLiteral("none \u2014 the transaction never ran")));
    return table(rows);
}

QVariantMap decodeScVal(const QByteArray& raw)
{
    QDataStream in(raw);
    stellar::SCVal value;
    in >> value;
    if (in.status() != QDataStream::Ok || !in.atEnd())
        throw std::runtime_error("the value is truncated or followed by other bytes");

    QVariantList rows;
    rows.append(field(QStringLiteral("type"),
                      QString::number(static_cast<qint32>(value.type))));
    try {
        switch (value.type) {
        case stellar::SCValType::SCV_BOOL:
            rows.append(field(QStringLiteral("value"),
                              value.b ? QStringLiteral("true") : QStringLiteral("false")));
            break;
        case stellar::SCValType::SCV_U32:
            rows.append(field(QStringLiteral("value"), QString::number(value.u32))); break;
        case stellar::SCValType::SCV_I32:
            rows.append(field(QStringLiteral("value"), QString::number(value.i32))); break;
        case stellar::SCValType::SCV_U64:
            rows.append(field(QStringLiteral("value"), QString::number(value.u64))); break;
        case stellar::SCValType::SCV_I64:
            rows.append(field(QStringLiteral("value"), QString::number(value.i64))); break;
        case stellar::SCValType::SCV_TIMEPOINT:
            rows.append(field(QStringLiteral("value"), QString::number(value.timepoint))); break;
        case stellar::SCValType::SCV_DURATION:
            rows.append(field(QStringLiteral("value"), QString::number(value.duration))); break;
        case stellar::SCValType::SCV_U128:
        case stellar::SCValType::SCV_I128:
        case stellar::SCValType::SCV_U256:
        case stellar::SCValType::SCV_I256:
            // Exact decimal, never a double: a token balance is an i128.
            rows.append(field(QStringLiteral("value"), Scv::decimalText(value))); break;
        case stellar::SCValType::SCV_SYMBOL:
            rows.append(field(QStringLiteral("symbol"), Scv::fromSymbol(value))); break;
        case stellar::SCValType::SCV_STRING:
            rows.append(field(QStringLiteral("string"), Scv::fromString(value))); break;
        case stellar::SCValType::SCV_ADDRESS:
            rows.append(field(QStringLiteral("address"),
                              Scv::fromAddress(value).toString())); break;
        case stellar::SCValType::SCV_BYTES:
            rows.append(field(QStringLiteral("bytes"),
                              QString::fromLatin1(Scv::fromBytes(value).toHex()))); break;
        case stellar::SCValType::SCV_VEC:
            rows.append(field(QStringLiteral("elements"),
                              QString::number(value.vec ? value.vec->size() : 0))); break;
        case stellar::SCValType::SCV_MAP:
            rows.append(field(QStringLiteral("entries"),
                              QString::number(value.map ? value.map->size() : 0))); break;
        default:
            break;
        }
    } catch (const std::exception&) {
        rows.append(field(QStringLiteral("value"), QStringLiteral("(unreadable)")));
    }
    return table(rows);
}

QVariantMap decodeSorobanData(const QByteArray& raw)
{
    QDataStream in(raw);
    stellar::SorobanTransactionData data;
    in >> data;
    if (in.status() != QDataStream::Ok || !in.atEnd())
        throw std::runtime_error("the resource data is truncated or followed by other bytes");

    QVariantList rows;
    rows.append(field(QStringLiteral("resource fee"), QString::number(data.resourceFee)));
    rows.append(field(QStringLiteral("instructions"),
                      QString::number(data.resources.instructions)));
    rows.append(field(QStringLiteral("read bytes"),
                      QString::number(data.resources.diskReadBytes)));
    rows.append(field(QStringLiteral("write bytes"),
                      QString::number(data.resources.writeBytes)));
    rows.append(field(QStringLiteral("footprint, read"),
                      QString::number(data.resources.footprint.readOnly.size())));
    rows.append(field(QStringLiteral("footprint, written"),
                      QString::number(data.resources.footprint.readWrite.size())));
    return table(rows);
}

QVariantMap decodeContractSpec(const QByteArray& raw)
{
    QDataStream in(raw);
    QVariantList rows;
    int seen = 0;
    while (!in.atEnd()) {
        stellar::SCSpecEntry entry;
        in >> entry;
        if (in.status() != QDataStream::Ok)
            throw std::runtime_error("the spec stream is truncated");
        ++seen;
        if (entry.kind == stellar::SCSpecEntryKind::SC_SPEC_ENTRY_FUNCTION_V0) {
            rows.append(field(QStringLiteral("function"),
                              QString::fromUtf8(entry.functionV0.name)));
        } else {
            rows.append(field(QStringLiteral("entry %1").arg(seen),
                              QStringLiteral("kind %1")
                                  .arg(static_cast<qint32>(entry.kind))));
        }
    }
    if (rows.isEmpty())
        rows.append(field(QStringLiteral("entries"), QStringLiteral("none")));
    return table(rows);
}

} // namespace

XdrQml::XdrQml(QObject* parent) : QObject(parent) {}

bool XdrQml::looksLikeBase64(const QString& base64) const
{
    // AbortOnBase64DecodingErrors is the whole point. Without it Qt does what
    // the rest of the SDK does — silently discards characters it does not
    // recognise — so "@@@@" would come back as a perfectly good empty answer
    // and this function would always say yes.
    const auto result = QByteArray::fromBase64Encoding(
        base64Text(base64),
        QByteArray::Base64Encoding | QByteArray::AbortOnBase64DecodingErrors);
    return result.decodingStatus == QByteArray::Base64DecodingStatus::Ok;
}

QVariantList XdrQml::supportedTypes() const
{
    auto entry = [](const char* name, const char* label, const char* note) {
        QVariantMap out;
        out.insert(QStringLiteral("name"), QString::fromLatin1(name));
        out.insert(QStringLiteral("label"), QString::fromLatin1(label));
        out.insert(QStringLiteral("note"), QString::fromUtf8(note));
        return QVariant(out);
    };

    return {
        entry("envelope", "Transaction envelope",
              "What you submit: the transaction and its signatures. Horizon "
              "calls this envelope_xdr."),
        entry("result", "Transaction result",
              "What came back. This is where the real reason a transaction "
              "failed lives — Horizon calls it result_xdr."),
        entry("scval", "Contract value (SCVal)",
              "A single Soroban value: what a contract function returned, or "
              "one of its arguments."),
        entry("sorobanData", "Soroban resource data",
              "The footprint and the resource budget attached to a Soroban "
              "transaction. Simulation is what produces it."),
        entry("contractSpec", "Contract interface",
              "The contractspecv0 section of a compiled contract: a stream of "
              "entries, one per exported function and type."),
    };
}

QVariantMap XdrQml::decode(const QString& type, const QString& base64) const
{
    QVariantMap out;
    try {
        // Nothing pasted but the whitespace that is skipped: a vertical tab
        // or a non-breaking space alone is text that is not base64, as it is
        // anywhere else.
        if (base64Text(base64).isEmpty())
            throw std::runtime_error("paste some base64 first");
        if (!looksLikeBase64(base64)) {
            // Worth its own message, before any decoding: what was pasted
            // is not base64 at all, whatever type was asked for.
            throw std::runtime_error("that is not valid base64");
        }

        const QByteArray raw = decodeBase64(base64);
        if (type == QLatin1String("envelope"))          out = decodeEnvelope(raw);
        else if (type == QLatin1String("result"))       out = decodeResult(raw);
        else if (type == QLatin1String("scval"))        out = decodeScVal(raw);
        else if (type == QLatin1String("sorobanData"))  out = decodeSorobanData(raw);
        else if (type == QLatin1String("contractSpec")) out = decodeContractSpec(raw);
        else
            throw std::runtime_error("nothing here decodes that type");

        out.insert(QStringLiteral("bytes"), raw.size());
    } catch (const std::exception& e) {
        out.clear();
        // XDR has no self-describing header, so the commonest mistake by far is
        // the right bytes read as the wrong type. Say so.
        out.insert(QStringLiteral("error"),
                   QString::fromUtf8(e.what())
                       + QStringLiteral(". XDR carries no type tag, so if the "
                                        "bytes are right the type may be wrong."));
    } catch (...) {
        out.clear();
        out.insert(QStringLiteral("error"), QStringLiteral("those bytes could not be read"));
    }
    return out;
}

QSTELLAR_END_NS
