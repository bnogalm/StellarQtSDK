#include "stellartransaction.h"

stellar::Operation::Operation(){
    type = stellar::OperationType::CREATE_ACCOUNT;//we set a trivial type, so correct destructor is called
    new (&operationCreateAccount) CreateAccountOp();    // and zero it: the arm `type` names is written as is
}

stellar::Operation::Operation(const stellar::Operation &op){
    sourceAccount = op.sourceAccount;
    type = op.type;
    switch(op.type){
    case OperationType::CREATE_ACCOUNT:
        operationCreateAccount = op.operationCreateAccount; break;
    case OperationType::PAYMENT:
        operationPayment = op.operationPayment; break;
    case OperationType::PATH_PAYMENT_STRICT_RECEIVE:
        new (&operationPathPaymentStrictReceive) PathPaymentStrictReceiveOp();
        operationPathPaymentStrictReceive = op.operationPathPaymentStrictReceive; break;
    case OperationType::MANAGE_SELL_OFFER:
        operationManageSellOffer = op.operationManageSellOffer; break;
    case OperationType::CREATE_PASSIVE_SELL_OFFER:
        operationCreatePassiveSellOffer = op.operationCreatePassiveSellOffer; break;
    case OperationType::SET_OPTIONS:
        new (&operationSetOptions) SetOptionsOp();
        operationSetOptions = op.operationSetOptions; break;
    case OperationType::CHANGE_TRUST:
        new (&operationChangeTrust) ChangeTrustOp();
        operationChangeTrust = op.operationChangeTrust; break;
    case OperationType::ALLOW_TRUST:
        operationAllowTrust = op.operationAllowTrust; break;
    case OperationType::ACCOUNT_MERGE:
        operationAccountMerge = op.operationAccountMerge; break;
    case OperationType::INFLATION:
        break;                       // void: there is nothing to copy
    case OperationType::MANAGE_DATA:
        new (&operationManageData) ManageDataOp();
        operationManageData = op.operationManageData; break;
    case OperationType::BUMP_SEQUENCE:
        operationBumpSequence = op.operationBumpSequence; break;
    case OperationType::MANAGE_BUY_OFFER:
        operationManageBuyOffer = op.operationManageBuyOffer; break;
    case OperationType::PATH_PAYMENT_STRICT_SEND:
        new (&operationPathPaymentStrictSend) PathPaymentStrictSendOp();
        operationPathPaymentStrictSend = op.operationPathPaymentStrictSend; break;
    case OperationType::CREATE_CLAIMABLE_BALANCE:
        new (&operationCreateClaimableBalance) CreateClaimableBalanceOp();
        operationCreateClaimableBalance = op.operationCreateClaimableBalance; break;
    case OperationType::CLAIM_CLAIMABLE_BALANCE:
        operationClaimClaimableBalance = op.operationClaimClaimableBalance; break;
    case OperationType::BEGIN_SPONSORING_FUTURE_RESERVES:
        operationBeginSponsoringFutureReserves = op.operationBeginSponsoringFutureReserves; break;
   case OperationType::END_SPONSORING_FUTURE_RESERVES:
       break;
    case OperationType::REVOKE_SPONSORSHIP:
        new (&operationRevokeSponsorship) RevokeSponsorshipOp();
        operationRevokeSponsorship = op.operationRevokeSponsorship; break;
    case OperationType::CLAWBACK:
        operationClawback = op.operationClawback; break;
    case OperationType::CLAWBACK_CLAIMABLE_BALANCE:
        new (&operationClawbackClaimableBalance) ClawbackClaimableBalanceOp();
        operationClawbackClaimableBalance = op.operationClawbackClaimableBalance; break;
    case OperationType::SET_TRUST_LINE_FLAGS:
        operationSetTrustLineFlags = op.operationSetTrustLineFlags; break;
    case OperationType::LIQUIDITY_POOL_DEPOSIT:
        operationLiquidityPoolDeposit = op.operationLiquidityPoolDeposit; break;
    case OperationType::LIQUIDITY_POOL_WITHDRAW:
        operationLiquidityPoolWithdraw = op.operationLiquidityPoolWithdraw; break;
    case OperationType::INVOKE_HOST_FUNCTION:
        new (&operationInvokeHostFunction) InvokeHostFunctionOp();
        operationInvokeHostFunction = op.operationInvokeHostFunction; break;
    case OperationType::EXTEND_FOOTPRINT_TTL:
        operationExtendFootprintTtl = op.operationExtendFootprintTtl; break;
    case OperationType::RESTORE_FOOTPRINT:
        operationRestoreFootprint = op.operationRestoreFootprint; break;
    //default: break;
    }
}

