#ifndef GETNETWORKRESPONSE_H
#define GETNETWORKRESPONSE_H

#include <QJsonObject>
#include <QString>
#include <QtGlobal>
#include "qstellar_namespace.h"

QSTELLAR_BEGIN_NS

/** Soroban RPC `getNetwork` result. */
class GetNetworkResponse
{
    QString m_friendbotUrl;
    QString m_passphrase;
    qint32 m_protocolVersion = 0;
public:
    /** Throws std::runtime_error when the passphrase is missing or empty, or
     *  protocolVersion is not a JSON integer in range. */
    static GetNetworkResponse fromJson(const QJsonObject& result);
    /** Empty for a network without a friendbot, such as mainnet. */
    QString getFriendbotUrl() const { return m_friendbotUrl; }
    QString getPassphrase() const { return m_passphrase; }
    qint32 getProtocolVersion() const { return m_protocolVersion; }
};

QSTELLAR_END_NS

QSTELLAR_ALIAS(GetNetworkResponse)




#endif // GETNETWORKRESPONSE_H
