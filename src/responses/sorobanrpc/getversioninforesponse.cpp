#include "getversioninforesponse.h"
#include "qtcompat.h"

QSTELLAR_BEGIN_NS


GetVersionInfoResponse GetVersionInfoResponse::fromJson(const QJsonObject& r)
{
    GetVersionInfoResponse out;
    out.m_version            = r.value("version").toString();
    out.m_commitHash         = r.value("commitHash").toString();
    out.m_buildTimestamp     = r.value("buildTimestamp").toString();
    out.m_captiveCoreVersion = r.value("captiveCoreVersion").toString();
    QJsonValue pv = r.value("protocolVersion");
    out.m_protocolVersion = pv.isString() ? pv.toString().toInt()
                                          : static_cast<qint32>(jsonToInt64(pv));
    return out;
}
QSTELLAR_END_NS