void stellar::Operation::clear()
{
    switch(type)
    {
    case OperationType::PATH_PAYMENT_STRICT_RECEIVE:
        operationPathPaymentStrictReceive.~PathPaymentStrictReceiveOp();
        break;
    case OperationType::SET_OPTIONS:
        operationSetOptions.~SetOptionsOp();
        break;
    case OperationType::MANAGE_DATA:
        operationManageData.~ManageDataOp();
        break;
    case OperationType::PATH_PAYMENT_STRICT_SEND:
        operationPathPaymentStrictSend.~PathPaymentStrictSendOp();
        break;
    case OperationType::REVOKE_SPONSORSHIP:
        operationRevokeSponsorship.~RevokeSponsorshipOp();
        break;
    case OperationType::CLAWBACK_CLAIMABLE_BALANCE:
        operationClawbackClaimableBalance.~ClawbackClaimableBalanceOp();
        break;
    case OperationType::CHANGE_TRUST:
        operationChangeTrust.~ChangeTrustOp();
        break;
    case OperationType::INVOKE_HOST_FUNCTION:
        operationInvokeHostFunction.~InvokeHostFunctionOp();
        break;
    case OperationType::CREATE_CLAIMABLE_BALANCE:
        operationCreateClaimableBalance.~CreateClaimableBalanceOp();
        break;
    default:
        break;
    }
}

stellar::Operation::~Operation(){
    clear();
}

