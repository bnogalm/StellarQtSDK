#include "bumpsequenceoperationresponse.h"
#include <limits>

QSTELLAR_BEGIN_NS


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

bool BumpSequenceOperationResponse::readMember(const QString& key, const QJsonValue& value, const QString& field)
{
    if (key != QLatin1String("bump_to") || value.isNull())
        return false;
    m_bumpTo = ResponseConverters::exactInteger(value, field, 0, std::numeric_limits<qint64>::max());
    return true;
}
QSTELLAR_END_NS
