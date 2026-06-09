#include "getnetworkresponse.h"

QSTELLAR_BEGIN_NS


GetNetworkResponse GetNetworkResponse::fromJson(const QJsonObject& r)
{
    GetNetworkResponse out;
    out.m_friendbotUrl = r.value("friendbotUrl").toString();
    out.m_passphrase   = r.value("passphrase").toString();
    QJsonValue pv = r.value("protocolVersion");
    out.m_protocolVersion = pv.isString() ? pv.toString().toInt()
                                          : static_cast<qint32>(pv.toInteger(0));
    return out;
}
QSTELLAR_END_NS