const stellar::Operation &stellar::Operation::operator =(const stellar::Operation &op) {
    if (this == &op)
        return *this;       // clear() below would wipe the source too
    clear();
    sourceAccount = op.sourceAccount;
    type = op.type;
    switch(op.type){
    case OperationType::CREATE_ACCOUNT:
        operationCreateAccount = op.operationCreateAccount; break;
    case OperationType::PAYMENT:
        operationPayment = op.operationPayment; break;
    case OperationType::PATH_PAYMENT_STRICT_RECEIVE:
        new (&operationPathPaymentStrictReceive) PathPaymentStrictReceiveOp();
        operationPathPaymentStrictReceive = op.operationPathPaymentStrictReceive; break;
    case OperationType::MANAGE_SELL_OFFER:
        operationManageSellOffer = op.operationManageSellOffer; break;
    case OperationType::CREATE_PASSIVE_SELL_OFFER:
        operationCreatePassiveSellOffer = op.operationCreatePassiveSellOffer; break;
    case OperationType::SET_OPTIONS:
        new (&operationSetOptions) SetOptionsOp();
        operationSetOptions = op.operationSetOptions; break;
    case OperationType::CHANGE_TRUST:
        new (&operationChangeTrust) ChangeTrustOp();
        operationChangeTrust = op.operationChangeTrust; break;
    case OperationType::ALLOW_TRUST:
        operationAllowTrust = op.operationAllowTrust; break;
    case OperationType::ACCOUNT_MERGE:
        operationAccountMerge = op.operationAccountMerge; break;
    case OperationType::INFLATION:
        break;                       // void: there is nothing to copy
    case OperationType::MANAGE_DATA:
        new (&operationManageData) ManageDataOp();
        operationManageData = op.operationManageData; break;
    case OperationType::BUMP_SEQUENCE:
        operationBumpSequence = op.operationBumpSequence; break;
    case OperationType::MANAGE_BUY_OFFER:
        operationManageBuyOffer = op.operationManageBuyOffer; break;
    case OperationType::PATH_PAYMENT_STRICT_SEND:
        new (&operationPathPaymentStrictSend) PathPaymentStrictSendOp();
        operationPathPaymentStrictSend = op.operationPathPaymentStrictSend; break;
    case OperationType::CREATE_CLAIMABLE_BALANCE:
        new (&operationCreateClaimableBalance) CreateClaimableBalanceOp();
        operationCreateClaimableBalance = op.operationCreateClaimableBalance; break;
    case OperationType::CLAIM_CLAIMABLE_BALANCE:
        operationClaimClaimableBalance = op.operationClaimClaimableBalance; break;
    case OperationType::BEGIN_SPONSORING_FUTURE_RESERVES:
        operationBeginSponsoringFutureReserves = op.operationBeginSponsoringFutureReserves; break;
    case OperationType::END_SPONSORING_FUTURE_RESERVES:
       break;
    case OperationType::REVOKE_SPONSORSHIP:
        new (&operationRevokeSponsorship) RevokeSponsorshipOp();
        operationRevokeSponsorship = op.operationRevokeSponsorship; break;
    case OperationType::CLAWBACK:
        operationClawback = op.operationClawback; break;
    case OperationType::CLAWBACK_CLAIMABLE_BALANCE:
        new (&operationClawbackClaimableBalance) ClawbackClaimableBalanceOp();
        operationClawbackClaimableBalance = op.operationClawbackClaimableBalance; break;
    case OperationType::SET_TRUST_LINE_FLAGS:
        operationSetTrustLineFlags = op.operationSetTrustLineFlags; break;
    case OperationType::LIQUIDITY_POOL_DEPOSIT:
        operationLiquidityPoolDeposit = op.operationLiquidityPoolDeposit; break;
    case OperationType::LIQUIDITY_POOL_WITHDRAW:
        operationLiquidityPoolWithdraw = op.operationLiquidityPoolWithdraw; break;
    case OperationType::INVOKE_HOST_FUNCTION:
        new (&operationInvokeHostFunction) InvokeHostFunctionOp();
        operationInvokeHostFunction = op.operationInvokeHostFunction; break;
    case OperationType::EXTEND_FOOTPRINT_TTL:
        operationExtendFootprintTtl = op.operationExtendFootprintTtl; break;
    case OperationType::RESTORE_FOOTPRINT:
        operationRestoreFootprint = op.operationRestoreFootprint; break;
    //default: break;
    }
    return *this;
}

stellar::PathPaymentStrictReceiveOp &stellar::Operation::fillPathPaymentStrictReceiveOp()
{
    if(type!=OperationType::PATH_PAYMENT_STRICT_RECEIVE)
    {
        clear();
        type=OperationType::PATH_PAYMENT_STRICT_RECEIVE;
        new (&operationPathPaymentStrictReceive) PathPaymentStrictReceiveOp();
    }
    return operationPathPaymentStrictReceive;
}

stellar::SetOptionsOp &stellar::Operation::fillSetOptionsOp()
{
    if(type!=OperationType::SET_OPTIONS)
    {
        clear();
        type=OperationType::SET_OPTIONS;
        new (&operationSetOptions) SetOptionsOp();
    }
    return operationSetOptions;
}

stellar::ManageDataOp &stellar::Operation::fillManageDataOp()
{
    if(type!=OperationType::MANAGE_DATA)
    {
        clear();
        type=OperationType::MANAGE_DATA;
        new (&operationManageData) ManageDataOp();
    }
    return operationManageData;
}

stellar::PathPaymentStrictSendOp &stellar::Operation::fillPathPaymentStrictSendOp()
{
    if(type!=OperationType::PATH_PAYMENT_STRICT_SEND)
    {
        clear();
        type=OperationType::PATH_PAYMENT_STRICT_SEND;
        new (&operationPathPaymentStrictSend) PathPaymentStrictSendOp();
    }
    return operationPathPaymentStrictSend;
}

stellar::RevokeSponsorshipOp &stellar::Operation::fillRevokeSponsorshipOp()
{
    if(type!=OperationType::REVOKE_SPONSORSHIP)
    {
        clear();
        type=OperationType::REVOKE_SPONSORSHIP;
        new (&operationRevokeSponsorship) RevokeSponsorshipOp();
    }
    return operationRevokeSponsorship;
}

