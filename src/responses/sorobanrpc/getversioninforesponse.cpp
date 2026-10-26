#include "getversioninforesponse.h"
#include "gethealthresponse.h"

QSTELLAR_BEGIN_NS


GetVersionInfoResponse GetVersionInfoResponse::fromJson(const QJsonObject& r)
{
    // Every field is always there; a missing one used to read as empty or 0,
    // and protocolVersion was read from a string too, text as 0.
    const sorobanrpcdetail::RpcFields fields(r, QStringLiteral("getVersionInfo"));
    GetVersionInfoResponse out;
    out.m_version            = fields.string(QStringLiteral("version"));
    out.m_commitHash         = fields.string(QStringLiteral("commitHash"));
    out.m_buildTimestamp     = fields.string(QStringLiteral("buildTimestamp"));
    out.m_captiveCoreVersion = fields.string(QStringLiteral("captiveCoreVersion"));
    out.m_protocolVersion    = fields.int32(QStringLiteral("protocolVersion"));
    return out;
}
QSTELLAR_END_NS
