#include "getnetworkresponse.h"
#include "gethealthresponse.h"

QSTELLAR_BEGIN_NS


GetNetworkResponse GetNetworkResponse::fromJson(const QJsonObject& r)
{
    // protocolVersion used to be read from a string too, text as 0, and a
    // missing passphrase or version read as empty and 0.
    const sorobanrpcdetail::RpcFields fields(r, QStringLiteral("getNetwork"));
    GetNetworkResponse out;
    // Only a network with a friendbot names one (mainnet does not).
    out.m_friendbotUrl    = fields.optionalString(QStringLiteral("friendbotUrl"));
    out.m_passphrase      = fields.string(QStringLiteral("passphrase"));
    if (out.m_passphrase.isEmpty())
        fields.refuse(QStringLiteral("passphrase"), QStringLiteral("is empty"));
    out.m_protocolVersion = fields.int32(QStringLiteral("protocolVersion"));
    return out;
}
QSTELLAR_END_NS