stellar::ChangeTrustOp &stellar::Operation::fillChangeTrustOp()
{
    if(type!=OperationType::CHANGE_TRUST)
    {
        clear();
        type=OperationType::CHANGE_TRUST;
        new (&operationChangeTrust) ChangeTrustOp();
    }
    return operationChangeTrust;
}

stellar::InvokeHostFunctionOp &stellar::Operation::fillInvokeHostFunctionOp()
{
    if(type!=OperationType::INVOKE_HOST_FUNCTION)
    {
        clear();
        type=OperationType::INVOKE_HOST_FUNCTION;
        new (&operationInvokeHostFunction) InvokeHostFunctionOp();
    }
    return operationInvokeHostFunction;
}

stellar::CreateClaimableBalanceOp &stellar::Operation::fillCreateClaimableBalanceOp()
{
    if(type!=OperationType::CREATE_CLAIMABLE_BALANCE)
    {
        clear();
        type=OperationType::CREATE_CLAIMABLE_BALANCE;
        new (&operationCreateClaimableBalance) CreateClaimableBalanceOp();
    }
    return operationCreateClaimableBalance;
}

stellar::OperationResult::OperationResult()
    : code(OperationResultCode::opINNER)
    , type(OperationType::CREATE_ACCOUNT)
{
    // The arm `type` names, set too: the code was left as whatever memory held.
    createAccountResult.code = CreateAccountResultCode::CREATE_ACCOUNT_SUCCESS;
}
//CreateAccountResult createAccountResult;
//SetOptionsResult setOptionsResult;
//ChangeTrustResult changeTrustResult;
//AllowTrustResult allowTrustResult;
//AccountMergeResult accountMergeResult;
//ManageDataResult manageDataResult;
//BumpSequenceResult bumpSequenceResult;
//};
////no trivial
//ManageBuyOfferResult manageBuyOfferResult;
//PaymentResult paymentResult;
//PathPaymentResult pathPaymentResult; Send and Receive
//ManageSellOfferResult manageSellOfferResult;
//ManageSellOfferResult createPassiveOfferResult;
//InflationResult inflationResult;
stellar::OperationResult::OperationResult(const stellar::OperationResult &op)
{
    code = op.code;
    type = op.type;
    switch(op.type){
    case OperationType::CREATE_ACCOUNT:
        createAccountResult = op.createAccountResult; break;
    case OperationType::PAYMENT:
        new (&paymentResult) PaymentResult();
        paymentResult = op.paymentResult; break;
    case OperationType::PATH_PAYMENT_STRICT_RECEIVE:
        new (&pathPaymentStrictReceiveResult) PathPaymentStrictReceiveResult();
        pathPaymentStrictReceiveResult = op.pathPaymentStrictReceiveResult; break;
    case OperationType::MANAGE_SELL_OFFER:
        new (&manageSellOfferResult) ManageSellOfferResult();
        manageSellOfferResult = op.manageSellOfferResult; break;
    case OperationType::CREATE_PASSIVE_SELL_OFFER:
        new (&createPassiveOfferResult) ManageSellOfferResult();
        createPassiveOfferResult = op.createPassiveOfferResult; break;
    case OperationType::SET_OPTIONS:
        setOptionsResult = op.setOptionsResult; break;
    case OperationType::CHANGE_TRUST:
        changeTrustResult = op.changeTrustResult; break;
    case OperationType::ALLOW_TRUST:
        allowTrustResult = op.allowTrustResult; break;
    case OperationType::ACCOUNT_MERGE:
        accountMergeResult = op.accountMergeResult; break;
    case OperationType::INFLATION:
        new (&inflationResult) InflationResult();
        inflationResult = op.inflationResult; break;
    case OperationType::MANAGE_DATA:
        manageDataResult = op.manageDataResult; break;
    case OperationType::BUMP_SEQUENCE:
        bumpSequenceResult = op.bumpSequenceResult; break;
    case OperationType::MANAGE_BUY_OFFER:
        new (&manageBuyOfferResult) ManageBuyOfferResult();
        manageBuyOfferResult = op.manageBuyOfferResult; break;
    case OperationType::PATH_PAYMENT_STRICT_SEND:
        new (&pathPaymentStrictSendResult) PathPaymentStrictSendResult();
        pathPaymentStrictSendResult = op.pathPaymentStrictSendResult; break;
    case OperationType::CREATE_CLAIMABLE_BALANCE:
        createClaimableBalanceResult = op.createClaimableBalanceResult; break;
    case OperationType::CLAWBACK:
        clawbackResult = op.clawbackResult; break;
    case OperationType::CLAWBACK_CLAIMABLE_BALANCE:
        clawbackClaimableBalanceResult = op.clawbackClaimableBalanceResult; break;
    case OperationType::SET_TRUST_LINE_FLAGS:
        setTrustLineFlagsResult = op.setTrustLineFlagsResult; break;
    case OperationType::CLAIM_CLAIMABLE_BALANCE:
        claimClaimableBalanceResult = op.claimClaimableBalanceResult; break;
    case OperationType::BEGIN_SPONSORING_FUTURE_RESERVES:
        beginSponsoringFutureReservesResult = op.beginSponsoringFutureReservesResult; break;
    case OperationType::END_SPONSORING_FUTURE_RESERVES:
        endSponsoringFutureReservesResult = op.endSponsoringFutureReservesResult; break;
    case OperationType::REVOKE_SPONSORSHIP:
        revokeSponsorshipResult = op.revokeSponsorshipResult; break;
    case OperationType::LIQUIDITY_POOL_DEPOSIT:
        liquidityPoolDepositResult = op.liquidityPoolDepositResult; break;
    case OperationType::LIQUIDITY_POOL_WITHDRAW:
        liquidityPoolWithdrawResult = op.liquidityPoolWithdrawResult; break;
    case OperationType::INVOKE_HOST_FUNCTION:
        invokeHostFunctionResult = op.invokeHostFunctionResult; break;
    case OperationType::EXTEND_FOOTPRINT_TTL:
        extendFootprintTTLResult = op.extendFootprintTTLResult; break;
    case OperationType::RESTORE_FOOTPRINT:
        restoreFootprintResult = op.restoreFootprintResult; break;
    default: break;
    }
}

