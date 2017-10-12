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
    case OperationType::PATH_PAYMENT:
        new (&operationPathPayment) PathPaymentOp();
        operationPathPayment = op.operationPathPayment; break;
    case OperationType::MANAGE_OFFER:
        operationManageOffer = op.operationManageOffer; break;
    case OperationType::CREATE_PASSIVE_OFFER:
        operationCreatePassiveOffer = op.operationCreatePassiveOffer; break;
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
    default: break;
    }
}

stellar::Operation::~Operation(){
    switch(type)
    {
    case OperationType::PATH_PAYMENT:
        operationPathPayment.~PathPaymentOp();
        break;
    case OperationType::SET_OPTIONS:
        operationSetOptions.~SetOptionsOp();
        break;
    case OperationType::MANAGE_DATA:
        operationManageData.~ManageDataOp();
        break;
    default:
        break;
    }
}

const stellar::Operation &stellar::Operation::operator =(const stellar::Operation &op) {
    switch(type)
    {
    case OperationType::PATH_PAYMENT:
        operationPathPayment.~PathPaymentOp();
        break;
    case OperationType::SET_OPTIONS:
        operationSetOptions.~SetOptionsOp();
        break;
    case OperationType::MANAGE_DATA:
        operationManageData.~ManageDataOp();
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
    case OperationType::PATH_PAYMENT:
        new (&operationPathPayment) PathPaymentOp();
        operationPathPayment = op.operationPathPayment; break;
    case OperationType::MANAGE_OFFER:
        operationManageOffer = op.operationManageOffer; break;
    case OperationType::CREATE_PASSIVE_OFFER:
        operationCreatePassiveOffer = op.operationCreatePassiveOffer; break;
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
    default: break;
    }
    return *this;
}
