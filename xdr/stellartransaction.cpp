#include "stellartransaction.h"

stellar::Operation::Operation(){
    type = stellar::OperationType::CREATE_ACCOUNT;//we set a trivial type, so correct destructor is called
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
        operationChangeTrust = op.operationChangeTrust; break;
    case OperationType::ALLOW_TRUST:
        operationAllowTrust = op.operationAllowTrust; break;
    case OperationType::ACCOUNT_MERGE:
        operationAccountMerge = op.operationAccountMerge; break;
    case OperationType::INFLATION:
        operationInflation = op.operationInflation; break;
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
    default: break;
    }
}

stellar::Operation::~Operation(){
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
    default:
        break;
    }
}

const stellar::Operation &stellar::Operation::operator =(const stellar::Operation &op) {
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
    default:
        break;
    }
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
        operationChangeTrust = op.operationChangeTrust; break;
    case OperationType::ALLOW_TRUST:
        operationAllowTrust = op.operationAllowTrust; break;
    case OperationType::ACCOUNT_MERGE:
        operationAccountMerge = op.operationAccountMerge; break;
    case OperationType::INFLATION:
        operationInflation = op.operationInflation; break;
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
    default: break;
    }
    return *this;
}

stellar::OperationResult::OperationResult():type(OperationType::CREATE_ACCOUNT)
{

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
    default: break;
    }
}

stellar::OperationResult::~OperationResult()
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
}

const stellar::OperationResult &stellar::OperationResult::operator =(const stellar::OperationResult &op)
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
    default:
        break;
    }
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
    default: break;
    }
    return *this;
}