stellar::OperationResult::~OperationResult()
{
    clear();
}

void stellar::OperationResult::clear()
{
    switch(type)
    {
    case OperationType::PAYMENT:
        paymentResult.~PaymentResult();
        break;
    case OperationType::PATH_PAYMENT_STRICT_RECEIVE:
        pathPaymentStrictReceiveResult.~PathPaymentStrictReceiveResult();
        break;
    case OperationType::MANAGE_SELL_OFFER:
        manageSellOfferResult.~ManageSellOfferResult();
        break;
    case OperationType::CREATE_PASSIVE_SELL_OFFER:
        createPassiveOfferResult.~ManageSellOfferResult();
        break;
    case OperationType::INFLATION:
        inflationResult.~InflationResult();
        break;
    case OperationType::MANAGE_BUY_OFFER:
        manageBuyOfferResult.~ManageBuyOfferResult();
        break;
    case OperationType::PATH_PAYMENT_STRICT_SEND:
        pathPaymentStrictSendResult.~PathPaymentStrictSendResult();
        break;
    default:
        break;
    }
    type = OperationType::CREATE_ACCOUNT;   // trivial: no arm left to destroy
}

const stellar::OperationResult &stellar::OperationResult::operator =(const stellar::OperationResult &op)
{
    // Without this, r = r destroyed the live arm, rebuilt it empty and then
    // copied that empty arm onto itself: a created offer came back with id 0.
    if (this == &op)
        return *this;
    clear();
    code = op.code;
    type = op.type;
    switch(op.type){
    case OperationType::CREATE_ACCOUNT:
        createAccountResult = op.createAccountResult; break;
    case OperationType::PAYMENT:
        new (&paymentResult) PaymentResult();
        paymentResult = op.paymentResult; break;
    case OperationType::PATH_PAYMENT_STRICT_RECEIVE:
        new (&pathPaymentStrictReceiveResult) PathPaymentStrictReceiveResult();
        pathPaymentStrictReceiveResult = op.pathPaymentStrictReceiveResult; break;
    case OperationType::MANAGE_SELL_OFFER:
        new (&manageSellOfferResult) ManageSellOfferResult();
        manageSellOfferResult = op.manageSellOfferResult; break;
    case OperationType::CREATE_PASSIVE_SELL_OFFER:
        new (&createPassiveOfferResult) ManageSellOfferResult();
        createPassiveOfferResult = op.createPassiveOfferResult; break;
    case OperationType::SET_OPTIONS:
        setOptionsResult = op.setOptionsResult; break;
    case OperationType::CHANGE_TRUST:
        changeTrustResult = op.changeTrustResult; break;
    case OperationType::ALLOW_TRUST:
        allowTrustResult = op.allowTrustResult; break;
    case OperationType::ACCOUNT_MERGE:
        accountMergeResult = op.accountMergeResult; break;
    case OperationType::INFLATION:
        new (&inflationResult) InflationResult();
        inflationResult = op.inflationResult; break;
    case OperationType::MANAGE_DATA:
        manageDataResult = op.manageDataResult; break;
    case OperationType::BUMP_SEQUENCE:
        bumpSequenceResult = op.bumpSequenceResult; break;
    case OperationType::MANAGE_BUY_OFFER:
        new (&manageBuyOfferResult) ManageBuyOfferResult();
        manageBuyOfferResult = op.manageBuyOfferResult; break;
    case OperationType::PATH_PAYMENT_STRICT_SEND:
        new (&pathPaymentStrictSendResult) PathPaymentStrictSendResult();
        pathPaymentStrictSendResult = op.pathPaymentStrictSendResult; break;
    case OperationType::CREATE_CLAIMABLE_BALANCE:
        createClaimableBalanceResult = op.createClaimableBalanceResult; break;
    case OperationType::CLAWBACK:
        clawbackResult = op.clawbackResult; break;
    case OperationType::CLAWBACK_CLAIMABLE_BALANCE:
        clawbackClaimableBalanceResult = op.clawbackClaimableBalanceResult; break;
    case OperationType::SET_TRUST_LINE_FLAGS:
        setTrustLineFlagsResult = op.setTrustLineFlagsResult; break;
    case OperationType::CLAIM_CLAIMABLE_BALANCE:
        claimClaimableBalanceResult = op.claimClaimableBalanceResult; break;
    case OperationType::BEGIN_SPONSORING_FUTURE_RESERVES:
        beginSponsoringFutureReservesResult = op.beginSponsoringFutureReservesResult; break;
    case OperationType::END_SPONSORING_FUTURE_RESERVES:
        endSponsoringFutureReservesResult = op.endSponsoringFutureReservesResult; break;
    case OperationType::REVOKE_SPONSORSHIP:
        revokeSponsorshipResult = op.revokeSponsorshipResult; break;
    case OperationType::LIQUIDITY_POOL_DEPOSIT:
        liquidityPoolDepositResult = op.liquidityPoolDepositResult; break;
    case OperationType::LIQUIDITY_POOL_WITHDRAW:
        liquidityPoolWithdrawResult = op.liquidityPoolWithdrawResult; break;
    case OperationType::INVOKE_HOST_FUNCTION:
        invokeHostFunctionResult = op.invokeHostFunctionResult; break;
    case OperationType::EXTEND_FOOTPRINT_TTL:
        extendFootprintTTLResult = op.extendFootprintTTLResult; break;
    case OperationType::RESTORE_FOOTPRINT:
        restoreFootprintResult = op.restoreFootprintResult; break;
    default: break;
    }
    return *this;
}

