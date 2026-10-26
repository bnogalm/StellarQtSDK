#include "managedataoperation.h"

#include <stdexcept>
#include "util.h"

QSTELLAR_BEGIN_NS


ManageDataOperation::ManageDataOperation(QString name, QByteArray value) {
    checkNotNull(name, "name cannot be null");
    QByteArray utf8 = name.toUtf8();
    typedef decltype(stellar::ManageDataOp::dataName) nameFieldType;
    int maxSize = nameFieldType::maxSize();
    // This used to trim the name in a loop until it fit, and Array::set
    // truncated the value with a qMin: we ended up signing something other
    // than what the user typed, with no warning at all. setHomeDomain already
    // threw in the same case; do it here too.
    if(utf8.size() > maxSize){
        throw std::runtime_error(
            QString("data name exceeds %1 bytes when UTF-8 encoded").arg(maxSize).toStdString());
    }
    m_op.dataName.set(utf8.data(),utf8.size());
    if(!value.isNull()){
        typedef decltype(stellar::ManageDataOp::dataValue.value) valueFieldType;
        const int maxValue = valueFieldType::maxSize();
        if(value.size() > maxValue){
            throw std::runtime_error(
                QString("data value exceeds %1 bytes").arg(maxValue).toStdString());
        }
        m_op.dataValue.value.set((uchar*)value.data(),value.size());
        m_op.dataValue.filled=1;
    }

}

ManageDataOperation::~ManageDataOperation()
{

}

ManageDataOperation::ManageDataOperation(stellar::ManageDataOp &op):m_op(op)
{

}

QString ManageDataOperation::getName() {
    // QString::fromUtf8 drops a leading UTF-8 BOM, so "\xEF\xBB\xBFclient_domain"
    // read as "client_domain" and TxRep re-signed a different name. js, java
    // and py keep it as U+FEFF; so does this.
    return Util::fromUtf8KeepingBom(getNameBytes());
}

QByteArray ManageDataOperation::getNameBytes() {
    return QByteArray(m_op.dataName.value.data(), m_op.dataName.value.size());
}

QByteArray ManageDataOperation::getValue() {
    if(m_op.dataValue.filled)
    {
        // Present and empty is not absent: an empty value must not come back
        // null, or a SEP-10 web_auth_domain of "" reads as "no value" (which
        // js and py accept and this refused).
        const QByteArray value((char*)m_op.dataValue.value.value.data(),m_op.dataValue.value.value.size());
        return value.isNull() ? QByteArray("") : value;
    }
    return QByteArray();
}

void ManageDataOperation::fillOperationBody(AccountConverter &accountConverter, stellar::Operation &operation)
{
    Q_UNUSED(accountConverter)
    auto& o = operation.fillManageDataOp();
    o = m_op;
}


ManageDataOperation *ManageDataOperation::build(stellar::ManageDataOp &op)
{
    return new ManageDataOperation(op);
}

ManageDataOperation *ManageDataOperation::create(QString name, QByteArray value)
{
    return new ManageDataOperation(name, value);
}

ManageDataOperation *ManageDataOperation::setSourceAccount(QString sourceAccount)
{
    Operation::setSourceAccount(sourceAccount);
    return this;
}

QSTELLAR_END_NS
