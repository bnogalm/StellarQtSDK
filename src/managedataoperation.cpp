#include "managedataoperation.h"
#include "util.h"

ManageDataOperation::ManageDataOperation(QString name, QByteArray value) {
    checkNotNull(name, "name cannot be null");
    QByteArray utf8 = name.toUtf8();
    typedef decltype(stellar::ManageDataOp::dataName) nameFieldType;
    int maxSize = nameFieldType::maxSize();
    int currentSize = name.size();
    while(utf8.size()>maxSize){
        currentSize--;
        name = name.left(currentSize);
        utf8 = name.toUtf8();
    }
    m_op.dataName.set(utf8.data(),utf8.size());
    if(!value.isNull()){
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
    return QString::fromUtf8(m_op.dataName.value.data(),m_op.dataName.value.size());
}

QByteArray ManageDataOperation::getValue() {
    if(m_op.dataValue.filled)
    {
        return QByteArray((char*)m_op.dataValue.value.value.data(),m_op.dataValue.value.value.size());
    }
    return QByteArray();
}

void ManageDataOperation::fillOperationBody(stellar::Operation &operation)
{
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