stellar::TransactionEnvelope::TransactionEnvelope():type(EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP)
{
    // The arm `type` names must exist: destroying (or assigning into) a
    // default envelope used to run the fee bump's destructor on raw memory.
    new (&feeBump) FeeBumpTransactionEnvelope();
}

stellar::TransactionEnvelope::TransactionEnvelope(const stellar::TransactionV0Envelope& t)
    :type(EnvelopeType::ENVELOPE_TYPE_TX_V0)
  ,v0(t)
{
}

stellar::TransactionEnvelope::TransactionEnvelope(const stellar::TransactionV1Envelope& t)
    :type(EnvelopeType::ENVELOPE_TYPE_TX)
  ,v1(t)
{
}

stellar::TransactionEnvelope::TransactionEnvelope(const stellar::FeeBumpTransactionEnvelope& t)
    :type(EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP)
    ,feeBump(t)
{
}

stellar::TransactionEnvelope::TransactionEnvelope(const stellar::TransactionEnvelope &te)
{
    type = te.type;
    switch(te.type){
    case EnvelopeType::ENVELOPE_TYPE_TX_V0:
        new (&v0) TransactionV0Envelope();
        v0 = te.v0; break;
    case EnvelopeType::ENVELOPE_TYPE_TX:
        new (&v1) TransactionV1Envelope();
        v1 = te.v1; break;
    case EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
        new (&feeBump) FeeBumpTransactionEnvelope();
        feeBump = te.feeBump; break;
    default: break;
    }
}

