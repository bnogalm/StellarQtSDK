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

Operation::Operation() : m_sourceAccount(nullptr)
{
}

Operation::~Operation()
{
    if(m_sourceAccount)
        delete m_sourceAccount;
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
    if(this->getSourceAccount()){
        xdr.sourceAccount = stellar::Optional<stellar::AccountID>(stellar::PublicKey::from(this->getSourceAccount()->getPublicKey()));
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
    default:
        throw std::runtime_error("Unknown operation body");
    }
    if (xdr.sourceAccount.filled) {
        operation->setSourceAccount(KeyPair::fromXdrPublicKey(xdr.sourceAccount.value));
    }
    return operation;
}

KeyPair *Operation::getSourceAccount() {
    return m_sourceAccount;
}

Operation *Operation::setSourceAccount(KeyPair *keypair) {
    m_sourceAccount = checkNotNull(keypair, "keypair cannot be null");
    return this;
}
Operation *Operation::setSourceAccount(KeyPair &keypair) {
    m_sourceAccount = new KeyPair(keypair);
    return this;
}
