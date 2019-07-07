#include "bumpsequenceoperationresponse.h"

BumpSequenceOperationResponse::BumpSequenceOperationResponse(QNetworkReply *reply)
    :OperationResponse(reply),m_bumpTo(0)
{

}

BumpSequenceOperationResponse::~BumpSequenceOperationResponse(){}

qint64 BumpSequenceOperationResponse::getBumpTo() const
{
    return m_bumpTo;
}

QString BumpSequenceOperationResponse::bumpTo() const
{
    return QString::number(m_bumpTo);
}

void BumpSequenceOperationResponse::setBumpTo(QString bump_to)
{
    m_bumpTo=bump_to.toLongLong();
}