stellar::TransactionEnvelope::~TransactionEnvelope()
{
    clear();
}

void stellar::TransactionEnvelope::clear()
{
    switch(type){
    case EnvelopeType::ENVELOPE_TYPE_TX_V0:
        v0.~TransactionV0Envelope();break;
    case EnvelopeType::ENVELOPE_TYPE_TX:
        v1.~TransactionV1Envelope();break;
    case EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
        feeBump.~FeeBumpTransactionEnvelope();break;
    default: break;
    }
}

const stellar::TransactionEnvelope &stellar::TransactionEnvelope::operator =(const stellar::TransactionEnvelope &te)
{
    // The fee-bump arm used to be neither destroyed (a leak) nor constructed:
    // a fee bump was assigned into memory that held another arm, or nothing,
    // and crashed. And e = e emptied the envelope.
    if (this == &te)
        return *this;
    clear();
    type = te.type;
    switch(te.type){
    case EnvelopeType::ENVELOPE_TYPE_TX_V0:
        new (&v0) TransactionV0Envelope(te.v0); break;
    case EnvelopeType::ENVELOPE_TYPE_TX:
        new (&v1) TransactionV1Envelope(te.v1); break;
    case EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
        new (&feeBump) FeeBumpTransactionEnvelope(te.feeBump); break;
    default: break;
    }
    return *this;
}

const stellar::FeeBumpTransactionEnvelope &stellar::FeeBumpTransactionEnvelope::operator =(const stellar::FeeBumpTransactionEnvelope &te)
{
    tx=te.tx;
    signatures=te.signatures;
    return *this;
}


const stellar::FeeBumpTransaction &stellar::FeeBumpTransaction::operator =(const stellar::FeeBumpTransaction &fbt)
{
    if (this == &fbt)
        return *this;
    feeSource = fbt.feeSource;
    fee = fbt.fee;
    type = fbt.type;
    // v1 is always live (every constructor builds it, the destructor destroys
    // it): assign it. Building a new one over it leaked the old one.
    v1 = fbt.v1;
    ext = fbt.ext;
    return *this;
}

stellar::FeeBumpTransaction::FeeBumpTransaction()
    : fee(0)
    , type(EnvelopeType::ENVELOPE_TYPE_TX)      // the only inner type, and the arm built below
{
    new (&v1) TransactionV1Envelope();
}

stellar::FeeBumpTransaction::FeeBumpTransaction(const stellar::FeeBumpTransaction &fbt)
    :feeSource(fbt.feeSource)
    ,fee(fbt.fee)
    ,type(fbt.type)
    ,ext(fbt.ext)
{
    new (&v1) TransactionV1Envelope(fbt.v1);
}

stellar::FeeBumpTransaction::~FeeBumpTransaction()
{
    v1.~TransactionV1Envelope();
}
